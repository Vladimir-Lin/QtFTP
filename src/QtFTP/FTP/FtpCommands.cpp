#include <qtftp.h>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#else
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

////////////////////////////////////////////////////////////////////////////////

N::FtpCommands:: FtpCommands (void)
               : handle      (-1  )
               , cavail      (0   )
               , cleft       (0   )
               , dir         (0   )
               , cmode       (0   )
               , CmdID       (0   )
               , xfered      (0   )
               , cbbytes     (0   )
               , xfered1     (0   )
               , cput        (NULL)
               , cget        (NULL)
               , buf         (NULL)
               , idlearg     (NULL)
               , ctrl        (NULL)
               , data        (NULL)
               , Parent      (NULL)
{
  ::memset ( &idletime , 0 , sizeof(timeval) ) ;
  ::memset ( response  , 0 , 1024            ) ;
}

N::FtpCommands:: FtpCommands (void * parent)
               : handle      (-1           )
               , cavail      (0            )
               , cleft       (0            )
               , dir         (0            )
               , cmode       (0            )
               , xfered      (0            )
               , cbbytes     (0            )
               , xfered1     (0            )
               , cput        (NULL         )
               , cget        (NULL         )
               , buf         (NULL         )
               , idlearg     (NULL         )
               , ctrl        (NULL         )
               , data        (NULL         )
               , Parent      (parent       )
{
  ::memset ( &idletime , 0 , sizeof(timeval) ) ;
  ::memset ( response  , 0 , 1024            ) ;
}

N::FtpCommands::~FtpCommands(void)
{
}

void N::FtpCommands::setParent(void * parent)
{
  Parent = parent ;
}

char * N::FtpCommands::Response(void)
{
  if ( Control == dir ) return response ;
  return NULL                           ;
}

void N::FtpCommands::setIdle(int ms)
{
  idletime . tv_sec  =   ms / 1000          ;
  idletime . tv_usec = ( ms % 1000 ) * 1000 ;
}

void N::FtpCommands::setTransfer(FtpTransferMode mode)
{
  cmode = (int) mode ;
}

void N::FtpCommands::doError(int code)
{
  if ( NULL == Parent ) return         ;
  FtpClient * fc = (FtpClient *)Parent ;
  return fc -> Error ( code , this )   ;
}

int N::FtpCommands::Callback(void)
{
  if ( NULL == Parent ) return 0       ;
  FtpClient * fc = (FtpClient *)Parent ;
  return fc -> Callback ( this )       ;
}

void N::FtpCommands::Progress(quint64 index)
{
  if ( NULL == Parent ) return         ;
  FtpClient * fc = (FtpClient *)Parent ;
  fc -> doProgress ( index , this )    ;
}

void N::FtpCommands::CallResponse(void)
{
  if ( NULL == Parent ) return         ;
  FtpClient * fc = (FtpClient *)Parent ;
  fc -> acceptResponse ( this )        ;
}

int N::FtpCommands::CloseHandle(void)
{
  #ifdef Q_OS_WIN
  return ::closesocket ( handle ) ;
  #else
  return ::close       ( handle ) ;
  #endif
}

int N::FtpCommands::ftpRead(int fd,char * buffer,size_t length)
{
  #ifdef Q_OS_WIN
  return ::recv ( fd , buffer , length , 0 )             ;
  #else
  int c = -1                                             ;
  while ( c < 0 )                                        {
    c = read ( fd , buffer , length )                    ;
    if ( c == -1 )                                       {
      if ( errno != EINTR && errno != EAGAIN ) return -1 ;
    }                                                    ;
  }                                                      ;
  return c                                               ;
  #endif
}

int N::FtpCommands::ftpWrite(int fd,const char * buffer,size_t length)
{
  #ifdef Q_OS_WIN
  return ::send ( fd , buffer , length , 0 )             ;
  #else
  int done = 0                                           ;
  while ( length > 0 )                                   {
    int c = write ( fd, buffer, length )                 ;
    if ( c == -1 )                                       {
      if ( errno != EINTR && errno != EAGAIN ) return -1 ;
    } else
    if ( c == 0 )                                        {
      return done                                        ;
    } else                                               {
      buffer += c                                        ;
      done   += c                                        ;
      length -= c                                        ;
    }                                                    ;
  }                                                      ;
  return done                                            ;
  #endif
}

/*
 * socket_wait - wait for socket to receive or flush data
 * return 1 if no user callback, otherwise, return value returned by
 * user callback
 */
int N::FtpCommands::socketWait(void)
{
  int            rv  = false                            ;
  fd_set      *  rfd = NULL                             ;
  fd_set      *  wfd = NULL                             ;
  fd_set         fd                                     ;
  struct timeval tv                                     ;
  if ( Control == dir ) return 1                        ;
  ///////////////////////////////////////////////////////
  if ( dir == Write ) wfd = &fd                         ;
                 else rfd = &fd                         ;
  FD_ZERO ( &fd )                                       ;
  ///////////////////////////////////////////////////////
  do                                                    {
    FD_SET ( handle , &fd )                             ;
    tv = idletime                                       ;
    rv = select ( handle + 1 , rfd , wfd , NULL , &tv ) ;
    if ( -1 == rv )                                     {
      rv = 0                                            ;
      strncpy ( ctrl->response                          ,
                strerror(errno)                         ,
                ResponseSize                          ) ;
      break                                             ;
    } else
    if ( rv > 0 )                                       {
      rv = 1                                            ;
      break                                             ;
    }                                                   ;
    rv = Callback ( )                                   ;
  } while ( 0 != rv )                                   ;
    /////////////////////////////////////////////////////
  return rv                                             ;
}

/*
 * read a line of text
 * return -1 on error or bytecount
 */
