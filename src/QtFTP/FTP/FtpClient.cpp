#include <qtftp.h>

N::FtpClient:: FtpClient     ( QObject * parent )
             : QObject       (           parent )
             , commands      ( NULL             )
             , ProgressIndex ( 0                )
             , UserAnswer    ( 1                )
{
  nConnect ( this , SIGNAL ( PostMessage   (QString) )   ,
             this , SLOT   ( acceptMessage (QString) ) ) ;
}

N::FtpClient::~FtpClient(void)
{
}

void N::FtpClient::acceptMessage(QString message)
{
  emit Message ( message ) ;
}

QByteArray N::FtpClient::Response(void)
{
  if (IsNull(commands)) return QByteArray ( ) ;
  char * response = commands->Response()      ;
  if (IsNull(response)) return QByteArray ( ) ;
  return QByteArray ( response )              ;
}

int N::FtpClient::Callback(FtpCommands * commands)
{ Q_UNUSED ( commands ) ;
  return UserAnswer     ;
}

void N::FtpClient::Error(int code,FtpCommands * commands)
{ Q_UNUSED ( commands ) ;
  Q_UNUSED ( code     ) ;
}

void N::FtpClient::Sending(QString cmd,FtpCommands * commands)
{
  if ( commands->CmdID == 0                 ) return ;
  if ( commands->CmdID == FtpCommands::PASS ) return ;
  if ( cmd.length()    <= 0                 ) return ;
  LogMutex . lock   ( )                              ;
  Loggings << cmd                                    ;
  LogMutex . unlock ( )                              ;
}

void N::FtpClient::doProgress(quint64 index,FtpCommands * commands)
{ Q_UNUSED ( commands ) ;
  ProgressIndex = index ;
}

void N::FtpClient::acceptResponse(FtpCommands * commands)
{
  char * response = commands->Response(  ) ;
  if (IsNull(response)) return             ;
  QByteArray B ( response )                ;
  QString    S = QString :: fromUtf8 ( B ) ;
  if (S.length()<=0) return                ;
  LogMutex . lock   ( )                    ;
  Loggings << S                            ;
  LogMutex . unlock ( )                    ;
}

bool N::FtpClient::exists(void)
{
  return NotNull ( commands ) ;
}

bool N::FtpClient::connectFtp(QString hostname,int port)
{
  QMutexLocker locker ( &mutex )                                ;
  if (NotNull(commands)) delete commands                        ;
  commands = new FtpCommands ( this )                           ;
  if (IsNull (commands)) return false                           ;
  if (!commands->connectTo(hostname.toUtf8().constData(),port)) {
    delete commands                                             ;
    commands = NULL                                             ;
    return false                                                ;
  }                                                             ;
  return true                                                   ;
}

bool N::FtpClient::Login(QString username,QString password)
{
  QMutexLocker locker ( &mutex )           ;
  if (IsNull(commands)) return false       ;
  return commands->Login                   (
           username.toUtf8().constData()   ,
           password.toUtf8().constData() ) ;
}

bool N::FtpClient::Quit(void)
{
  QMutexLocker locker ( &mutex )    ;
  if (IsNull(commands)) return true ;
  bool r = commands->Quit()         ;
  delete commands                   ;
  commands = NULL                   ;
  return r                          ;
}

bool N::FtpClient::Site(QString command)
{
  QMutexLocker locker ( &mutex )                      ;
  if (IsNull(commands)) return false                  ;
  return commands->Site(command.toUtf8().constData()) ;
}

QString N::FtpClient::SystemType (void)
{
  QMutexLocker locker ( &mutex )            ;
  if (IsNull(commands)) return ""           ;
  char BUFX[8192]                           ;
  memset ( BUFX , 0 , 8192 )                ;
  if (!commands->SYST(BUFX,8190)) return "" ;
  return QString(BUFX)                      ;
}

bool N::FtpClient::MakeDirectory(QString path)
{
  QMutexLocker locker ( &mutex )                      ;
  if (IsNull(commands)) return false                  ;
  return commands->MakeDir(path.toUtf8().constData()) ;
}

bool N::FtpClient::ChangeDirectory(QString path)
{
  QMutexLocker locker ( &mutex )                        ;
  if (IsNull(commands)) return false                    ;
  return commands->ChangeDir(path.toUtf8().constData()) ;
}

bool N::FtpClient::CdUp(void)
{
  QMutexLocker locker ( &mutex )     ;
  if (IsNull(commands)) return false ;
  return commands->CdUp()            ;
}

bool N::FtpClient::RemoveDirectory(QString path)
{
  QMutexLocker locker ( &mutex )                        ;
  if (IsNull(commands)) return false                    ;
  return commands->RemoveDir(path.toUtf8().constData()) ;
}

