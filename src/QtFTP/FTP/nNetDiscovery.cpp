#include <qtftp.h>

#define NDport 18956

N::NetDiscovery * N::NetDiscovery::pInstance = NULL ;

N::NetDiscovery:: NetDiscovery ( void                    )
                : TcpSharing   ( "NetDiscovery" , NDport )
                , mutex        ( QMutex::Recursive       )
                , bRunning     ( false                   )
                , registrar    ( NULL                    )
{
  nConnect(this,SIGNAL(enterServerMode()),this,SLOT(start())) ;
  setAutoDelete ( false )                                     ;
  bStop              = false                                  ;
  bRefreshForced     = false                                  ;
  bSendNotifyRequest = false                                  ;
  start         (       )                                     ;
  forceRefresh  (       )                                     ;
}

N::NetDiscovery::~NetDiscovery(void)
{
  terminate ( )                               ;
  while(isRunning()) FtpDelay :: msleep ( 1 ) ;
  if (registrar) delete registrar             ;
}

N::NetDiscovery * N::NetDiscovery::instance(void)
{
  if (pInstance) return pInstance       ;
  return pInstance = new NetDiscovery() ;
}

void N::NetDiscovery::run(void)
{
  if ( ! isServing ( ) ) return                                     ;
  mutex . lock ( )                                                  ;
  if (bRunning)                                                     {
    mutex . unlock ( )                                              ;
    return                                                          ;
  }                                                                 ;
  ///////////////////////////////////////////////////////////////////
  bRunning = true                                                   ;
  mutex    . unlock ( )                                             ;
  ///////////////////////////////////////////////////////////////////
  mServer  . clear  ( )                                             ;
  emit serverListChanged ( )                                        ;
  ///////////////////////////////////////////////////////////////////
  int bport = NDport - 100                                          ;
  QUdpSocket sock                                                   ;
  sock . bind ( bport )                                             ;
  bSendNotifyRequest = true                                         ;
  ///////////////////////////////////////////////////////////////////
  quint32 n   = -1                                                  ;
  int     nTO =  0                                                  ;
  ///////////////////////////////////////////////////////////////////
  while ( ! hasToStop ( ) && isServing ( ) )                        {
    FtpDelay :: msleep ( 1000 )                                     ;
    ++n                                                             ;
    while ( sock . hasPendingDatagrams ( ) )                        {
      char         buf [ 1024 ]                                     ;
      QHostAddress host                                             ;
      memset ( buf , 0 , sizeof(buf) )                              ;
      sock . readDatagram ( buf , 1024 , &host )                    ;
      setOrUpdate ( buf , host )                                    ;
    }                                                               ;
    /////////////////////////////////////////////////////////////////
    if ( n >= 10 || bRefreshForced )                                {
      bRefreshForced = false                                        ;
      QString msg                                                   ;
      msg  = FtpSettings::getHostname()                             ;
      msg += '\n'                                                   ;
      msg += QString::number ( FtpSettings::getServerPort() )       ;
      sock.writeDatagram(msg.toLatin1(),QHostAddress(QHostAddress::Broadcast),bport) ;
      n = 0                                                         ;
    }                                                               ;
    /////////////////////////////////////////////////////////////////
    if ( bSendNotifyRequest )                                       {
      bSendNotifyRequest = false                                    ;
      sock . writeDatagram                                          (
        "REFRESH"                                                   ,
        QHostAddress ( QHostAddress::Broadcast )                    ,
        bport                                                     ) ;
    }                                                               ;
    /////////////////////////////////////////////////////////////////
    ++nTO                                                           ;
    if ( nTO >= 2 )                                                 {
      QVector<QString> vDead                                        ;
      for (QMap<QString,FtpComputer>::iterator i = mServer.begin()  ;
           i != mServer.end()                                       ;
           ++i                                                    ) {
        if (i->timedOut()) vDead << i . key ( )                     ;
      }                                                             ;
      if (!vDead.isEmpty())                                         {
        for (QVector<QString>::const_iterator i = vDead.begin()     ;
             i != vDead.end()                                       ;
             ++i                                                  ) {
          mServer . remove ( *i )                                   ;
        }                                                           ;
        emit serverListChanged ( )                                  ;
      }                                                             ;
      nTO = 0                                                       ;
    }                                                               ;
  }                                                                 ;
  ///////////////////////////////////////////////////////////////////
  mutex    . lock   ( )                                             ;
  bStop    = false                                                  ;
  bRunning = false                                                  ;
  mutex    . unlock ( )                                             ;
}

void N::NetDiscovery::terminate(void)
{
  mutex . lock   ( ) ;
  bStop = true       ;
  mutex . unlock ( ) ;
}

