#include <qtftp.h>

bool isValidUTF8(const QByteArray & str)
{
  for (int i = 0;i<str.size();++i)                                     {
    int l = 0                                                          ;
    if (!(str[i] & 0x80U)) l = 1                                       ; else //     0....... ASCII-7
    if (!(str[i] & 0x40U)) return false                                ; else //    10....... Invalid sequence
    if (!(str[i] & 0x20U)) l = 2                                       ; else //   110.......
    if (!(str[i] & 0x10U)) l = 3                                       ; else //  1110.......
    if (!(str[i] & 0x08U)) l = 4                                       ; else // 11110.......
      return false                                                     ;      // Invalid sequence
    ////////////////////////////////////////////////////////////////////
    if ( l == 1 ) continue                                             ;
    if ( i + l - 1 >= str.size() ) return false                        ; // Truncated code is not valid
    for ( int j = 1 ; j < l ; ++j )                                    {
      if ( (str[i + j] & 0xC0U) != 0x80 )                              { // Expect 10......
        return false                                                   ;
      }                                                                ;
    }                                                                  ;
    ////////////////////////////////////////////////////////////////////
    switch ( l )                                                       {
      case 2                                                           :
        if ( (((str[i] & 0x0FU) << 6) | (str[i+1] & 0x3FU) ) < 0x80U ) {
          return false                                                 ; // Too small ?
        }                                                              ;
      break                                                            ;
      case 3                                                           :
        if ( (((str[i    ] & 0x07U) << 12)                             |
              ((str[i + 1] & 0x3FU) <<  6)                             | // Too small ?
               (str[i + 2] & 0x3FU)) < 0x0800U )                       {
          return false                                                 ;
        }                                                              ;
      break                                                            ;
      case 4:                                                          {
        const quint32 n = ((str[i    ] & 0x07U) << 18)                 |
                          ((str[i + 1] & 0x3FU) << 12)                 |
                          ((str[i + 2] & 0x3FU) <<  6)                 |
                           (str[i + 3] & 0x3FU                       ) ;
        if ( n < 0x01000U || n > 0x10FFFFU )                           { // Too small or too big ?
          return false                                                 ;
        }                                                              ;
      }                                                                ;
      break                                                            ;
    }                                                                  ;
    i += l - 1                                                         ;
  }                                                                    ;
  return true                                                          ;
}

N::FtpThread:: FtpThread (QTcpSocket * s,QObject * parent)
             : QObject   (                         parent)
{ // Since Neutrino File sharing supports RFC 2640, UTF-8 is enabled by default
  utf8Enabled    = true                                         ;
  logged         = false                                        ;
  login_accepted = false                                        ;
  pass_accepted  = false                                        ;
  skip           = 0                                            ;
  renameFrom     . clear ( )                                    ;
  sock           = s                                            ;
  serv           = new QTcpServer ( this )                      ;
  serv          -> listen ( QHostAddress::Any )                 ;
  ///////////////////////////////////////////////////////////////
  nConnect ( serv , SIGNAL (newConnection ())                   ,
             this , SLOT   (pasvConnection())                 ) ;
  login . clear ( )                                             ;
  if ( sock->state() == QTcpSocket::ConnectedState )            {
    connection ( )                                              ;
  } else                                                        {
    connect ( sock , SIGNAL(connected ())                       ,
              this , SLOT  (connection())                     ) ;
  }                                                             ;
  ///////////////////////////////////////////////////////////////
  nConnect ( sock , SIGNAL(error(QAbstractSocket::SocketError)) ,
             this , SLOT  (deleteLater   ())                  ) ;
  nConnect ( sock , SIGNAL(disconnected  ())                    ,
             this , SLOT  (deleteLater   ())                  ) ;
  nConnect ( sock , SIGNAL(readyRead     ())                    ,
             this , SLOT  (processCommand())                  ) ;
  ///////////////////////////////////////////////////////////////
  passv         = NULL                                          ;
  mode          = NONE                                          ;
  waitingForDTP = false                                         ;
  ///////////////////////////////////////////////////////////////
  if (FtpSettings::isGui()) FtpSettings::connect(this)          ;
  ///////////////////////////////////////////////////////////////
  nConnect ( this , SIGNAL( DTPconnected  () )                  ,
             this , SLOT  ( processCommand() )                ) ;
}

