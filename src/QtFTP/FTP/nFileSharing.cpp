#include <qtftp.h>

N::FileSharing * N::FileSharing::pInstance = NULL;

N::FileSharing:: FileSharing ( void                 )
               : TcpSharing  ( "FtpSharing" , 31013 )
               , sock        ( NULL                 )
{
  //////////////////////////////////////////////////////
  pInstance = this                                     ;
  if (FtpSettings::isGui()) FtpSettings::connect(this) ;
  //////////////////////////////////////////////////////
  nConnect ( this , SIGNAL (newThread            ())   ,
             this , SLOT   (broadcastNewThread   ()) ) ;
  nConnect ( this , SIGNAL (removeThread         ())   ,
             this , SLOT   (broadcastRemoveThread()) ) ;
  nConnect ( this , SIGNAL (enterServerMode      ())   ,
             this , SLOT   (startService         ()) ) ;
  nConnect ( this , SIGNAL (enterClientMode      ())   ,
             this , SLOT   (startService         ()) ) ;
  nConnect ( this , SIGNAL (clientLeft           ())   ,
             this , SLOT   (removeClient         ()) ) ;
  //////////////////////////////////////////////////////
  totalSent        = 0                                 ;
  totalReceived    = 0                                 ;
  maxConnections   = 0                                 ;
  totalConnections = 0                                 ;
}

N::FileSharing::~FileSharing(void)
{
  emit closing ( ) ;
  pInstance = NULL ;
}

const QSet<QString> & N::FileSharing::getUsers(void) const
{
  return users ;
}

void N::FileSharing::startServer(void)
{
  if (NotNull(pInstance))                  {
    if (pInstance->serverRunning())        {
      delete pInstance                     ;
      pInstance = NULL                     ;
    }                                      ;
  }                                        ;
  //////////////////////////////////////////
  if (IsNull(pInstance))                   {
    pInstance = new FileSharing ( )        ;
  }                                        ;
  //////////////////////////////////////////
  pInstance  -> startService    ( )        ;
}

void N::FileSharing::stopServer(void)
{
  if (NotNull(pInstance)) delete pInstance ;
  pInstance = NULL                         ;
}

void N::FileSharing::restartServer(void)
{
  stopServer  ( ) ;
  startServer ( ) ;
}

bool N::FileSharing::serverRunning(void)
{
  return NotNull ( pInstance ) ;
}

void N::FileSharing::removeClient(void)
{
  users    . clear (                     ) ;
  validIDs . clear (                     ) ;
  send             ( "refresh user list" ) ;
  users . insert   ( FtpSettings::User() ) ;
  validIDs << FtpSettings::getUID ( )      ;
}

void N::FileSharing::startService(void)
{
  if (isServing() && IsNull(sock) )                              {
    //////////////////////////////////////////////////////////////
    sock = new QTcpServer ( this )                               ;
    nConnect ( sock , SIGNAL ( newConnection() )                 ,
               this , SLOT   ( addConnection() )               ) ;
    sock->listen(QHostAddress::Any,FtpSettings::getServerPort()) ;
    users    . clear  (                     )                    ;
    users    . insert ( FtpSettings::User() )                    ;
    validIDs . clear  (                     )                    ;
    validIDs << FtpSettings::getUID()                            ;
  } else                                                         {
    send ( "user" , FtpSettings::User() )                        ;
  }                                                              ;
}

QList<N::FileSharing::ConnectionInfo> N::FileSharing::getInfo(void)
{
  QList<ConnectionInfo> lInfo                            ;
  foreach ( QObject * obj , children() )                 {
    FtpThread * pThread = dynamic_cast<FtpThread *>(obj) ;
    if (IsNull(pThread)) continue                        ;
    ConnectionInfo info                                  ;
    info . user    = pThread -> getLogin   ( )           ;
    info . address = pThread -> getAddress ( )           ;
    switch (pThread->getMode())                          {
      case FtpThread::NONE                               :
        info.activity.clear()                            ;
      break                                              ;
      case FtpThread::RETR                               :
        info.activity = "RETR " + pThread->getFilename() ;
      break                                              ;
      case FtpThread::STOR                               :
        info.activity = "STOR " + pThread->getFilename() ;
      break                                              ;
      case FtpThread::APPE                               :
        info.activity = "APPE " + pThread->getFilename() ;
      break                                              ;
      case FtpThread::FIND                               :
        info.activity = "FIND " + pThread->getFilename() ;
      break                                              ;
    }                                                    ;
    lInfo << info                                        ;
  }                                                      ;
  return lInfo                                           ;
}