void N::NetDiscovery::setOrUpdate(const QString & data,const QHostAddress & host)
{
  if (!isServing()) return                                           ;
  QStringList params = data.split('\n', QString::SkipEmptyParts)     ;
  if (params.size() < 2)                                             {
    if (params.size() == 1 && params.front() == "REFRESH")           {
      bRefreshForced = true                                          ;
    }                                                                ;
    return                                                           ;
  }                                                                  ;
  ////////////////////////////////////////////////////////////////////
  FtpComputer server                                                 ;
  server . setHostname ( host.toString()   )                         ;
  server . setName     ( params[0]         )                         ;
  server . setPort     ( params[1].toInt() )                         ;
  ////////////////////////////////////////////////////////////////////
  bool changed = false                                               ;
  mutex . lock   ( )                                                 ;
  QString key = host . toString ( )                                  ;
  changed = ( !mServer.contains(key) ) || ( mServer[key] != server ) ;
  mServer [ key ] = server                                           ;
  send ( "SET" , key , server )                                      ;
  mutex . unlock ( )                                                 ;
  if (changed) emit serverListChanged ( )                            ;
}

void N::NetDiscovery::remove(const QHostAddress & host)
{
  if (isServing()) send ( "REMOVE" , host.toString() ) ;
  mutex . lock ( )                                     ;
  if ( mServer . contains ( host . toString ( ) ) )    {
    mServer . remove ( host.toString() )               ;
    mutex   . unlock (                 )               ;
    if (isServing()) emit serverListChanged ( )        ;
  } else mutex . unlock ( )                            ;
}

void N::NetDiscovery::restart(void)
{
  terminate ( )                                ;
  while (isRunning()) FtpDelay :: msleep ( 1 ) ;
  start     ( )                                ;
}

QList<N::FtpComputer> N::NetDiscovery::getServerList(void)
{
  QList<FtpComputer> lServer                                   ;
  mutex . lock   ( )                                           ;
  for (QMap<QString,FtpComputer>::iterator i = mServer.begin() ;
       i != mServer.end()                                      ;
       ++i                                                   ) {
    lServer . push_back ( i . value ( ) )                      ;
  }                                                            ;
  mutex . unlock ( )                                           ;
  return lServer                                               ;
}

void N::NetDiscovery::forceRefresh(void)
{
  if (!isServing())               {
    mServer . clear           ( ) ;
    emit serverListChanged    ( ) ;
    send ( "forceRefresh" )       ;
    return                        ;
  }                               ;
  mutex              . lock   ( ) ;
  mServer            . clear  ( ) ;
  bRefreshForced     = true       ;
  bSendNotifyRequest = true       ;
  emit serverListChanged      ( ) ;
  mutex              . unlock ( ) ;
}

N::FtpComputer N::NetDiscovery::getComputer(const QString & name)
{
  QMutexLocker mLock ( &mutex )                                 ;
  for ( QMap<QString,FtpComputer>::iterator i = mServer.begin() ;
        i != mServer.end()                                      ;
        ++i                                                   ) {
    if (i.value().getName() == name) return i.value()           ;
  }                                                             ;
  return FtpComputer ( )                                        ;
}

void N::NetDiscovery::receive(const QByteArray & message)
{
  QDataStream stream(message)                     ;
  QString   type                                  ;
  stream >> type                                  ;
  if (type == "SIG serverListChanged")            {
    if ( isServing ( ) ) return                   ;
    emit serverListChanged ( )                    ;
    return                                        ;
  }                                               ;
  if (type == "forceRefresh")                     {
    if ( isServing ( ) ) forceRefresh ( )         ;
    return                                        ;
  }                                               ;
  if (type == "REMOVE")                           {
    QString   host                                ;
    stream >> host                                ;
    remove ( QHostAddress ( host ) )              ;
    return                                        ;
  }                                               ;
  if (type == "SET")                              {
    QString     name                              ;
    FtpComputer computer                          ;
    stream >> name >> computer                    ;
    bool bChange = ( !mServer.contains(name)   ) ||
                   ( mServer[name] != computer )  ;
    mServer[name] = computer                      ;
    if (bChange) emit serverListChanged ( )       ;
    return                                        ;
  }                                               ;
}

void N::NetDiscovery::propagateServerListChanged(void)
{
  send ( "SIG serverListChanged" ) ;
}

void N::NetDiscovery::start(void)
{
  setAutoDelete ( false )                                            ;
  QThreadPool::globalInstance()->start(this,QThread::LowestPriority) ;
}

bool N::NetDiscovery::hasToStop(void)
{
  QMutexLocker mLock ( &mutex ) ;
  return bStop                  ;
}

bool N::NetDiscovery::isRunning(void)
{
  QMutexLocker mLock ( &mutex ) ;
  return bRunning               ;
}