int N::FtpCommands::readLine(char * buffer,int max)
{
  int    retval = 0                                      ;
  int    x                                               ;
  char * end                                             ;
  char * bp     = buffer                                 ;
  int    eof    = 0                                      ;
  ////////////////////////////////////////////////////////
  if ( 0 == max                              ) return  0 ;
  if ( ( Control != dir ) && ( Read != dir ) ) return -1 ;
  ////////////////////////////////////////////////////////
  do                                                     {
    //////////////////////////////////////////////////////
    if ( cavail > 0 )                                    {
      x   = ( max >= cavail ) ? cavail : max - 1         ;
      end = (char *)memccpy ( bp , cget , '\n' , x )     ;
      if ( NULL != end ) x = end - bp                    ;
      retval += x                                        ;
      bp     += x                                        ;
      *bp     = '\0'                                     ;
      max    -= x                                        ;
      cget   += x                                        ;
      cavail -= x                                        ;
      if ( NULL != end )                                 {
        bp -= 2                                          ;
        if ( strcmp(bp,"\r\n") == 0 )                    {
          *bp++ = '\n'                                   ;
          *bp++ = '\0'                                   ;
          --retval                                       ;
        }                                                ;
        break                                            ;
      }                                                  ;
    }                                                    ;
    //////////////////////////////////////////////////////
    if ( 1 == max )                                      {
      *buffer = '\0'                                     ;
      break                                              ;
    }                                                    ;
    //////////////////////////////////////////////////////
    if ( cput == cget )                                  {
      cput   = buf                                       ;
      cget   = buf                                       ;
      cavail = 0                                         ;
      cleft  = BufferSize                                ;
    }                                                    ;
    //////////////////////////////////////////////////////
    if (eof)                                             {
      if ( 0 == retval ) retval = -1                     ;
      break                                              ;
    }                                                    ;
    //////////////////////////////////////////////////////
    if ( 0 == socketWait ( ) ) return retval             ;
    //////////////////////////////////////////////////////
    x = ftpRead ( handle , cput , cleft )                ;
    if ( -1 == x )                                       {
      retval = -1                                        ;
      break                                              ;
    }                                                    ;
    //////////////////////////////////////////////////////
    if ( 0 == x ) eof = 1                                ;
    cleft  -= x                                          ;
    cavail += x                                          ;
    cput   += x                                          ;
    //////////////////////////////////////////////////////
  } while ( 1 )                                          ;
  ////////////////////////////////////////////////////////
  return retval                                          ;
}

/*
 * write lines of text
 * return -1 on error or bytecount
 */
int N::FtpCommands::writeLine(const char * buffer,int length)
{
  int          x                                                       ;
  int          w                                                       ;
  int          nb  = 0                                                 ;
  char         lc  = 0                                                 ;
  const char * ubp = buffer                                            ;
  char       * nbp                                                     ;
  //////////////////////////////////////////////////////////////////////
  if ( Write != dir ) return -1                                        ;
  nbp = buf                                                            ;
  //////////////////////////////////////////////////////////////////////
  for ( x = 0 ; x < length ; x++ )                                     {
    ////////////////////////////////////////////////////////////////////
    if ( ( (*ubp) == '\n' ) && ( lc != '\r' ) )                        {
      if ( BufferSize == nb )                                          {
        if ( 0 == socketWait() ) return x                              ;
        w = ftpWrite ( handle , nbp , BufferSize )                     ;
        if ( BufferSize != w ) return -1                               ;
        nb = 0                                                         ;
      }                                                                ;
      nbp [ nb++ ] = '\r'                                              ;
    }                                                                  ;
    ////////////////////////////////////////////////////////////////////
    if ( BufferSize == nb )                                            {
      if ( 0 == socketWait() ) return x                                ;
      w = ftpWrite ( handle , nbp , BufferSize )                       ;
      if ( BufferSize != w ) return -1                                 ;
      nb = 0                                                           ;
    }                                                                  ;
    ////////////////////////////////////////////////////////////////////
    nbp [ nb++ ] = lc = *ubp++                                         ;
  }                                                                    ;
  //////////////////////////////////////////////////////////////////////
  if ( 0 != nb )                                                       {
    if ( 0 == socketWait() ) return x                                  ;
    w = ftpWrite ( handle , nbp , nb )                                 ;
    if ( w != nb ) return -1                                           ;
  }                                                                    ;
  //////////////////////////////////////////////////////////////////////
  return length                                                        ;
}

/*
 * read a response from the server
 * return 0 if first char doesn't match
 * return 1 if first char matches
 */
int N::FtpCommands::ReadResp(char c)
{
  char match [ 5 ]                              ;
  int  rl                                       ;
  ///////////////////////////////////////////////
  rl = readLine ( response , ResponseSize )     ;
  if ( -1 == rl ) return 0                      ;
  ///////////////////////////////////////////////
  if ( '-' == response[3] )                     {
    ::strncpy ( match , response , 3 )          ;
    match[3] = ' '                              ;
    match[4] = '\0'                             ;
    do                                          {
      rl = readLine ( response , ResponseSize ) ;
      if ( -1 == rl ) return 0                  ;
    } while ( strncmp(response,match,4) != 0 )  ;
  }                                             ;
  CallResponse ( )                              ;
  if ( response[0] == c ) return 1              ;
  return 0                                      ;
}

/*
 * SendCMD - send a command and wait for expected response
 * return 1 if proper response received, 0 otherwise
 */
int N::FtpCommands::SendCMD(const char * cmd,char expresp)
{
  char buff [ TempBufferSize ]                        ;
  if ( dir != Control                      ) return 0 ;
  if ( (strlen(cmd) + 3) >= TempBufferSize ) return 0 ;
  /////////////////////////////////////////////////////
  if (NotNull(Parent))                                {
    QString     CMD ( cmd )                           ;
    FtpClient * fc = (FtpClient *)Parent              ;
    fc -> Sending ( CMD , this )                      ;
  }                                                   ;
  /////////////////////////////////////////////////////
  sprintf ( buff , "%s\r\n" , cmd )                   ;
  int L = strlen ( buff )                             ;
  int r                                               ;
  r = ftpWrite ( handle , buff , L )                  ;
  if ( r <= 0 ) return 0                              ;
  return ReadResp ( expresp )                         ;
}