void N::FileSharing::addConnection(void)
{
  while (sock->hasPendingConnections())                 {
    QTcpSocket * s = sock->nextPendingConnection()      ;
    if (s)                                              {
      FtpThread * thread = new FtpThread ( s , this )   ;
      nConnect  ( thread , SIGNAL (destroyed ())        ,
                  this   , SLOT   (killThread())      ) ;
      ++totalConnections                                ;
    }                                                   ;
  }                                                     ;
  maxConnections = qMax(maxConnections,nbConnections()) ;
  emit newThread ( )                                    ;
}

void N::FileSharing::killThread(void)
{
  emit removeThread ( ) ;
}

int N::FileSharing::nbConnections(void)
{
  int n = 0                                 ;
  QObjectList list = children()             ;
  foreach (QObject *obj, list)              {
    if (dynamic_cast<FtpThread *>(obj)) ++n ;
  }                                         ;
  return n                                  ;
}

QList<N::FileSharing::ConnectionInfo> N::FileSharing::getClientsInfo(void)
{
  if (NotNull(pInstance))        {
    return pInstance->getInfo () ;
  }                              ;
  return QList<ConnectionInfo>() ;
}

void N::FileSharing::statsSend(int n)
{
  if (isServing()) send ( "statsSend" , n ) ;
  totalSent += n                            ;
}

void N::FileSharing::statsRecv(int n)
{
  if (isServing()) send ( "statsRecv" , n ) ;
  totalReceived += n                        ;
}

QStringList N::FileSharing::getServerInfo(void)
{
  QStringList lInfo                                            ;
  if (!pInstance)                                              {
    lInfo << QString ( )                                       ;
    lInfo << QString ( )                                       ;
    lInfo << QString ( )                                       ;
    lInfo << QString ( )                                       ;
    lInfo << QString ( )                                       ;
    return lInfo                                               ;
  }                                                            ;
  lInfo << QString :: number ( pInstance -> totalSent        ) ;
  lInfo << QString :: number ( pInstance -> totalReceived    ) ;
  lInfo << QString :: number ( pInstance -> nbConnections()  ) ;
  lInfo << QString :: number ( pInstance -> maxConnections   ) ;
  lInfo << QString :: number ( pInstance -> totalConnections ) ;
  return lInfo                                                 ;
}

void N::FileSharing::receive(const QByteArray & message)
{
  QDataStream stream ( message )              ;
  QString     type                            ;
  stream  >>  type                            ;
  /////////////////////////////////////////////
  if (type == "SIG newThread")                {
    if (!isServing()) emit newThread    ( )   ;
    return                                    ;
  }                                           ;
  /////////////////////////////////////////////
  if (type == "SIG removeThread")             {
    if (!isServing()) emit removeThread ( )   ;
    return                                    ;
  }                                           ;
  /////////////////////////////////////////////
  if (type == "statsSend")                    {
    int n                                     ;
    stream >> n                               ;
    totalSent += n                            ;
    return                                    ;
  }                                           ;
  /////////////////////////////////////////////
  if (type == "statsRecv")                    {
    int n                                     ;
    stream >> n                               ;
    totalReceived += n                        ;
    return                                    ;
  }                                           ;
  /////////////////////////////////////////////
  if (type == "user")                         {
    QString   u                               ;
    stream >> u                               ;
    users  << u                               ;
    return                                    ;
  }                                           ;
  /////////////////////////////////////////////
  if (type == "userid")                       {
    FtpUserID   uid                           ;
    stream   >> uid                           ;
    validIDs << uid                           ;
    return                                    ;
  }                                           ;
  /////////////////////////////////////////////
  if (type == "refresh user list")            {
    send ( "user"   , FtpSettings::User  () ) ;
    send ( "userid" , FtpSettings::getUID() ) ;
   return                                     ;
  }                                           ;
}

void N::FileSharing::broadcastNewThread(void)
{
  if (isServing()) send ( "SIG newThread" ) ;
}

void N::FileSharing::broadcastRemoveThread(void)
{
  if (isServing()) send ( "SIG removeThread" ) ;
}

N::FileSharing * N::FileSharing::instance(void)
{
  return pInstance ;
}

bool N::FileSharing::checkID(const FtpUserID & uid) const
{
  return validIDs . contains ( uid ) ;
}

bool N::FileSharing::knownLogin(const QString & login) const
{
  for (int i=0;i<validIDs.count();i++)             {
    if (validIDs[i].getLogin()==login) return true ;
  }                                                ;
  return false                                     ;
}

bool N::FileSharing::assureUser(QString login,QString password)
{
  if (login   .length()<=0) return false ;
  if (password.length()<=0) return false ;
  if (knownLogin(login)) return true     ;
  FtpUserID F(login,password)            ;
  validIDs << F                          ;
  users . insert ( login )               ;
  return true                            ;
}
