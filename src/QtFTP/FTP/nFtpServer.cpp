#include <qtftp.h>

#if defined(Q_OS_WIN)
//#define WIN32_LEAN_AND_MEAN
//#include <io.h>
//#include <direct.h>
//#include <winsock2.h>
//#include <process.h>
#else
#include <limits.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#endif

N::FtpServer:: FtpServer (void   )
             : Thread    (0,false)
             , TcpServer (       )
{
  Address . setPort ( 21 )                           ;
  Ports                      = RangeInt(10000,32000) ;
  bIsListening               = false                 ;
  bIsAccepting               = false                 ;
  uiNumberOfUser             = 0                     ;
  uiNumberOfClient           = 0                     ;
  ulNoTransferTimeout        = 0                     ; // No timeout.
  ulNoLoginTimeout           = 0                     ; // No timeout.
  uiCheckPassDelay           = 0                     ; // No pass delay.
  uiMaxPasswordTries         = 10                    ; // 3 pass tries before the client gets kicked.
  uiTransferBufferSize       = 32 * 1024             ;
  uiTransferSocketBufferSize = 64 * 1024             ;
  bEnableFXP                 = false                 ;
}

N::FtpServer::~FtpServer (void)
{
}

int N::FtpServer::type(void) const
{
  return 959 ;
}

bool N::FtpServer::isListening(void) const
{
  return bIsListening ;
}

bool N::FtpServer::isAccepting(void) const
{
  return bIsAccepting ;
}

unsigned short N::FtpServer::listeningPort (void) const
{
  return usListeningPort ;
}

bool N::FtpServer::isFXPEnabled(void) const
{
  return bEnableFXP ;
}

unsigned long int N::FtpServer::GetNoLoginTimeout(void) const
{
  return ulNoLoginTimeout ;
}

unsigned long int N::FtpServer::GetNoTransferTimeout(void) const
{
  return ulNoTransferTimeout ;
}

unsigned int N::FtpServer::GetCheckPassDelay(void) const
{
  return uiCheckPassDelay ;
}

unsigned int N::FtpServer::GetMaxPasswordTries(void) const
{
  return uiMaxPasswordTries ;
}

unsigned int N::FtpServer::GetTransferBufferSize(void) const
{
  return uiTransferBufferSize ;
}

unsigned int N::FtpServer::GetTransferSocketBufferSize(void) const
{
  return uiTransferSocketBufferSize ;
}

unsigned int N::FtpServer::GetNbClient(void) const
{
  return uiNumberOfClient ;
}

unsigned int N::FtpServer::GetNbUser(void) const
{
  return uiNumberOfUser ;
}

void N::FtpServer::SetNoLoginTimeout( unsigned long int ulSecond )
{
  ulNoLoginTimeout = ulSecond ;
}

void N::FtpServer::SetNoTransferTimeout( unsigned long int ulSecond )
{
  ulNoTransferTimeout = ulSecond ;
}

void N::FtpServer::SetCheckPassDelay( unsigned int ulMilliSecond )
{
  uiCheckPassDelay = ulMilliSecond ;
}

void N::FtpServer::SetMaxPasswordTries( unsigned int uiMaxPassTries )
{
  uiMaxPasswordTries = uiMaxPassTries ;
}

void N::FtpServer::EnableFXP( bool bEnable )
{
  bEnableFXP = bEnable ;
}

void N::FtpServer::SetTransferBufferSize( unsigned int uiSize )
{
  uiTransferBufferSize = uiSize ;
}

void N::FtpServer::SetTransferSocketBufferSize( unsigned int uiSize )
{
  uiTransferSocketBufferSize = uiSize ;
}

bool N::FtpServer::StopListening(void)
{
  if (!isListening()) return false  ;
  Connector    . close ( )          ;
  bIsListening = false              ;
//  OnServerEventCb( STOP_LISTENING ) ;
  return true                       ;
}

void N::FtpServer::run(void)
{
}