/*
 * Login - log in to remote server
 * return true if logged in, false otherwise
 */
bool N::FtpCommands::Login(const char * user,const char * pass)
{
  char tempbuf [ 64 ]                          ;
  int  ulen = 0                                ;
  int  plen = 0                                ;
  int  rv   = 0                                ;
  //////////////////////////////////////////////
  if ( NULL != user ) ulen = strlen(user)      ;
  if ( NULL != pass ) plen = strlen(pass)      ;
  //////////////////////////////////////////////
  if ( ( ulen + 7 ) > 64 ) return false        ;
  if ( ( plen + 7 ) > 64 ) return false        ;
  //////////////////////////////////////////////
  if ( ulen>0 && NULL != user )                {
    ::sprintf ( tempbuf , "USER %s" , user )   ;
    ////////////////////////////////////////////
    CmdID = USER                               ;
    rv = SendCMD ( tempbuf , '3' )             ;
    CmdID = 0                                  ;
    if ( 0 == rv )                             {
      if ( '2' == response [ 0 ] ) return true ;
      return false                             ;
    }                                          ;
  }                                            ;
  //////////////////////////////////////////////
  if ( plen>0 && NULL != pass )                {
    ::sprintf ( tempbuf , "PASS %s" , pass )   ;
    CmdID = PASS                               ;
    rv = SendCMD ( tempbuf , '2' )             ;
    CmdID = 0                                  ;
  }                                            ;
  return ( rv != 0 )                           ;
}

/*
 * Site - send a SITE command
 * return true if command successful, false otherwise
 */
bool N::FtpCommands::Site(const char * cmd)
{
  char tempbuff[TempBufferSize]                     ;
  int  clen = strlen(cmd)                           ;
  int  rv                                           ;
  ///////////////////////////////////////////////////
  if ( ( clen + 7 ) > TempBufferSize ) return false ;
  ::sprintf ( tempbuff , "SITE %s" , cmd )          ;
  CmdID = SITE                                      ;
  rv = SendCMD ( tempbuff , '2' )                   ;
  CmdID = 0                                         ;
  return ( 0 != rv )                                ;
}

/*
 * SYST - send a SYST command
 *
 * Fills in the user buffer with the remote system type.  If more
 * information from the response is required, the user can parse
 * it out of the response buffer returned by FtpLastResponse().
 * return true if command successful, false otherwise
 */
bool N::FtpCommands::SYST(char * buffer,int max)
{
  int    l = max                       ;
  char * b = buffer                    ;
  char * s                             ;
  int    rv                            ;
  //////////////////////////////////////
  CmdID = SysT                         ;
  rv = SendCMD ( "SYST" , '2' )        ;
  CmdID = 0                            ;
  if ( 0 == rv ) return false          ;
  //////////////////////////////////////
  s = &(response[4])                   ;
  while ( ( --l ) && ( (*s) != ' ' ) ) {
    *b++ = *s++                        ;
  }                                    ;
  *b++ = '\0'                          ;
  //////////////////////////////////////
  return true                          ;
}

/*
 * MakeDir - create a directory at server
 * return true if successful, false otherwise
 */
bool N::FtpCommands::MakeDir(const char * path)
{
  char tempbuff [ TempBufferSize ]                  ;
  int  plen = strlen(path)                          ;
  int  rv                                           ;
  ///////////////////////////////////////////////////
  if ( ( plen + 6 ) > TempBufferSize ) return false ;
  ::sprintf ( tempbuff , "MKD %s" , path )          ;
  CmdID = MKD                                       ;
  rv = SendCMD ( tempbuff , '2' )                   ;
  CmdID = 0                                         ;
  return ( 0 != rv )                                ;
}

/*
 * ChangeDir - change path at remote
 * return true if successful, false otherwise
 */
bool N::FtpCommands::ChangeDir(const char * path)
{
  char tempbuff [ TempBufferSize ]                  ;
  int  plen = strlen(path)                          ;
  int  rv                                           ;
  ///////////////////////////////////////////////////
  if ( ( plen + 6 ) > TempBufferSize ) return false ;
  ::sprintf ( tempbuff , "CWD %s" , path )          ;
  CmdID = CWD                                       ;
  rv = SendCMD ( tempbuff , '2' )                   ;
  CmdID = 0                                         ;
  return ( 0 != rv )                                ;
}

/*
 * CdUp - move to parent directory at remote
 * return true if successful, false otherwise
 */
bool N::FtpCommands::CdUp(void)
{
  int rv = 0                    ;
  CmdID = CDUP                  ;
  rv = SendCMD ( "CDUP" , '2' ) ;
  CmdID = 0                     ;
  return ( 0 != rv )            ;
}

/*
 * RemoveDir - remove directory at remote
 * return true if successful, false otherwise
 */
bool N::FtpCommands::RemoveDir(const char * path)
{
  char tempbuff [ TempBufferSize ]                  ;
  int  plen = strlen(path)                          ;
  int  rv                                           ;
  ///////////////////////////////////////////////////
  if ( ( plen + 6 ) > TempBufferSize ) return false ;
  ::sprintf ( tempbuff , "RMD %s" , path )          ;
  CmdID = RMD                                       ;
  rv = SendCMD ( tempbuff , '2' )                   ;
  CmdID = 0                                         ;
  return ( 0 != rv )                                ;
}

/*
 * PWD - get working directory at remote
 * return true if successful, false otherwise
 */
bool N::FtpCommands::PWD(char * path,int max)
{
  int    l = max                               ;
  char * b = path                              ;
  char * s                                     ;
  int    rv                                    ;
  //////////////////////////////////////////////
  CmdID = Pwd                                  ;
  rv = SendCMD ( "PWD" , '2' )                 ;
  CmdID = 0                                    ;
  if ( 0 == rv ) return false                  ;
  //////////////////////////////////////////////
  s = strchr ( response , '"' )                ;
  if ( NULL == s ) return false                ;
  s++                                          ;
  while ( ( --l ) && (*s) && ( (*s) != '"' ) ) {
    *b++ = *s++                                ;
  }                                            ;
  *b++ = '\0'                                  ;
  //////////////////////////////////////////////
  return true                                  ;
}