N::FtpThread::~FtpThread(void)
{
  mode = NONE ;
}

QString N::FtpThread::toStringGuessEncoding(const QByteArray & data)
{
  if (utf8Enabled || isValidUTF8(data))               {
    return QString::fromUtf8(data.data(),data.size()) ;
  }                                                   ;
  return QString::fromLatin1(data.data(),data.size()) ;
}

bool N::FtpThread::UTF8mode(void) const
{
  return utf8Enabled ;
}

void N::FtpThread::connection(void)
{
  send ( "220 Neutrino file sharing server" ) ;
}

bool N::FtpThread::isConnected(void)
{
  return   sock
      &&   sock -> isValid ( )
      && ( sock -> state   ( ) == QTcpSocket::ConnectedState
      ||   sock -> state   ( ) == QTcpSocket::ConnectingState ) ;
}

void N::FtpThread::processCommand(void)
{
  if ( IsNull(sock)                                                         ||
     ! ( sock -> isValid ( )                                                &&
       ( sock -> state   ( ) == QTcpSocket::ConnectedState                  ||
         sock -> state   ( ) == QTcpSocket::ConnectingState ) )             ||
        !sock -> canReadLine ( ) ) return                                    ;
  ////////////////////////////////////////////////////////////////////////////
  while ( sock -> canReadLine ( ) )                                          {
    if (waitingForDTP) return                                                ;
    const QByteArray line = sock -> readLine ( ) . trimmed ( )               ;
    if (line.isEmpty()) return                                               ;
    QList<QByteArray> params = line . split ( ' ' )                          ;
    params[0] = params [ 0 ] . toUpper ( )                                   ;
    QByteArray _param = line.right(line.size() - params[0].size()).trimmed() ;
    QString     param = toStringGuessEncoding ( _param )                     ;
    if (!logged) renameFrom . clear ( )                                      ;
    //////////////////////////////////////////////////////////////////////////
    if (params[0] == "USER")                                                 {
      if ( params.size() == 1                                               ||
         ( params.size() >= 2                                               &&
         ( FileSharing::instance()->knownLogin(param)                       ||
           param == "anonymous" ) )                                        ) {
        if (params.size() == 1                                              ||
           (params.size() >= 2                                              &&
            param         == "anonymous"                                 ) ) {
          login = "anonymous"                                                ;
          send ( "230 User logged in, proceed." )                            ;
          pass_accepted = true                                               ;
          uid           = FtpUserID ( login , QString("anonymous") )         ;
          path          . SetUID    ( uid                          )         ;
        } else                                                               {
          login = param                                                      ;
          send ( "331 User name okay, need password." )                      ;
        }                                                                    ;
        login_accepted = true                                                ;
      } else                                                                 {
        send ( "332 Need account for login." )                               ;
        login_accepted = false                                               ;
      }                                                                      ;
      logged = login_accepted && pass_accepted                               ;
    } else
    if ( params[0] == "PASS" )                                               {
      if ( params.size() >= 2 )                                              {
        uid  = FtpUserID ( login , param )                                   ;
        path . SetUID    ( uid           )                                   ;
        if ( FileSharing :: instance ( ) -> checkID ( uid ) )                {
          send ( "230 User logged in, proceed." )                            ;
          pass_accepted = true                                               ;
        } else                                                               {
          send ( "530 Not logged in." )                                      ;
          pass_accepted = false                                              ;
        }                                                                    ;
      } else                                                                 {
        send ( "530 Not logged in." )                                        ;
        pass_accepted = false                                                ;
      }                                                                      ;
      logged = login_accepted && pass_accepted                               ;
    } else
    if ( params[0] == "NOOP" )                                               {
      send ( "200 Command okay." )                                           ;
    } else // Send the list of supported features
    if ( params[0] == "FEAT" )                                               {
      send ( "211-Features"  )                                               ;
      send ( " MDTM"         )                                               ;
      send ( " UTF8"         )                                               ;
      send ( " REST"         )                                               ;
      send ( " SIZE"         )                                               ;
      send ( " LANG en-US*"  )                                               ;
      send ( "211 end"       )                                               ;
    } else // The OPTS command
    if (params[0] == "OPTS")                                                 {
      if (params.size() == 3 && params[1].toUpper() == "UTF8")               {
        utf8Enabled = params [ 2 ] . toUpper ( ) == "ON"                     ;
        send ( "200 Command okay." )                                         ;
      } else send ( "550 Requested action not taken." )                      ;
    } else
    if ( params[0] == "LANG" )                                               {
      if (params.size() == 1) send("200 Using default language en_US") ;  else
      if (params.size() == 2)                                                {
        if ( params[1] == "en-US" || params[1] == "en" )                     {
          send ( "200 Using default language en_US" )                        ;
        } else                                                               {
          send ( "504 Unsupported parameter"        )                        ;
        }                                                                    ;
      } else send ( "501 Bad argument" )                                     ;
    } else
    if (logged)                                                              {
      if ( params[0] == "PWD"  )                                             {
        send ( "257 \"" + path.pwd() + "\"" )                                ;
      } else
      if ( params[0] == "RNFR" )                                             {
        renameFrom = param                                                   ;
        send ( "350 Requested file action pending further information." )    ;
      } else
      if ( params[0] == "RNTO" )                                             {
        if (path.rename(renameFrom, param))                                  {
          send ( "250 Requested file action okay, completed." )              ;
        } else                                                               {
          send ( "550 Requested action not taken."            )              ;
        }                                                                    ;
        renameFrom . clear ( )                                               ;
      } else
      if ( params[0] == "SITE" )                                             {
        if ( params.size() > 3 && params[1].toUpper() == "CHMOD" )           {
          _param = _param.right(_param.size() - params[1].size()).trimmed()  ;
          _param = _param.right(_param.size() - params[2].size()).trimmed()  ;
           param = toStringGuessEncoding ( _param            )               ;
           path  . chmod                 ( param , params[2] )               ;
           send ( "200 Command okay." )                                      ;
        } else                                                               {
          send  ( "202 Command not implemented, superfluous at this site." ) ;
        }                                                                    ;
      } else
      if ( params[0] == "CWD" )                                              {
        if ( params.size() >= 2 && path.cwd(param) )                         {
          send ( "250 Requested file action okay, completed." )              ;
        } else
        if ( params.size() == 1 && path.cwd("/") )                           {
          send ( "250 Requested file action okay, completed." )              ;
        } else send ( "550 Requested action not taken." )                    ;
      } else
      if ( params[0] == "CDUP" )                                             {
        if ( path.cdup() ) send ( "200 Command okay."               )        ;
                      else send ( "550 Requested action not taken." )        ;
      } else
      if ( params[0] == "RMD" )                                              {
        if ( params.size() >= 2 && path . rmd ( param ) )                    {
          send ( "250 Requested file action okay, completed." )              ;
        } else                                                               {
          send ( "550 Requested action not taken."            )              ;
        }                                                                    ;
      } else
      if ( params[0] == "DELE" )                                             {
        if ( params.size() >= 2 && path . dele ( param ) )                   {
          send ( "250 Requested file action okay, completed." )              ;
        } else                                                               {
          send ( "550 Requested action not taken."            )              ;
        }                                                                    ;
      } else
      if (params[0] == "MKD")                                                {
        if ( params.size() >= 1 && path . mkd ( param ) )                    {
          send ( "257 directory \"" + param + "\" created." )                ;
        } else                                                               {
          send ( "550 Requested action not taken."          )                ;
        }                                                                    ;
      } else
      if ( params[0] == "QUIT" )                                             {
        logged         = false                                               ;
        pass_accepted  = false                                               ;
        login_accepted = false                                               ;
        login          . clear ( )                                           ;
        send ( "221 Service closing control connection." )                   ;
        sock          -> close ( )                                           ;
      } else
      if ( params[0] == "SYST" )                                             {
        send ( "215 UNIX Type: L8" )                                         ;
      } else
      if ( params[0] == "TYPE" )                                             {
        send ( "200 Command okay." )                                         ;
      } else
      if ( params[0] == "PORT" )                                             {
        if ( params.size() > 1 )                                             {
          QStringList address = param . split ( ',' )                        ;
          if ( address.size() != 6 )                                         {
            send ( "501 Syntax error in parameters or arguments." )          ;
          } else                                                             {
            int     port = (address[4].toUInt() <<  8)                       |
                            address[5].toUInt()                              ;
            quint32 ipv4 = (address[0].toUInt() << 24)                       |
                           (address[1].toUInt() << 16)                       |
                           (address[2].toUInt() <<  8)                       |
                            address[3].toUInt()                              ;
            QHostAddress portAddr ( ipv4 )                                   ;
            killDTP ( )                                                      ;
            passv = new QTcpSocket ( this )                                  ;
            nConnect ( passv , SIGNAL ( connected     () )                   ,
                       this  , SLOT   ( portConnection() )                 ) ;
            nConnect ( passv , SIGNAL ( error(QAbstractSocket::SocketError)) ,
                       this  , SLOT   ( errorDTP      () )                 ) ;
            passv -> connectToHost ( portAddr , port )                       ;
            waitingForDTP = true                                             ;
            send ( "200 Command okay." )                                     ;
          }                                                                  ;
        } else                                                               {
          send ( "501 Syntax error in parameters or arguments." )            ;
        }                                                                    ;
      } else
      if ( params[0] == "EPRT" )                                             {
        if ( params.size() > 1 )                                             {
          QStringList address = param.split('|')                             ;
          if ( address.size() != 4 && address.size() != 3 )                  {
            send ( "501 Syntax error in parameters or arguments." )          ;
          } else                                                             {
            int shift = address.size() - 3                                   ;
            int port  = address.last() . toInt ( )                           ;
            QHostAddress portAddr ( address[1 + shift] )                     ;
            killDTP ( )                                                      ;
            passv = new QTcpSocket ( this )                                  ;
            nConnect ( passv , SIGNAL (connected     ())                     ,
                       this  , SLOT   (portConnection())                   ) ;
            nConnect ( passv , SIGNAL (error(QAbstractSocket::SocketError) ) ,
                       this  , SLOT   (errorDTP      ())                   ) ;
            passv -> connectToHost ( portAddr , port )                       ;
            waitingForDTP = true                                             ;
            send ( "200 Command okay." )                                     ;
          }                                                                  ;
        } else                                                               {
          send ( "501 Syntax error in parameters or arguments." )            ;
        }                                                                    ;
      } else
      if ( params[0] == "PASV" )                                             {
        if (mode != NONE)                                                    {
          send ( "550 Requested action not taken." )                         ;
        } else openPasv ( )                                                  ;
      } else
      if ( params[0] == "EPSV" )                                             {
        if (mode != NONE)                                                    {
          send ( "550 Requested action not taken." )                         ;
        } else openEpsv ( )                                                  ;
      } else
      if ( params[0] == "LIST" )                                             {
        cmdParam = param                                                     ;
        if ( passv ) list ( ) ; else                                         {
          nConnect ( this , SIGNAL(DTPconnected()) , this , SLOT(list()) )   ;
          openPasv ( )                                                       ;
        }                                                                    ;
      } else
      if ( params[0] == "NLST" )                                             {
        cmdParam = param                                                     ;
        if (passv) nlst ( ) ; else                                           {
          nConnect ( this , SIGNAL(DTPconnected()) , this , SLOT(nlst()) )   ;
          openPasv ( )                                                       ;
        }                                                                    ;
      } else
      if ( params[0] == "REST" )                                             {
        skip = param . toLongLong ( )                                        ;
        send ( "350 Requested file action pending further information." )    ;
      } else
      if ( params[0] == "RETR" )                                             {
        if ( mode != NONE )                                                  {
          send ( "550 Requested action not taken." )                         ;
        } else                                                               {
          if ( params.size() == 1 )                                          {
            send ( "500 'RETR' not understood" )                             ;
          } else                                                             {
            mode     = RETR                                                  ;
            filename = path . absolute ( param )                             ;
            start ( )                                                        ;
          }                                                                  ;
        }                                                                    ;
      } else
      if ( params[0] == "STOR" )                                             {
        if ( mode != NONE )                                                  {
          send ( "550 Requested action not taken." )                         ;
        } else                                                               {
          if (params.size() == 1)                                            {
            send ( "500 'STOR' not understood" )                             ;
          } else                                                             {
            if ( path . isWritable ( param ) )                               {
              mode     = STOR                                                ;
              filename = path . absolute ( param )                           ;
              start ( )                                                      ;
            } else                                                           {
              send("451 Requested action aborted. Local error in processing.");
            }                                                                ;
          }                                                                  ;
        }                                                                    ;
      } else
      if ( params[0] == "APPE" )                                             {
        if ( mode != NONE )                                                  {
          send ( "550 Requested action not taken." )                         ;
        } else                                                               {
          if ( params.size() == 1 )                                          {
            send ( "500 'APPE' not understood" )                             ;
          } else                                                             {
            if ( path.isWritable(param) )                                    {
              mode     = APPE                                                ;
              filename = path . absolute ( param )                           ;
              start ( )                                                      ;
            } else                                                           {
              send("451 Requested action aborted. Local error in processing.");
            }                                                                ;
          }                                                                  ;
        }                                                                    ;
      } else
      if ( params[0] == "SIZE" )                                             {
        if ( params.size() == 1 )                                            {
          send ( "500 'SIZE' not understood" )                               ;
        } else                                                               {
          if ( path.exists(param) )                                          {
            send ( "213 " + QString::number(path.size(param)) )              ;
          } else                                                             {
            send ( "451 Requested action aborted. Local error in processing." ) ;
          }                                                                  ;
        }                                                                    ;
      } else
      if ( params[0] == "MDTM" )                                             {
        if ( params.size() == 1 )                                            {
          send ( "500 'MDTM' not understood" )                               ;
        } else                                                               {
          if ( path.exists(param) )                                          {
            send ( "213 " + path.lastModified(param).toString("yyyyMMddhhmmss") ) ;
          } else                                                             {
            send ( "451 Requested action aborted. Local error in processing." ) ;
          }                                                                  ;
        }                                                                    ;
      } else
      if ( params[0] == "ABOR" )                                             {
        if ( mode != NONE )                                                  {
          mode = NONE                                                        ;
          send ( "226 Closing data connection." )                            ;
        } else
        if (passv                                                           &&
            passv->isValid()                                                &&
            passv->state() == QTcpSocket::ConnectedState                   ) {
          send ( "225 Data connection open; no transfer in progress." )      ;
        } else                                                               {
          killDTP ( )                                                        ;
          send    ( "226 Closing data connection." )                         ;
        }                                                                    ;
      } else
      if ( params[0] == "FIND" )                                             {
        if (mode != NONE)                                                    {
          send ( "550 Requested action not taken." )                         ;
        } else                                                               {
          if (params.size() == 1)                                            {
            send ( "500 'FIND' not understood" )                             ;
          } else                                                             {
            mode     = FIND                                                  ;
            filename = param                                                 ;
            start ( )                                                        ;
          }                                                                  ;
        }                                                                    ;
      } else                                                                 {
        send ( "502 Command not implemented." )                              ;
      }                                                                      ;
    } else                                                                   {
      send ( "550 Requested action not taken." )                             ;
    }                                                                        ;
  }                                                                          ;
}

