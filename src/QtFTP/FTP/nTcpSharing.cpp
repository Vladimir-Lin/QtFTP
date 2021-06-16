#include <qtftp.h>

N::TcpSharing:: TcpSharing      (const QString & name,const quint16 port,QObject * parent)
              : QObject         (                                                  parent)
              , port            (                                   port                 )
              , mutex           (QMutex::Recursive                                       )
              , bLinkInProgress (false                                                   )
{
  ///////////////////////////////////////////////////////////////////////
  client = new QTcpSocket       ( this                                ) ;
  server = new QTcpServer       ( this                                ) ;
  ///////////////////////////////////////////////////////////////////////
  #ifdef Q_OS_IOS
  #else
  lock   = new QSystemSemaphore ( name , 1 , QSystemSemaphore::Create ) ;
  #endif
  ///////////////////////////////////////////////////////////////////////
  nConnect ( client , SIGNAL (disconnected     ())                      ,
             this   , SLOT   (link             ())                    ) ;
  nConnect ( client , SIGNAL (error(QAbstractSocket::SocketError)     ) ,
             this   , SLOT   (link             ())                    ) ;
  nConnect ( client , SIGNAL (readyRead        ())                      ,
             this   , SLOT   (receive          ())                    ) ;
  nConnect ( server , SIGNAL (newConnection    ())                      ,
             this   , SLOT   (processConnection())                    ) ;
  ///////////////////////////////////////////////////////////////////////
  link     (                                                          ) ;
}

N::TcpSharing::~TcpSharing(void)
{
  #ifdef Q_OS_IOS
  #else
  delete lock ;
  #endif
}

void N::TcpSharing::handleDisconnection(void)
{
  emit clientLeft ( ) ;
}

void N::TcpSharing::processConnection(void)
{
  if ( ! server -> isListening           ()           ||
       ! server -> hasPendingConnections () ) return   ;
  QTcpSocket * sock = NULL                             ;
  while ( ( sock = server->nextPendingConnection() ) ) {
    nConnect ( sock , SIGNAL (disconnected       ())   ,
               this , SLOT   (handleDisconnection()) ) ;
    nConnect ( sock , SIGNAL (disconnected       ())   ,
               sock , SLOT   (deleteLater        ()) ) ;
    nConnect ( sock , SIGNAL (readyRead          ())   ,
               this , SLOT   (broadcast          ()) ) ;
  }                                                    ;
}

void N::TcpSharing::link(void)
{
  if (server->isListening()                        ) return  ;
  if (client->state()!=QTcpSocket::UnconnectedState) return  ;
  if (bLinkInProgress                              ) return  ;
  ////////////////////////////////////////////////////////////
  mutex . lock   ( )                                         ;
  bLinkInProgress = true                                     ;
  mutex . unlock ( )                                         ;
  ////////////////////////////////////////////////////////////
  #ifdef Q_OS_IOS
  #else
  lock   -> acquire ( )                                      ;
  #endif
  client -> connectToHost ( QHostAddress::LocalHost , port ) ;
  if (!client->waitForConnected(2000))                       {
    server -> listen ( QHostAddress::LocalHost , port )      ;
  }                                                          ;
  #ifdef Q_OS_IOS
  #else
  lock   -> release ( )                                      ;
  #endif
  ////////////////////////////////////////////////////////////
  mutex . lock   ( )                                         ;
  bLinkInProgress = false                                    ;
  mutex . unlock ( )                                         ;
  ////////////////////////////////////////////////////////////
  if ( isServing() )                                         {
    emit enterServerMode ( )                                 ;
  } else                                                     {
    emit enterClientMode ( )                                 ;
  }                                                          ;
}

bool N::TcpSharing::isServing(void) const
{
  return server -> isListening ( ) ;
}