bool N::FtpCommands::Size(const char * path,quint64 & size,char mode)
{
  char    cmd [ TempBufferSize ]                       ;
  int     plen = strlen(path)                          ;
  int     rv   = 1                                     ;
  int     resp                                         ;
  quint64 sz                                           ;
  //////////////////////////////////////////////////////
  if ( ( plen + 7 ) > TempBufferSize ) return false    ;
  ::sprintf ( cmd , "TYPE %c" , mode )                 ;
  CmdID = TYPE                                         ;
  rv = SendCMD ( cmd , '2' )                           ;
  CmdID = 0                                            ;
  if ( 0 == rv ) return false                          ;
  ::sprintf ( cmd , "SIZE %c" , mode )                 ;
  CmdID = SIZE                                         ;
  rv = SendCMD ( cmd , '2' )                           ;
  CmdID =0                                             ;
  if ( 0 == rv ) return false                          ;
  //////////////////////////////////////////////////////
  rv = ::sscanf ( response , "%d %llu" , &resp , &sz ) ;
  if ( 2 == rv ) size = sz ; else return false         ;
  //////////////////////////////////////////////////////
  return true                                          ;
}

/*
 * ChangedDate - determine the modification date of a remote file
 * return true if successful, false otherwise
 */
bool N::FtpCommands::ChangedDate(const char * path,char * dt,int max)
{
  char cmd [ TempBufferSize ]                       ;
  int  plen = strlen(path)                          ;
  int  rv   = 1                                     ;
  ///////////////////////////////////////////////////
  if ( ( plen + 7 ) > TempBufferSize ) return false ;
  ::sprintf ( cmd , "MDTM %s" , path )              ;
  CmdID = MDTM                                      ;
  rv = SendCMD ( cmd , '2' )                        ;
  CmdID = 0                                         ;
  if ( 0 != rv )                                    {
    ::strncpy ( dt , &(response[4]) , max )         ;
  }                                                 ;
  ///////////////////////////////////////////////////
  return ( 0 != rv )                                ;
}

/*
 * Rename - rename a file at remote
 * return true if successful, false otherwise
 */
bool N::FtpCommands::Rename(const char * src,const char * dst)
{
  char cmd [ TempBufferSize ]                       ;
  int  slen = strlen(src)                           ;
  int  dlen = strlen(dst)                           ;
  int  rv                                           ;
  ///////////////////////////////////////////////////
  if ( ( slen + 7 ) > TempBufferSize ) return false ;
  if ( ( dlen + 7 ) > TempBufferSize ) return false ;
  ::sprintf ( cmd , "RNFR %s" , src )               ;
  CmdID = RNFR                                      ;
  rv = SendCMD ( cmd , '3' )                        ;
  CmdID = 0                                         ;
  if ( 0 == rv ) return false                       ;
  ::sprintf ( cmd , "RNTO %s" , dst )               ;
  CmdID = RNTO                                      ;
  rv = SendCMD ( cmd , '2' )                        ;
  CmdID = 0                                         ;
  if ( 0 == rv ) return false                       ;
  return true                                       ;
}

/*
 * Delete - delete a file at remote
 * return true if successful, false otherwise
 */
bool N::FtpCommands::Delete(const char * filename)
{
  char tempbuff [ TempBufferSize ]                  ;
  int  plen = strlen(filename)                      ;
  int  rv                                           ;
  ///////////////////////////////////////////////////
  if ( ( plen + 7 ) > TempBufferSize ) return false ;
  ::sprintf ( tempbuff , "DELE %s" , filename )     ;
  CmdID = DELE                                      ;
  rv = SendCMD ( tempbuff , '2' )                   ;
  CmdID = 0                                         ;
  return ( 0 != rv )                                ;
}

/*
 * Quit - disconnect from remote
 * return true if successful, false otherwise
 */
bool N::FtpCommands::Quit(void)
{
  if ( Control != dir ) return false ;
  int rv                             ;
  ////////////////////////////////////
  CmdID = QUIT                       ;
  rv = SendCMD ( "QUIT" , '2' )      ;
  CmdID = 0                          ;
  CloseHandle  (              )      ;
  ////////////////////////////////////
  return ( 0 != rv )                 ;
}

/*
 * DataPort - set up data connection
 * return Data FtpCommands if successful, NULL otherwise
 */