void N::FtpThread::send(const QString & str)
{
  if (!sock) return                                                ;
  if (sock->isValid())                                             {
    if (utf8Enabled) sock -> write ( str . toUtf8   ( ) + "\r\n" ) ;
                else sock -> write ( str . toLatin1 ( ) + "\r\n" ) ;
  }                                                                ;
}

void N::FtpThread::openPasv(void)
{
  killDTP ( )                                                                  ;
  int     port    = serv -> serverPort   ( )                                   ;
  quint32 address = sock -> localAddress ( ) . toIPv4Address ( )               ;
  const QString msg = QString(                                                 )
                     .sprintf("227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)." ,
                              address >> 24                                    ,
                             (address >> 16) & 0xFF                            ,
                             (address >>  8) & 0xFF                            ,
                              address & 0xFF                                   ,
                             (port    >>  8) & 0xFF                            ,
                              port & 0xFF                                    ) ;
  waitingForDTP = true                                                         ;
  send ( msg )                                                                 ;
}

void N::FtpThread::openEpsv(void)
{
  killDTP                       ( )                                    ;
  int port = serv -> serverPort ( )                                    ;
  const QString msg = QString("229 Extended Passive Mode Entered |||") +
                      QString::number(port) + '|'                      ;
  waitingForDTP = true                                                 ;
  send ( msg )                                                         ;
}