void N::TcpSharing::broadcast(void)
{
  if (!isServing()) return                                           ;
  foreach ( QObject * obj , server->children() )                     {
    QTcpSocket * sock = dynamic_cast<QTcpSocket *>(obj)              ;
    if (!sock                                    ) continue          ;
    if (sock->state()!=QTcpSocket::ConnectedState) continue          ;
    while(sock->bytesAvailable())                                    {
      int len = 0                                                    ;
      if (sock->read((char*)&len, sizeof(int)) != sizeof(int)) break ;
      QByteArray data = sock->read(len)                              ;
      len -= data.length()                                           ;
      QTime timer                                                    ;
      timer.start()                                                  ;
      while(len && timer.elapsed() < 10000)                          {
        sock->waitForReadyRead()                                     ;
        QByteArray tmp = sock->read(len)                             ;
        len -= tmp.length()                                          ;
        data.append(tmp)                                             ;
      }                                                              ;
      send    ( data )                                               ;
      receive ( data )                                               ;
    }                                                                ;
  }                                                                  ;
}

void N::TcpSharing::receive(void)
{
  while ( client->bytesAvailable() )                              {
    int len = 0                                                   ;
    if (client->read((char*)&len,sizeof(len))!=sizeof(len)) break ;
    QByteArray data = client->read(len)                           ;
    len -= data.length()                                          ;
    QTime timer                                                   ;
    timer.start()                                                 ;
    while(len > 0 && timer.elapsed() < 10000)                     {
      client->waitForReadyRead()                                  ;
      QByteArray tmp = client->read(len)                          ;
      len -= tmp.length()                                         ;
      data.append(tmp)                                            ;
    }                                                             ;
    receive(data)                                                 ;
  }                                                               ;
}

void N::TcpSharing::send(const QByteArray & message)
{
  const int len = message.size()                              ;
  if (!isServing())                                           {
    if (client->state() == QTcpSocket::ConnectedState)        {
      while (client->bytesToWrite() > 0)                      {
        client->waitForBytesWritten()                         ;
      }                                                       ;
      client->write((const char*)&len, sizeof(int))           ;
      qint64 l = client->write(message)                       ;
      QTime timer                                             ;
      timer.start()                                           ;
      while(l < len && timer.elapsed() < 10000)               {
        while (client->bytesToWrite() > 0)                    {
          client->waitForBytesWritten()                       ;
        }                                                     ;
        l += client->write(message.right(len - l))            ;
      }                                                       ;
    }                                                         ;
    return                                                    ;
  }                                                           ;
  /////////////////////////////////////////////////////////////
  foreach ( QObject * obj , server->children() )              {
    QTcpSocket * sock = dynamic_cast<QTcpSocket*>(obj)        ;
    if (!sock) continue                                       ;
    if (sock->state() != QTcpSocket::ConnectedState) continue ;
    while (sock->bytesToWrite() > 0)                          {
      sock->waitForBytesWritten()                             ;
    }                                                         ;
    sock->write((const char*)&len,sizeof(int))                ;
    qint64 l = sock->write(message)                           ;
    QTime timer                                               ;
    timer.start()                                             ;
    while (l < len && timer.elapsed() < 10000)                {
      while (sock->bytesToWrite() > 0)                        {
        sock->waitForBytesWritten()                           ;
      }                                                       ;
      l += sock->write(message.right(len - l))                ;
    }                                                         ;
  }                                                           ;
}

void N::TcpSharing::send(const QString & message)
{
  QByteArray  data                                    ;
  QDataStream stream ( &data , QIODevice::WriteOnly ) ;
  stream << message                                   ;
  send ( data )                                       ;
}

void N::TcpSharing::send(const char * message)
{
  send ( QString ( message ) ) ;
}

void N::TcpSharing::waitForData(int msec)
{
  if (!isServing())                                                        {
    if (client->bytesAvailable() == 0) client -> waitForReadyRead ( msec ) ;
    if (client->bytesAvailable() >  0) receive                    (      ) ;
  } else                                                                   {
    bool ready = false                                                     ;
    foreach ( QObject * obj , server->children() )                         {
      QTcpSocket * sock = dynamic_cast<QTcpSocket *>(obj)                  ;
      if (!sock) continue                                                  ;
      if (sock->state() != QTcpSocket::ConnectedState) continue            ;
      if (sock->bytesAvailable() == 0) sock->waitForReadyRead(msec)        ;
      ready |= sock->bytesAvailable() > 0                                  ;
    }                                                                      ;
    if (ready) broadcast ( )                                               ;
  }                                                                        ;
}