QString N::FtpClient::PWD(void)
{
  QMutexLocker locker ( &mutex )           ;
  if (IsNull(commands)) return ""          ;
  char BUFX[8192]                          ;
  memset ( BUFX , 0 , 8192 )               ;
  if (!commands->PWD(BUFX,8190)) return "" ;
  return QString(BUFX)                     ;
}

bool N::FtpClient::Filesize(QString path,quint64 & size)
{
  QMutexLocker locker ( &mutex )                            ;
  if (IsNull(commands)) return false                        ;
  return commands->Size(path.toUtf8().constData(),size,'I') ;
}

bool N::FtpClient::LastModified(QString path,QDateTime & modification)
{
  QMutexLocker locker ( &mutex )                                               ;
  if (IsNull(commands)) return false                                           ;
  char DTF[1024]                                                               ;
  memset ( DTF , 0 , 1024 )                                                    ;
  if (!commands->ChangedDate(path.toUtf8().constData(),DTF,1020)) return false ;
  //////////////////////////////////////////////////////////////////////////////
  // DTF => QDateTime
  //////////////////////////////////////////////////////////////////////////////
  return true                                                                  ;
}

bool N::FtpClient::Rename(QString source,QString target)
{
  QMutexLocker locker ( &mutex )         ;
  if (IsNull(commands)) return false     ;
  return commands->Rename                (
           source.toUtf8().constData()   ,
           target.toUtf8().constData() ) ;
}

bool N::FtpClient::Delete(QString filename)
{
  QMutexLocker locker ( &mutex )                         ;
  if (IsNull(commands)) return false                     ;
  return commands->Delete(filename.toUtf8().constData()) ;
}

bool N::FtpClient::NLST(QIODevice & io,QString path)
{
  QMutexLocker locker ( &mutex )                      ;
  if (IsNull(commands)) return false                  ;
  return commands->nLst(io,path.toUtf8().constData()) ;
}

bool N::FtpClient::List(QIODevice & io,QString path)
{
  QMutexLocker locker ( &mutex )                      ;
  if (IsNull(commands)) return false                  ;
  return commands->List(io,path.toUtf8().constData()) ;
}

void N::FtpClient::FixupDateTime(QDateTime & dateTime)
{ // Adjust for future tolerance.
  const int futureTolerance = 86400                  ;
  int       ds                                       ;
  ds = dateTime.secsTo(QDateTime::currentDateTime()) ;
  if ( ds >= -futureTolerance ) return               ;
  QDate d = dateTime . date ( )                      ;
  d . setDate ( d.year() - 1 , d.month() , d.day() ) ;
  dateTime . setDate ( d )                           ;
}

bool N::FtpClient::List(QString path,QList<QUrlInfo> & files,QString username)
{
  QMutexLocker locker ( &mutex )               ;
  if (IsNull(commands)) return false           ;
  QByteArray Body                              ;
  QBuffer    Buffer ( &Body )                  ;
  bool       correct                           ;
  char * ppp = NULL                            ;
  char   bbp [ 1024 ]                          ;
  if (path.length()>0)                         {
    ppp = bbp                                  ;
    memset ( bbp , 0 , 1024                  ) ;
    strcpy ( bbp , path.toUtf8().constData() ) ;
  }                                            ;
  correct = commands->List(Buffer,ppp)         ;
  files . clear ( )                            ;
  if ( !correct       ) return false           ;
  if ( Body.size()<=0 ) return false           ;
  QString     B = QString::fromUtf8(Body)      ;
  QStringList L = B.split('\n')                ;
  QString     S                                ;
  foreach (S,L)                                {
    B = S.trimmed()                            ;
    if (B.length()>0)                          {
      QUrlInfo info                            ;
      if (ParseDir(B,username,info))           {
        files << info                          ;
      }                                        ;
    }                                          ;
  }                                            ;
  return correct                               ;
}

bool N::FtpClient::Get(QIODevice & io,QString path)
{
  QMutexLocker locker ( &mutex )                         ;
  if (IsNull(commands)) return false                     ;
  return commands->Get(io,path.toUtf8().constData(),'I') ;
}

bool N::FtpClient::Put(QIODevice & io,QString path)
{
  QMutexLocker locker ( &mutex )                         ;
  if (IsNull(commands)) return false                     ;
  return commands->Put(io,path.toUtf8().constData(),'I') ;
}

