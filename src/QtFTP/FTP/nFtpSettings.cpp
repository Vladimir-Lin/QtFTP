#include <qtftp.h>

N::FtpSettings * N::FtpSettings::pInstance = NULL ;

N::FtpSettings:: FtpSettings ( void  )
               : Gui         ( false )
               , Anonymous   ( false )
               , ReadOnly    ( true  )
               , HiddenFiles ( false )
               , ServerPort  ( 2000  )
{
  if (IsNull(pInstance)) pInstance = this ;
  SystemUser = DefaultUser ( )            ;
}

N::FtpSettings::~FtpSettings (void)
{
}

N::FtpSettings * N::FtpSettings::instance(void)
{
  if (IsNull(pInstance)) pInstance = new FtpSettings() ;
  return pInstance                                     ;
}

N::FileSharing * N::FtpSettings::server(void)
{
  FileSharing * sharing = FileSharing::instance() ;
  if (IsNull(sharing)) return new FileSharing()   ;
  return sharing                                  ;
}

void N::FtpSettings::setInstance(FtpSettings * settings)
{
  pInstance = settings ;
}

bool N::FtpSettings::isGui(void)
{
  nKickOut ( IsNull(pInstance) , false ) ;
  return pInstance->Gui                  ;
}

QString N::FtpSettings::DefaultUser(void)
{
  QStringList SE = QProcess :: systemEnvironment ( ) ;
  QString     U  = ""                                ;
  QString     S                                      ;
  QString     B                                      ;
  #ifdef Q_OS_WIN
  B = "USERNAME="                                    ;
  #else
  B = "USER="                                        ;
  #endif
  foreach ( S , SE )                                 {
    if (S.contains(B))                               {
      U = S                                          ;
      U = U . replace ( B    , "" )                  ;
      U = U . replace ( "\r" , "" )                  ;
      U = U . replace ( "\n" , "" )                  ;
    }                                                ;
  }                                                  ;
  return U                                           ;
}

QString N::FtpSettings::User(void)
{
  nKickOut ( IsNull(pInstance) , "" ) ;
  return pInstance->SystemUser        ;
}

bool N::FtpSettings::connect(FileSharing * sharing)
{
  nKickOut ( IsNull ( sharing   ) , false )      ;
  nKickOut ( IsNull ( pInstance ) , false )      ;
  return pInstance -> connectSharing ( sharing ) ;
}

bool N::FtpSettings::connectSharing(FileSharing * sharing)
{ Q_UNUSED ( sharing ) ;
//  connect(this, SIGNAL(newThread()), Stats::instance(), SLOT(updateStats()));
//  connect(this, SIGNAL(closing()), Stats::instance(), SLOT(updateStats()));
//  connect(this, SIGNAL(removeThread()), Stats::instance(), SLOT(updateStats()));
  return false         ;
}

bool N::FtpSettings::connect(FtpThread * thread)
{
  nKickOut ( IsNull ( thread    ) , false )    ;
  nKickOut ( IsNull ( pInstance ) , false )    ;
  return pInstance -> connectThread ( thread ) ;
}

bool N::FtpSettings::connectThread(FtpThread * thread)
{ Q_UNUSED ( thread ) ;
//  nConnect ( this              , SIGNAL ( activity   () )     ,
//             Stats::instance() , SLOT   ( updateStats() )   ) ;
  return false        ;
}

bool N::FtpSettings::Load(QMap<QString,FtpFolder> & folders)
{
#ifdef XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  Config::lock();
  int n = settings->beginReadArray("folders");
  folders.clear();
  for(int i = 0 ; i < n ; ++i)
  {
    settings->setArrayIndex(i);
    Folder folder(settings->value("path").toString(),
                  settings->value("writable").toBool());
                  folders[settings->value("name").toString()] = folder;
  }
  settings->endArray();
  Config::unlock();
#endif
  return false ;
}

bool N::FtpSettings::Save(QMap<QString,FtpFolder> & folders)
{
#ifdef XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  Config::lock();
  settings->beginWriteArray("folders");
  int e(0);
  for (QMap<QString,FtpFolder>::iterator i = folders.begin() ;
       i != folders.end()                                    ;
       ++i                                                 ) {
    settings->setArrayIndex(e++);
    settings->setValue("path", i.value().getPath());
    settings->setValue("writable", i.value().isWritable());
    settings->setValue("name", i.key());
  }
  settings->endArray();
  Config::unlock();
#endif
  return false ;
}