void N::FtpThread::killDTP(void)
{
  if (passv)                 {
    passv -> close       ( ) ;
    passv -> deleteLater ( ) ;
    passv  = NULL            ;
  }                          ;
  mode = NONE                ;
  if (waitingForDTP)         {
    waitingForDTP = false    ;
    emit DTPconnected ( )    ;
  }                          ;
}

void N::FtpThread::errorDTP(void)
{
  killDTP ( ) ;
}

void N::FtpThread::portConnection(void)
{
  waitingForDTP = false ;
  emit DTPconnected ( ) ;
}

void N::FtpThread::pasvConnection(void)
{
  if (passv)                              {
    passv -> deleteLater ( )              ;
    passv  = NULL                         ;
  }                                       ;
  passv = serv->nextPendingConnection ( ) ;
  waitingForDTP = false                   ;
  emit DTPconnected ( )                   ;
}

void N::FtpThread::start(void)
{
  if ( ! passv ) return                                              ;
  emit activity ( )                                                  ;
  switch ( mode )                                                    {
    case FIND:                                                       {
      FINDcmd * cmd = new FINDcmd(uid,filename,this           )      ;
      nConnect( cmd , SIGNAL(activity()) , this , SLOT(activate()) ) ;
    }                                                                ;
    break                                                            ;
    case RETR:                                                       {
      RETRcmd * cmd = new RETRcmd(uid,filename,this           )      ;
      nConnect( cmd , SIGNAL(activity()) , this , SLOT(activate()) ) ;
    }                                                                ;
    break                                                            ;
    case STOR                                                        :
    case APPE:                                                       {
      STORcmd * cmd = new STORcmd(uid,filename,mode==APPE,this)      ;
      nConnect( cmd , SIGNAL(activity()) , this , SLOT(activate()) ) ;
    }                                                                ;
    break                                                            ;
    case NONE                                                        :
    break                                                            ;
  }                                                                  ;
}