bool N::FtpClient::ParseUnixDir     (
       const QStringList & tokens   ,
       const QString     & userName ,
       QUrlInfo          & info     )
{
  // Unix style, 7 + 1 entries
  // -rw-r--r--  1 ftp  ftp  17358091 Aug 10  2004 qt-x11-free-3.3.3.tar.gz
  // drwxr-xr-x  3 ftp  ftp      4096 Apr 14  2000 compiled-examples
  // lrwxrwxrwx  1 ftp  ftp         9 Oct 29  2005 qtscape -> qtmozilla
  ////////////////////////////////////////////////////////////////////////////
  if ( tokens.size() != 8 ) return false                                     ;
  ////////////////////////////////////////////////////////////////////////////
  char first = tokens.at(1).at(0).toLatin1()                                 ;
  if (first == 'd')                                                          {
    info . setDir     ( true  )                                              ;
    info . setFile    ( false )                                              ;
    info . setSymLink ( false )                                              ;
  } else
  if (first == '-')                                                          {
    info . setDir     ( false )                                              ;
    info . setFile    ( true  )                                              ;
    info . setSymLink ( false )                                              ;
  } else
  if (first == 'l')                                                          {
    info . setDir     ( true  )                                              ;
    info . setFile    ( false )                                              ;
    info . setSymLink ( true  )                                              ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  // Resolve filename
  QString name = tokens.at(7)                                                ;
  if (info.isSymLink())                                                      {
    int linkPos = name.indexOf(QLatin1String(" ->"))                         ;
    if (linkPos != -1) name.resize(linkPos)                                  ;
  }                                                                          ;
  info.setName(name)                                                         ;
  ////////////////////////////////////////////////////////////////////////////
  // Resolve owner & group
  info.setOwner(tokens.at(3))                                                ;
  info.setGroup(tokens.at(4))                                                ;
  ////////////////////////////////////////////////////////////////////////////
  // Resolve size
  info.setSize(tokens.at(5).toLongLong())                                    ;
  ////////////////////////////////////////////////////////////////////////////
  QStringList formats                                                        ;
  formats << QLatin1String("MMM dd  yyyy")
          << QLatin1String("MMM dd hh:mm")
          << QLatin1String("MMM  d  yyyy")
          << QLatin1String("MMM  d hh:mm")
          << QLatin1String("MMM  d yyyy" )
          << QLatin1String("MMM dd yyyy" )                                   ;
  ////////////////////////////////////////////////////////////////////////////
  QString dateString = tokens.at(6)                                          ;
  dateString[0] = dateString[0].toUpper()                                    ;
  // Resolve the modification date by parsing all possible formats
  QDateTime dateTime                                                         ;
  int n = 0;
  #ifndef QT_NO_DATESTRING
  do                                                                         {
    dateTime = QLocale::c().toDateTime(dateString, formats.at(n++))          ;
  }  while (n < formats.size() && (!dateTime.isValid()))                     ;
  #endif
  if (n == 2 || n == 4)                                                      {
    // Guess the year.
    dateTime.setDate ( QDate ( QDate::currentDate().year()                   ,
                               dateTime.date().month()                       ,
                               dateTime.date().day  ())                    ) ;
    FixupDateTime    ( dateTime                                            ) ;
  }                                                                          ;
  if (dateTime.isValid()) info.setLastModified(dateTime)                     ;
  ////////////////////////////////////////////////////////////////////////////
  // Resolve permissions
  int     permissions = 0                                                    ;
  QString p           = tokens.at(2)                                         ;
  permissions |= ( p[0] == QLatin1Char('r') ? QUrlInfo::ReadOwner  : 0 )     ;
  permissions |= ( p[1] == QLatin1Char('w') ? QUrlInfo::WriteOwner : 0 )     ;
  permissions |= ( p[2] == QLatin1Char('x') ? QUrlInfo::ExeOwner   : 0 )     ;
  permissions |= ( p[3] == QLatin1Char('r') ? QUrlInfo::ReadGroup  : 0 )     ;
  permissions |= ( p[4] == QLatin1Char('w') ? QUrlInfo::WriteGroup : 0 )     ;
  permissions |= ( p[5] == QLatin1Char('x') ? QUrlInfo::ExeGroup   : 0 )     ;
  permissions |= ( p[6] == QLatin1Char('r') ? QUrlInfo::ReadOther  : 0 )     ;
  permissions |= ( p[7] == QLatin1Char('w') ? QUrlInfo::WriteOther : 0 )     ;
  permissions |= ( p[8] == QLatin1Char('x') ? QUrlInfo::ExeOther   : 0 )     ;
  info . setPermissions ( permissions )                                      ;
  ////////////////////////////////////////////////////////////////////////////
  bool isOwner = info.owner() == userName                                    ;
  info.setReadable((permissions & QUrlInfo::ReadOther ) || ((permissions & QUrlInfo::ReadOwner ) && isOwner)) ;
  info.setWritable((permissions & QUrlInfo::WriteOther) || ((permissions & QUrlInfo::WriteOwner) && isOwner)) ;
  ////////////////////////////////////////////////////////////////////////////
  return true                                                                ;
}

bool N::FtpClient::ParseDosDir     (
       const QStringList & tokens   ,
       const QString     & userName ,
       QUrlInfo          & info     )
{
  // DOS style, 3 + 1 entries
  // 01-16-02  11:14AM       <DIR>          epsgroup
  // 06-05-03  03:19PM                 1973 readme.txt
  //////////////////////////////////////////////////////////////////////////
  if ( tokens.size() != 4 ) return false                                   ;
  Q_UNUSED ( userName )                                                    ;
  //////////////////////////////////////////////////////////////////////////
  QString name = tokens.at(3);
  info . setName    ( name                                           )     ;
  info . setSymLink ( name.toLower().endsWith(QLatin1String(".lnk")) )     ;
  //////////////////////////////////////////////////////////////////////////
  if (tokens.at(2) == QLatin1String("<DIR>"))                              {
    info . setFile ( false                     )                           ;
    info . setDir  ( true                      )                           ;
  } else                                                                   {
    info . setFile ( true                      )                           ;
    info . setDir  ( false                     )                           ;
    info . setSize ( tokens.at(2).toLongLong() )                           ;
  }                                                                        ;
  //////////////////////////////////////////////////////////////////////////
  // Note: We cannot use QFileInfo; permissions are for the server-side
  // machine, and QFileInfo's behavior depends on the local platform.
  int permissions = QUrlInfo::ReadOwner                                    |
                    QUrlInfo::WriteOwner                                   |
                    QUrlInfo::ReadGroup                                    |
                    QUrlInfo::WriteGroup                                   |
                    QUrlInfo::ReadOther                                    |
                    QUrlInfo::WriteOther                                   ;
  QString ext                                                              ;
  int extIndex = name.lastIndexOf(QLatin1Char('.'))                        ;
  if (extIndex != -1) ext = name.mid(extIndex + 1)                         ;
  if (ext == QLatin1String("exe")                                         ||
      ext == QLatin1String("bat")                                         ||
      ext == QLatin1String("com")                                        ) {
    permissions |= QUrlInfo::ExeOwner                                      |
                   QUrlInfo::ExeGroup                                      |
                   QUrlInfo::ExeOther                                      ;
  }                                                                        ;
  info . setPermissions ( permissions   )                                  ;
  info . setReadable    ( true          )                                  ;
  info . setWritable    ( info.isFile() )                                  ;
  //////////////////////////////////////////////////////////////////////////
  QDateTime dateTime                                                       ;
  #ifndef QT_NO_DATESTRING
  dateTime = QLocale::c().toDateTime(tokens.at(1), QLatin1String("MM-dd-yy  hh:mmAP")) ;
  if (dateTime.date().year() < 1971)                                       {
    dateTime.setDate(QDate(dateTime.date().year () + 100                   ,
                           dateTime.date().month()                         ,
                           dateTime.date().day  ())                      ) ;
  }                                                                        ;
  #endif
  info . setLastModified ( dateTime )                                      ;
  //////////////////////////////////////////////////////////////////////////
  return true                                                              ;
}

bool N::FtpClient::ParseDir     (
       const QString & buffer   ,
       const QString & userName ,
       QUrlInfo      & info     )
{
  if (buffer.length()<=0) return false                                    ;
  // Unix style FTP servers
  QRegExp unixPattern                                                     (
            QLatin1String                                                 (
              "^([\\-dl])([a-zA-Z\\-]{9,9})\\s+\\d+\\s+(\\S*)\\s+"
              "(\\S*)\\s+(\\d+)\\s+(\\S+\\s+\\S+\\s+\\S+)\\s+(\\S.*)" ) ) ;
  if (unixPattern.indexIn(buffer) == 0)                                   {
    if (ParseUnixDir ( unixPattern.capturedTexts() , userName , info ))   {
      return true                                                         ;
    }                                                                     ;
  }                                                                       ;
  /////////////////////////////////////////////////////////////////////////
  // DOS style FTP servers
  QRegExp dosPattern                                                      (
            QLatin1String                                                 (
              "^(\\d\\d-\\d\\d-\\d\\d\\d?\\d?\\ \\ \\d\\d:\\d\\d[AP]M)\\s+"
              "(<DIR>|\\d+)\\s+(\\S.*)$"                              ) ) ;
  if (dosPattern.indexIn(buffer) == 0)                                    {
    if (ParseDosDir ( dosPattern.capturedTexts() , userName , info ) )    {
      return true                                                         ;
    }                                                                     ;
  }                                                                       ;
  /////////////////////////////////////////////////////////////////////////
  return false                                                            ;
}