N::FtpCommands * N::FtpCommands::DataPort(int Mode,int Dir)
{
  union                                                    {
    struct sockaddr    sa                                  ;
    struct sockaddr_in in                                  ;
  } sin                                                    ;
  struct linger  lng = { 0, 0 }                            ;
  unsigned int   l                                         ;
  int            sData                                     ;
  int            rv                                        ;
  int            on  = 1                                   ;
  FtpCommands  * ctl = NULL                                ;
  char         * cp                                        ;
  unsigned int   v        [ 6              ]               ;
  char           tempbuff [ TempBufferSize ]               ;
  //////////////////////////////////////////////////////////
  if ( Control != dir ) return NULL                        ;
  if ( ( Read    != Dir ) && ( Dir != Write ) )            {
    ::sprintf ( response , "Invalid direction %d\n", Dir ) ;
    doError ( 2001 )                                       ;
    return NULL                                            ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  if ( ( 'A' != Mode ) && ( 'I' != Mode ) )                {
    ::sprintf ( response , "Invalid mode %c\n" , Mode    ) ;
    doError ( 2002 )                                       ;
    return NULL                                            ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  l = sizeof(sin)                                          ;
  if ( Passive == cmode )                                  {
    ::memset ( &sin , 0 , l )                              ;
    sin.in.sin_family = AF_INET                            ;
    CmdID = PASV                                           ;
    rv = SendCMD ( "PASV" , '2' )                          ;
    CmdID = 0                                              ;
    if ( 0 == rv )                                         {
      doError ( 2003 )                                     ;
      return NULL                                          ;
    }                                                      ;
    cp = ::strchr ( response , '(' )                       ;
    if ( NULL == cp )                                      {
      doError ( 2004 )                                     ;
      return NULL                                          ;
    }                                                      ;
    cp++                                                   ;
    ::sscanf ( cp , "%u,%u,%u,%u,%u,%u"                    ,
               &v[2],&v[3],&v[4],&v[5],&v[0],&v[1]       ) ;
    sin.sa.sa_data[2] = v[2]                               ;
    sin.sa.sa_data[3] = v[3]                               ;
    sin.sa.sa_data[4] = v[4]                               ;
    sin.sa.sa_data[5] = v[5]                               ;
    sin.sa.sa_data[0] = v[0]                               ;
    sin.sa.sa_data[1] = v[1]                               ;
  } else                                                   {
    #ifdef Q_OS_WIN
    rv = ::getsockname ( handle , &sin.sa , (int       *)&l )    ;
    #else
    rv = ::getsockname ( handle , &sin.sa , (socklen_t *)&l )    ;
    #endif
    if ( rv < 0)                                           {
      doError ( 2005 )                                     ;
      return NULL                                          ;
    }                                                      ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  sData = ::socket ( PF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
  if ( -1 == sData )                                       {
    doError ( 2006 )                                       ;
    return NULL                                            ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  #ifdef Q_OS_WIN
  rv = ::setsockopt                                        (
         sData                                             ,
         SOL_SOCKET                                        ,
         SO_REUSEADDR                                      ,
         (const char *) &on                                ,
         sizeof(int)                                     ) ;
  #else
  rv = ::setsockopt                                        (
         sData                                             ,
         SOL_SOCKET                                        ,
         SO_REUSEADDR                                      ,
         (void *) &on                                      ,
         sizeof(int)                                     ) ;
  #endif
  if ( -1 == rv )                                          {
    #ifdef Q_OS_WIN
    ::closesocket ( sData )                                ;
    #else
    ::close       ( sData )                                ;
    #endif
    doError ( 2007 )                                       ;
    return NULL                                            ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  #ifdef Q_OS_WIN
  rv = ::setsockopt                                        (
         sData                                             ,
         SOL_SOCKET                                        ,
         SO_LINGER                                         ,
         (const char *) &lng                               ,
         sizeof(linger)                                  ) ;
  #else
  rv = ::setsockopt                                        (
         sData                                             ,
         SOL_SOCKET                                        ,
         SO_LINGER                                         ,
         (void *) &lng                                     ,
         sizeof(linger)                                  ) ;
  #endif
  if ( -1 == rv )                                          {
    #ifdef Q_OS_WIN
    ::closesocket ( sData )                                ;
    #else
    ::close       ( sData )                                ;
    #endif
    doError ( 2008 )                                       ;
    return NULL                                            ;
  }
  //////////////////////////////////////////////////////////
  if ( Passive == cmode )                                  {
    rv = ::connect ( sData , &sin.sa , sizeof(sockaddr) )  ;
    if ( -1 == rv )                                        {
      #ifdef Q_OS_WIN
      ::closesocket ( sData )                              ;
      #else
      ::close       ( sData )                              ;
      #endif
      doError ( 2009 )                                     ;
      return NULL                                          ;
    }                                                      ;
  } else                                                   {
    sin.in.sin_port = 0                                    ;
    rv = ::bind ( sData , &sin.sa , sizeof(sockaddr) )     ;
    if ( -1 == rv )                                        {
      #ifdef Q_OS_WIN
      ::closesocket ( sData )                              ;
      #else
      ::close       ( sData )                              ;
      #endif
      doError ( 2010 )                                     ;
      return NULL                                          ;
    }                                                      ;
    rv = ::listen(sData, 1)                                ;
    if ( rv < 0)                                           {
      #ifdef Q_OS_WIN
      ::closesocket ( sData )                              ;
      #else
      ::close       ( sData )                              ;
      #endif
      doError ( 2011 )                                     ;
      return NULL                                          ;
    }                                                      ;
    #ifdef Q_OS_WIN
    rv = ::getsockname ( sData,&sin.sa,(int       *)&l )   ;
    #else
    rv = ::getsockname ( sData,&sin.sa,(socklen_t *)&l )   ;
    #endif
    if ( rv < 0)                                           {
      doError ( 2012 )                                     ;
      return NULL                                          ;
    }                                                      ;
    ::sprintf                                              (
      tempbuff                                             ,
      "PORT %d,%d,%d,%d,%d,%d"                             ,
      (unsigned char) sin.sa.sa_data[2]                    ,
      (unsigned char) sin.sa.sa_data[3]                    ,
      (unsigned char) sin.sa.sa_data[4]                    ,
      (unsigned char) sin.sa.sa_data[5]                    ,
      (unsigned char) sin.sa.sa_data[0]                    ,
      (unsigned char) sin.sa.sa_data[1]                  ) ;
    CmdID = PORT                                           ;
    rv = SendCMD ( tempbuff , '2' )                        ;
    CmdID = 0                                              ;
    if ( 0 == rv )                                         {
      #ifdef Q_OS_WIN
      ::closesocket ( sData )                              ;
      #else
      ::close       ( sData )                              ;
      #endif
      doError ( 2013 )                                     ;
      return NULL                                          ;
    }                                                      ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  ctl = new FtpCommands ( Parent )                         ;
  if ( NULL == ctl ) {
    #ifdef Q_OS_WIN
    ::closesocket ( sData )                                ;
    #else
    ::close       ( sData )                                ;
    #endif
    doError ( 2014 )                                       ;
    return NULL                                            ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  if ( 'A' == Mode)                                        {
    ctl->buf = new char [ BufferSize ]                     ;
    if ( NULL == ctl -> buf )                              {
      delete ctl                                           ;
      ctl = NULL                                           ;
      #ifdef Q_OS_WIN
      ::closesocket ( sData )                              ;
      #else
      ::close       ( sData )                              ;
      #endif
      doError ( 2015 )                                     ;
      return NULL                                          ;
    }
  }                                                        ;
  //////////////////////////////////////////////////////////
  ctl -> handle   = sData                                  ;
  ctl -> dir      = Dir                                    ;
  ctl -> idletime = idletime                               ;
  ctl -> idlearg  = idlearg                                ;
  ctl -> xfered   = 0                                      ;
  ctl -> xfered1  = 0                                      ;
  ctl -> cbbytes  = cbbytes                                ;
  //////////////////////////////////////////////////////////
  return ctl                                               ;
}

/*
 * FtpAcceptConnection - accept connection from server
 * return true if successful, false otherwise
 */
bool N::FtpCommands::AcceptConnection(void)
{
  int             sData                                            ;
  struct sockaddr addr                                             ;
  unsigned int    l                                                ;
  int             i                                                ;
  struct timeval  tv                                               ;
  fd_set          mask                                             ;
  int             rv                                               ;
  //////////////////////////////////////////////////////////////////
  if ( NULL == ctrl ) return false                                 ;
  if ( NULL == data ) return false                                 ;
  //////////////////////////////////////////////////////////////////
  FD_ZERO ( &mask                  )                               ;
  FD_SET  ( ctrl -> handle , &mask )                               ;
  FD_SET  ( data -> handle , &mask )                               ;
  //////////////////////////////////////////////////////////////////
  tv.tv_usec = 0                                                   ;
  tv.tv_sec  = AcceptTimeout                                       ;
  //////////////////////////////////////////////////////////////////
  i = ctrl -> handle                                               ;
  if ( i < data -> handle ) i = data -> handle                     ;
  //////////////////////////////////////////////////////////////////
  i = ::select ( i + 1 , &mask , NULL , NULL , &tv )               ;
  if ( -1 == i )                                                   {
    ::strncpy ( ctrl->response , strerror(errno),ResponseSize)     ;
    data -> CloseHandle ( )                                        ;
    data -> handle = 0                                             ;
    rv = 0                                                         ;
  } else
  if ( 0 == i )                                                    {
    ::strcpy ( ctrl->response,"timed out waiting for connection" ) ;
    data -> CloseHandle ( )                                        ;
    data -> handle = 0                                             ;
    rv = 0                                                         ;
  } else                                                           {
    if ( FD_ISSET ( data->handle , &mask ) )                       {
      l     = sizeof    ( sockaddr                               ) ;
      #ifdef Q_OS_WIN
      sData = ::accept  ( data->handle , &addr , (int       *)&l ) ;
      #else
      sData = ::accept  ( data->handle , &addr , (socklen_t *)&l ) ;
      #endif
      i     = errno                                                ;
      data -> CloseHandle (                                  )     ;
      if ( sData > 0 )                                             {
        rv             = 1                                         ;
        data -> handle = sData                                     ;
      } else                                                       {
        ::strncpy ( ctrl->response , strerror(i) , ResponseSize  ) ;
        data->handle = 0                                           ;
        rv           = 0                                           ;
      }                                                            ;
    } else
    if ( FD_ISSET ( ctrl->handle , &mask ) )                       {
      data -> CloseHandle (     )                                  ;
      data -> handle = 0                                           ;
      ctrl -> ReadResp    ( '2' )                                  ;
      rv    = 0                                                    ;
    }                                                              ;
  }                                                                ;
  //////////////////////////////////////////////////////////////////
  return ( rv != 0 )                                               ;
}

/*
 * FtpAccess - return a handle for a data stream
 * return FtpCommands if successful, NULL otherwise
 */
N::FtpCommands * N::FtpCommands::Access(const char * path,int Type,int Mode)
{
  char          tempbuff [ TempBufferSize ]                              ;
  int           Dir                                                      ;
  int           rv                                                       ;
  FtpCommands * DTP = NULL                                               ;
  ////////////////////////////////////////////////////////////////////////
  if ( NULL == ctrl ) return NULL                                        ;
  if ( 0    == Type ) return NULL                                        ;
  if ( NULL == path )                                                    {
    if ( ( RETR == Type ) || ( STOR == Type ) )                          {
      ::sprintf                                                          (
        ctrl->response                                                   ,
        "Missing path argument for file transfer\n"                    ) ;
      doError ( 3001 )                                                   ;
      return NULL                                                        ;
    }                                                                    ;
  }                                                                      ;
  ////////////////////////////////////////////////////////////////////////
  ::sprintf    ( tempbuff , "TYPE %c" , Mode )                           ;
  rv = SendCMD ( tempbuff , '2'              )                           ;
  if ( 0 == rv )                                                         {
    doError ( 3002 )                                                     ;
    return NULL                                                          ;
  }                                                                      ;
  ////////////////////////////////////////////////////////////////////////
  switch ( Type )                                                        {
    case NLST                                                            :
      ::strcpy ( tempbuff , "NLST" )                                     ;
      Dir   = Read                                                       ;
      CmdID = NLST                                                       ;
    break                                                                ;
    case LIST                                                            :
      ::strcpy ( tempbuff , "LIST" )                                     ;
      Dir   = Read                                                       ;
      CmdID = LIST                                                       ;
    break                                                                ;
    case RETR                                                            :
      ::strcpy ( tempbuff  ,"RETR" )                                     ;
      Dir   = Read                                                       ;
      CmdID = RETR                                                       ;
    break                                                                ;
    case STOR                                                            :
      ::strcpy ( tempbuff , "STOR" )                                     ;
      Dir   = Write                                                      ;
      CmdID = STOR                                                       ;
    break                                                                ;
    default                                                              :
      ::sprintf ( ctrl->response , "Invalid open type %d\n" , Type )     ;
      doError ( 3003 )                                                   ;
    return NULL                                                          ;
  }                                                                      ;
  ////////////////////////////////////////////////////////////////////////
  if ( NULL != path )                                                    {
    int i = strlen(tempbuff)                                             ;
     tempbuff [ i++ ] = ' '                                              ;
     if ( ( strlen(path) + i + 1 ) >= TempBufferSize ) return NULL       ;
     ::strcpy ( &tempbuff[i] , path )                                    ;
  }                                                                      ;
  ////////////////////////////////////////////////////////////////////////
  DTP = DataPort ( Mode , Dir )                                          ;
  if ( NULL == DTP ) return NULL                                         ;
  ////////////////////////////////////////////////////////////////////////
  rv  = SendCMD ( tempbuff , '1' )                                       ;
  CmdID = 0                                                              ;
  if ( 0 == rv )                                                         {
    DTP -> UdpClose ( )                                                  ;
    delete DTP                                                           ;
    doError ( 3004 )                                                     ;
    return NULL                                                          ;
  }                                                                      ;
  ////////////////////////////////////////////////////////////////////////
  DTP -> ctrl = this                                                     ;
  DTP -> data = DTP                                                      ;
  data        = DTP                                                      ;
  ////////////////////////////////////////////////////////////////////////
  if ( Active == ctrl -> cmode )                                         {
    if (AcceptConnection())                                              {
      data -> UdpClose ( )                                               ;
      delete data                                                        ;
      data = NULL                                                        ;
      doError ( 3005 )                                                   ;
      return NULL                                                        ;
    }                                                                    ;
  }                                                                      ;
  ////////////////////////////////////////////////////////////////////////
  return DTP                                                             ;
}

/* UdpRead - read from a data connection */
int N::FtpCommands::UdpRead(void * buffer,int max)
{
  int i                                                           ;
  /////////////////////////////////////////////////////////////////
  if ( Read != dir ) return 0                                     ;
  if ( NULL != buf ) i = readLine ( (char *)buffer , max ) ; else {
    i = socketWait ( )                                            ;
    if ( 0 == i ) return 0                                        ;
    i = ftpRead ( handle , (char *)buffer , max )                 ;
  }                                                               ;
  if ( -1 == i ) return 0                                         ;
  xfered += i                                                     ;
  /////////////////////////////////////////////////////////////////
  if ( cbbytes > 0 )                                              {
    xfered1 += i                                                  ;
    if ( xfered1 > cbbytes )                                      {
      i = Callback ( )                                            ;
      if ( 0 == i ) return 0                                      ;
      xfered1 = 0                                                 ;
    }                                                             ;
  }                                                               ;
  /////////////////////////////////////////////////////////////////
  return i                                                        ;
}

/* UdpWrite - write to a data connection */
int N::FtpCommands::UdpWrite(const void * buffer,int len)
{
  int i                                                             ;
  ///////////////////////////////////////////////////////////////////
  if ( Write != dir ) return 0                                      ;
  if ( NULL  != buf ) i = writeLine ( (char *)buffer , len ) ; else {
    socketWait ( )                                                  ;
    i = ftpWrite ( handle , (char *)buffer , len)                   ;
  }                                                                 ;
  if ( -1 == i ) return 0                                           ;
  xfered += i                                                       ;
  ///////////////////////////////////////////////////////////////////
  if ( cbbytes > 0 )                                                {
    xfered1 += i                                                    ;
    if ( xfered1 > cbbytes)                                         {
      Callback ( )                                                  ;
      xfered1 = 0                                                   ;
    }                                                               ;
  }                                                                 ;
  ///////////////////////////////////////////////////////////////////
  return i                                                          ;
}

/* UpdClose - close a data connection */
bool N::FtpCommands::UdpClose(void)
{
  FtpCommands * ctl = NULL            ;
  switch ( dir )                      {
    case Write                        :
      if ( NULL != buf )              {
        writeLine ( NULL , 0 )        ;
      }                               ;
    case Read                         :
      if ( NULL != buf )              {
        delete [] buf                 ;
      }                               ;
      ::shutdown  ( handle , 2 )      ;
      CloseHandle (            )      ;
      ctl        = ctrl               ;
      if ( NULL != ctl )              {
        ctl -> data = NULL            ;
        if ( ctl->response[0] != '4' &&
             ctl->response[0] !=  5 ) {
          return ctl->ReadResp('2')   ;
        }                             ;
      }                               ;
    return true                       ;
    case Control                      :
      if ( NULL != data )             {
        ctrl  = NULL                  ;
        data -> UdpClose ( )          ;
      }                               ;
      CloseHandle ( )                 ;
    return false                      ;
  }                                   ;
  return true                         ;
}

/*
 * Transfer - issue a command and transfer data
 * return true if successful, false otherwise
 */
bool N::FtpCommands::Transfer(QIODevice & io,const char * path,int Type,int Mode)
{
  int    l                                                                ;
  int    c                                                                ;
  char * dbuf                                                             ;
  int    rv = 1                                                           ;
  /////////////////////////////////////////////////////////////////////////
  QIODevice::OpenMode flags                                               ;
  switch ( Type )                                                         {
    case NLST                                                             :
    case LIST                                                             :
    case RETR                                                             :
      flags  = QIODevice :: WriteOnly                                     ;
    break                                                                 ;
    case STOR                                                             :
      flags  = QIODevice :: ReadOnly                                      ;
    break                                                                 ;
    default                                                               :
    return false                                                          ;
  }                                                                       ;
  if ( 'A' == Mode ) flags |= QIODevice::Text                             ;
  if ( ! io . open ( flags ) )                                            {
    doError ( 4001 )                                                      ;
    return false                                                          ;
  }                                                                       ;
  /////////////////////////////////////////////////////////////////////////
  data = Access ( path , Type , Mode )                                    ;
  if ( NULL == data )                                                     {
    io . close ( )                                                        ;
    doError ( 4002 )                                                      ;
    return false                                                          ;
  }                                                                       ;
  /////////////////////////////////////////////////////////////////////////
  dbuf          = new char [ BufferSize ]                                 ;
  quint64 total = 0                                                       ;
  switch ( Type )                                                         {
    case NLST                                                             :
    case LIST                                                             :
    case RETR                                                             :
      while ( ( l = data -> UdpRead ( dbuf , BufferSize ) ) > 0 )         {
        if ( io . write ( dbuf , l ) != l )                               {
          rv = 0                                                          ;
          break                                                           ;
        }                                                                 ;
        total += l                                                        ;
        if ( RETR == Type ) Progress ( total )                            ;
      }                                                                   ;
    break                                                                 ;
    case STOR                                                             :
      while ( ( l = io . read ( dbuf , BufferSize ) ) > 0 )               {
        if ( ( c = data -> UdpWrite ( dbuf , l ) ) < l )                  {
          rv = 0                                                          ;
          break                                                           ;
        }                                                                 ;
        total += l                                                        ;
        Progress ( total )                                                ;
      }                                                                   ;
    break                                                                 ;
    default                                                               :
    return false                                                          ;
  }                                                                       ;
  /////////////////////////////////////////////////////////////////////////
  delete [] dbuf                                                          ;
  io      . close    ( )                                                  ;
  data   -> UdpClose ( )                                                  ;
  return ( 0 != rv )                                                      ;
}

/*
 * FtpNlst - issue an NLST command and write response to output
 * return true if successful, false otherwise
 */
bool N::FtpCommands::nLst(QIODevice & io,const char * path)
{
  return Transfer ( io , path , NLST , 'A' ) ;
}

/*
 * FtpDir - issue a LIST command and write response to output
 * return 1 if successful, 0 otherwise
 */
bool N::FtpCommands::List(QIODevice & io,const char * path)
{
  return Transfer ( io , path , LIST , 'A' ) ;
}

/*
 * FtpGet - issue a GET command and write received data to output
 * return 1 if successful, 0 otherwise
 */
bool N::FtpCommands::Get(QIODevice & io,const char * path,char mode)
{
  return Transfer ( io , path , RETR , mode ) ;
}

/*
 * FtpPut - issue a PUT command and send data from input
 * return 1 if successful, 0 otherwise
 */
bool N::FtpCommands::Put(QIODevice & io,const char * path,char mode)
{
  return Transfer ( io , path , STOR , mode ) ;
}

/*
 * FtpConnect - connect to remote server
 * return true if connected, false if not
 */
bool N::FtpCommands::connectTo(const char * host,int port)
{
  int                sControl                                       ;
  struct sockaddr_in sin                                            ;
  int                on = 1                                         ;
  int                rv                                             ;
  char             * lhost                                          ;
  ///////////////////////////////////////////////////////////////////
  ::memset ( &sin , 0 , sizeof(sockaddr_in) )                       ;
  sin.sin_family = AF_INET                                          ;
  lhost          = strdup ( host )                                  ;
  sin.sin_port   = htons  ( port )                                  ;
  ///////////////////////////////////////////////////////////////////
  sin.sin_addr.s_addr = inet_addr(lhost)                            ;
  if ( sin.sin_addr.s_addr == INADDR_NONE)                          {
    struct hostent * phe = gethostbyname(lhost)                     ;
    if ( NULL == phe )                                              {
      doError ( 1001 )                                              ;
      ::free ( lhost )                                              ;
      return false                                                  ;
    }                                                               ;
    ::memcpy( (char *)&sin.sin_addr , phe->h_addr , phe->h_length ) ;
  }                                                                 ;
  ::free ( lhost )                                                  ;
  ///////////////////////////////////////////////////////////////////
  sControl = ::socket ( PF_INET , SOCK_STREAM , IPPROTO_TCP )       ;
  if ( -1 == sControl )                                             {
    doError ( 1002 )                                                ;
    return false                                                    ;
  }                                                                 ;
  ///////////////////////////////////////////////////////////////////
  #ifdef Q_OS_WIN
  rv = ::setsockopt                                                 (
         sControl                                                   ,
         SOL_SOCKET                                                 ,
         SO_REUSEADDR                                               ,
         (const char *) &on                                         ,
         sizeof(int)                                              ) ;
  #else
  rv = ::setsockopt                                                 (
         sControl                                                   ,
         SOL_SOCKET                                                 ,
         SO_REUSEADDR                                               ,
         (void *) &on                                               ,
         sizeof(on)                                               ) ;
  #endif
  if ( -1 == rv )                                                   {
    doError ( 1003 )                                                ;
    #ifdef Q_OS_WIN
    ::closesocket ( sControl )                                      ;
    #else
    ::close       ( sControl )                                      ;
    #endif
    return false                                                    ;
  }                                                                 ;
  ///////////////////////////////////////////////////////////////////
  rv = ::connect(sControl,(struct sockaddr *)&sin,sizeof(sockaddr)) ;
  if ( -1 == rv)                                                    {
    doError ( 1004 )                                                ;
    #ifdef Q_OS_WIN
    ::closesocket ( sControl )                                      ;
    #else
    ::close       ( sControl )                                      ;
    #endif
    return false                                                    ;
  }                                                                 ;
  ///////////////////////////////////////////////////////////////////
  buf = new char [ BufferSize ]                                     ;
  if ( NULL == buf )                                                {
    doError ( 1005 )                                                ;
    #ifdef Q_OS_WIN
    ::closesocket ( sControl )                                      ;
    #else
    ::close       ( sControl )                                      ;
    #endif
    return false                                                    ;
  }                                                                 ;
  ///////////////////////////////////////////////////////////////////
  handle             = sControl                                     ;
  dir                = Control                                      ;
  ctrl               = this                                         ;
  cmode              = Passive                                      ;
  idletime . tv_sec  = 0                                            ;
  idletime . tv_usec = 0                                            ;
  idlearg            = NULL                                         ;
  xfered             = 0                                            ;
  xfered1            = 0                                            ;
  cbbytes            = 0                                            ;
  ///////////////////////////////////////////////////////////////////
  if ( 0 == ReadResp ( '2' ) )                                      {
    doError ( 1010 )                                                ;
    #ifdef Q_OS_WIN
    ::closesocket ( sControl )                                      ;
    #else
    ::close       ( sControl )                                      ;
    #endif
    delete [] buf                                                   ;
    return false                                                    ;
  }                                                                 ;
  ///////////////////////////////////////////////////////////////////
  return true                                                       ;
}