void N::FtpThread::activate(void)
{
  emit activity ( ) ;
}

N::FtpThread::DTP_MODE N::FtpThread::getMode(void)
{
  return mode ;
}

QString N::FtpThread::getLogin(void)
{
  return login ;
}

QString N::FtpThread::getFilename(void)
{
  return filename ;
}

QString N::FtpThread::getAddress(void)
{
  return sock -> peerAddress ( ) . toString ( ) ;
}

void N::FtpThread::list(void)
{
  disconnect ( this , SLOT(list())                         ) ;
  if (passv)                                                 {
    LISTcmd * cmd = new LISTcmd(cmdParam,false,this)         ;
    connect ( cmd,SIGNAL(activity()),this,SLOT(activate()) ) ;
  } else                                                     {
    send    ( "426 Connection closed; transfer aborted." )   ;
    killDTP (                                            )   ;
  }                                                          ;
}

void N::FtpThread::nlst(void)
{
  disconnect ( this , SLOT(nlst())                         ) ;
  if (passv)                                                 {
    LISTcmd * cmd = new LISTcmd(cmdParam,true,this)          ;
    connect ( cmd,SIGNAL(activity()),this,SLOT(activate()) ) ;
  } else                                                     {
    send    ( "426 Connection closed; transfer aborted." )   ;
    killDTP (                                            )   ;
  }                                                          ;
}
