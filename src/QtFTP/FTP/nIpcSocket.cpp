#include <qtftp.h>

N::IpcSocket:: IpcSocket (QObject * parent)
             : QObject   (          parent)
{
  sock = NULL                                     ;
  nConnect ( this , SIGNAL ( disconnecting () )   ,
             this , SLOT   ( resetState    () ) ) ;
}

N::IpcSocket:: IpcSocket (const QString & name,QObject * parent)
             : QObject   (                               parent)
{
  sock = NULL                                      ;
  nConnect  ( this , SIGNAL ( disconnecting () )   ,
              this , SLOT   ( resetState    () ) ) ;
  connectTo ( name                               ) ;
}

N::IpcSocket::~IpcSocket(void)
{
}

void N::IpcSocket::connectTo(const QString & name)
{
  emit disconnecting ( )                                                ;
  sock         = NULL                                                   ;
  currentState = NotConnected                                           ;
  ///////////////////////////////////////////////////////////////////////
  QLocalSocket * s = new QLocalSocket ( this )                          ;
  nConnect ( this , SIGNAL (disconnecting())                            ,
             s    , SLOT   (deleteLater  ())                          ) ;
  nConnect ( s    , SIGNAL (error(QLocalSocket::LocalSocketError))      ,
             this , SLOT   (resetState   ())                          ) ;
  nConnect ( s    , SIGNAL (destroyed    ())                            ,
             this , SLOT   (resetState   ())                          ) ;
  ///////////////////////////////////////////////////////////////////////
  s -> connectToServer ( name )                                         ;
  if ( s -> waitForConnected ( 2000 ) )                                 {
    IpcConnection * ipc = new IpcConnection ( s )                       ;
    nConnect ( ipc  , SIGNAL (message         (QByteArray) )            ,
               this , SLOT   (dispatchMessage (QByteArray) )          ) ;
    nConnect ( this , SIGNAL (sigSendMessage  (QByteArray) )            ,
               ipc  , SLOT   (sendMessage     (QByteArray) )          ) ;
    relayConnectedSignal ( )                                            ;
  } else                                                                {
    s -> deleteLater ( )                                                ;
    sock = new QLocalServer ( this )                                    ;
    nConnect ( sock , SIGNAL (newConnection    ())                      ,
               this , SLOT   (processConnection())                    ) ;
    nConnect ( this , SIGNAL (disconnecting    ())                      ,
               sock , SLOT   (deleteLater      ())                    ) ;
    QLocalServer :: removeServer ( name )                               ;
    if (sock->listen(name))                                             {
      nConnect ( sock , SIGNAL (destroyed ())                           ,
                 this , SLOT   (resetState())                         ) ;
      currentState = Server                                             ;
      emit connected ( )                                                ;
    }                                                                   ;
  }                                                                     ;
}

void N::IpcSocket::processConnection(void)
{
  if (!sock) return                                          ;
  while ( sock -> hasPendingConnections ( ) )                {
    QLocalSocket * s = sock -> nextPendingConnection ( )     ;
    IpcConnection * ipc = new IpcConnection ( s )            ;
    nConnect ( ipc  , SIGNAL (message        (QByteArray))   ,
               this , SLOT   (dispatchMessage(QByteArray)) ) ;
    nConnect ( this , SIGNAL (sigSendMessage (QByteArray))   ,
               ipc  , SLOT   (sendMessage    (QByteArray)) ) ;
  }
}

void N::IpcSocket::dispatchMessage(const QByteArray & msg)
{
  emit message ( msg ) ;
}

void N::IpcSocket::sendMessage(const QByteArray & msg)
{
  emit sigSendMessage ( msg ) ;
}

N::IpcSocket::State N::IpcSocket::state(void)
{
  return currentState ;
}

void N::IpcSocket::resetState(void)
{
  currentState = NotConnected ;
}

void N::IpcSocket::relayConnectedSignal(void)
{
  currentState = Client ;
  emit connected ( )    ;
}