bool N::FtpSettings::isAnonymous(void)
{
  nKickOut ( IsNull(pInstance) , false ) ;
  return pInstance->Anonymous            ;
}

bool N::FtpSettings::isAnonymousReadAllowed(void)
{
  nKickOut ( IsNull(pInstance) , true ) ;
  return pInstance->ReadOnly            ;
}

bool N::FtpSettings::showHiddenFiles(void)
{
  nKickOut ( IsNull(pInstance) , false ) ;
  return pInstance->HiddenFiles          ;
}

bool N::FtpSettings::checkPassword(const QString & passwd)
{
  nKickOut ( IsNull(pInstance) , false )   ;
  return pInstance->verifyPassword(passwd) ;
}

bool N::FtpSettings::verifyPassword(const QString & passwd)
{
  QMutexLocker mLock ( &Mutex )                         ;
  QByteArray   password                                 ;
  bool         isEncrypted                              ;
  isEncrypted = Password ( password )                   ;
  if (isEncrypted)                                      {
    return ( password                                  ==
             QCryptographicHash::hash                   (
               passwd . toUtf8 ( )                      ,
               QCryptographicHash::Sha1 ) . toHex ( ) ) ;
  }                                                     ;
  return ( password == passwd.toUtf8() )                ;
}

bool N::FtpSettings::Password(QByteArray & password)
{
#ifdef XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  settings->beginGroup("config");
  const QByteArray password = settings->value("password").toByteArray();
  const bool isEncrypted = settings->value("encrypted", false).toBool();
  settings->endGroup();
  if (isEncrypted)
          return password == QCryptographicHash::hash(passwd.toUtf8(), QCryptographicHash::Sha1).toHex();
  else
          return password == passwd.toUtf8();
#endif
  return false ;
}

QByteArray N::FtpSettings::Encrypted(void)
{
  QByteArray password                 ;
  bool       isEncrypted              ;
  isEncrypted = Password ( password ) ;
  if (!isEncrypted)                   {
    password = QCryptographicHash::hash(password,QCryptographicHash::Sha1).toHex() ;
  }                                   ;
  return password                     ;
}

QString N::FtpSettings::getLogin(void)
{
  nKickOut ( IsNull(pInstance) , "" ) ;
  return pInstance->Login()           ;
}

QString N::FtpSettings::Login(void)
{
  return "" ;
}

QString N::FtpSettings::getHostname(void)
{
  nKickOut ( IsNull(pInstance) , QHostInfo::localHostName() ) ;
  return pInstance->Hostname()                                ;
}

QString N::FtpSettings::Hostname(void)
{
  return QHostInfo::localHostName() ;
}

int N::FtpSettings:: getServerPort(void)
{
  nKickOut ( IsNull(pInstance) , 2000 ) ;
  return pInstance->ServerPort          ;
}

N::FtpUserID N::FtpSettings::getUID(void)
{
  FtpUserID ID                        ;
  nKickOut ( IsNull(pInstance) , ID ) ;
  return pInstance -> ActualUID ( )   ;
}

N::FtpUserID N::FtpSettings::ActualUID(void)
{
  if (isAnonymous())                         {
    return FtpUserID("anonymous", QString()) ;
  }                                          ;
  QMutexLocker mLock ( &Mutex )              ;
  return FtpUserID ( Login() , Encrypted() ) ;
}

bool N::FtpSettings::checkID(const FtpUserID & uid,const bool read)
{
  nKickOut ( IsNull(pInstance) , false )                       ;
  if (pInstance->isAnonymous()                   ) return true ;
  if (read && pInstance->isAnonymousReadAllowed()) return true ;
  return getUID() == uid                                       ;
}

bool N::FtpSettings::addFolder(QString source,QString ftp,bool writable)
{
  nKickOut                         ( IsNull(pInstance) , false ) ;
  return pInstance -> appendFolder ( source , ftp , writable   ) ;
}

bool N::FtpSettings::appendFolder(QString source,QString ftp,bool writable)
{
  FtpFolder F ( source , writable )              ;
  FtpSharedFolders::instance()->addFolder(ftp,F) ;
  return true                                    ;
}
