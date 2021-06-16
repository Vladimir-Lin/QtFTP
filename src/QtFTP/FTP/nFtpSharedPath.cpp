#include <qtftp.h>

inline QString getOwner(const QFileInfo & info)
{
  return info.owner().isEmpty()                                     ?
        (info.ownerId() != uint(-2) ? QString::number(info.ownerId())
                                    : QString("owner")              )
                                    : info . owner ( )              ;
}

inline QString getGroup(const QFileInfo &info)
{
  return info.group().isEmpty()                                      ?
        (info.groupId() != uint(-2)  ? QString::number(info.groupId())
                                     : QString("group")              )
                                     : info . group ( )              ;
}

N::FtpSharedPath * N::FtpSharedPath::pInstance = NULL ;

const QChar N::FtpSharedPath::separator('/') ;

/////////////////////////////////////////////////////////////////////////

N::FtpSharedPath:: FtpSharedPath     ( void                    )
                 : TcpSharing        ( "FtpSharedPath" , 12134 )
                 , nextTransactionID ( 0                       )
{
}

N::FtpSharedPath::~FtpSharedPath(void)
{
}

N::FtpSharedPath * N::FtpSharedPath::instance(void)
{
  if (!pInstance) pInstance = new FtpSharedPath() ;
  return pInstance                                ;
}

void N::FtpSharedPath::receive(const QByteArray & message)
{
  QDataStream msg ( message )                                                ;
  QString type                                                               ;
  msg >>  type                                                               ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "mkd" )                                                       {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , mkd ( uid , user , name ) )              ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "answer" )                                                    {
    quint32 ID                                                               ;
    QString sender                                                           ;
    msg >> sender >> ID                                                      ;
    if (sender == FtpSettings::User())                                       {
      messageQueue[ID] = message.right(message.length()-msg.device()->pos()) ;
    }                                                                        ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "rmd" )                                                       {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , rmd ( uid , user , name ) )              ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "dele" )                                                      {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , dele ( uid , user , name ) )             ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "rename" )                                                    {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   from                                                           ;
    QString   to                                                             ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> from >> to                         ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , rename ( uid , user , from , to ) )      ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "list" )                                                      {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    bool      utf8                                                           ;
    msg >> sender >> ID >> uid >> user >> name >> utf8                       ;
    if ( user != FtpSettings::User() ) return                                ;
    QList<QByteArray> out                                                    ;
    list ( uid      , user   , out , name , utf8 )                           ;
    send ( "answer" , sender , ID  , out         )                           ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "nlst" )                                                      {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    bool      utf8                                                           ;
    msg >> sender >> ID >> uid >> user >> name >> utf8                       ;
    if ( user != FtpSettings::User() ) return                                ;
    QList<QByteArray> out                                                    ;
    nlst ( uid      , user   , out , name , utf8 )                           ;
    send ( "abswer" , sender , ID  , out         )                           ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "exists" )                                                    {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , exists ( uid , user , name ) )           ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "isDir" )                                                     {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , isDir ( uid , user , name ) )            ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "isFile" )                                                    {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , isFile ( uid , user , name ) )           ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "chmod" )                                                     {
    quint32    ID                                                            ;
    QString    sender                                                        ;
    QString    user                                                          ;
    QString    name                                                          ;
    FtpUserID  uid                                                           ;
    QByteArray perms                                                         ;
    msg >> sender >> ID >> uid >> user >> name >> perms                      ;
    if ( user != FtpSettings::User() ) return                                ;
    chmod ( uid      , user   , name , perms )                               ;
    send  ( "answer" , sender , ID           )                               ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "size" )                                                      {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , size ( uid , user , name ) )             ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "lastModified" )                                              {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , lastModified ( uid , user , name ) )     ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "isReadable" )                                                {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , isReadable ( uid , user , name ) )       ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "isWritable" )                                                {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , isWritable ( uid , user , name ) )       ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "listDirs" )                                                  {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , listDirs ( uid , user , name ) )         ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( type == "listAll" )                                                   {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   name                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> name                               ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , listAll ( uid , user , name ) )          ;
    return                                                                   ;
  }                                                                          ;
}

bool N::FtpSharedPath::mkd(const FtpUserID & uid,const QString & user,const QString & name)
{
  if ( user != FtpSettings::User() )                                       {
    QByteArray  result = remote( "mkd" , uid , user , name )               ;
    QDataStream msg            ( result                    )               ;
    bool   r                                                               ;
    msg >> r                                                               ;
    return r                                                               ;
  }                                                                        ;
  if ( ! FtpSettings :: checkID ( uid ) ) return false                     ;
  if (   FtpSharedFolders::getParentPath(name) == separator ) return false ;
  if ( ! FtpSharedFolders::instance()->isWritable(name)     ) return false ;
  QDir qdir(FtpSharedFolders::instance()->getRealPath(FtpSharedFolders::getParentPath(name)));
  return qdir.mkdir(FtpSharedFolders::getFileName(name))                   ;
}

bool N::FtpSharedPath::rmd(const FtpUserID & uid,const QString & user,const QString & name)
{
  if ( user != FtpSettings::User() )                                     {
    QByteArray  result = remote ( "rmd" , uid , user , name )            ;
    QDataStream msg             ( result                    )            ;
    bool   r                                                             ;
    msg >> r                                                             ;
    return r                                                             ;
  }                                                                      ;
  if ( !FtpSettings     ::checkID(uid)                    ) return false ;
  if (  FtpSharedFolders::getParentPath(name) == separator) return false ;
  if ( !FtpSharedFolders::instance()->isWritable(name)    ) return false ;
  QDir qdir ( FtpSharedFolders::instance()->getRealPath(FtpSharedFolders::getParentPath(name)) ) ;
  return qdir . rmdir ( FtpSharedFolders::getFileName(name) )            ;
}

bool N::FtpSharedPath::dele(const FtpUserID & uid,const QString & user,const QString & name)
{
  if ( user != FtpSettings::User() )                                        {
    QByteArray  result = remote ( "dele" , uid , user , name )              ;
    QDataStream msg             ( result                     )              ;
    bool   r                                                                ;
    msg >> r                                                                ;
    return r                                                                ;
  }                                                                         ;
  ///////////////////////////////////////////////////////////////////////////
  if (!FtpSettings      ::checkID(uid)                   ) return false     ;
  if ( FtpSharedFolders::getParentPath(name) == separator) return false     ;
  if (!FtpSharedFolders::instance()->isWritable(name)    ) return false     ;
  const QString realname = FtpSharedFolders::instance()->getRealPath(name)  ;
  QFileInfo info ( realname )                                               ;
  if ( ! info . isFile ( ) ) return false                                   ;
  QDir qdir                                                                 ;
  return qdir . remove ( realname )                                         ;
}

bool N::FtpSharedPath::rename(const FtpUserID & uid,const QString & user,const QString & from,const QString & to)
{
  if (user != FtpSettings::User())
  {
    QByteArray result = remote("rename", uid, user, from, to);
    QDataStream msg(result);
    bool r;
    msg >> r;
    return r;
  }
  if (!FtpSettings::checkID(uid)) return false;

  if (FtpSharedFolders::getParentPath(from) == separator) return false;
  if (!FtpSharedFolders::instance()->isWritable(from)) return false;
  if (FtpSharedFolders::getParentPath(to) == separator) return false;
  if (!FtpSharedFolders::instance()->isWritable(to)) return false;

  const QString realFrom =FtpSharedFolders::instance()->getRealPath(from);
  const QString realTo =FtpSharedFolders::instance()->getRealPath(to);
  QDir qdir;
  return qdir.rename(realFrom, realTo);
}

void N::FtpSharedPath::list(const FtpUserID & uid,const QString & user,QList<QByteArray> & out,const QString & name,bool utf8)
{
  if (user != FtpSettings::User())
  {
    QByteArray result = remote("list", uid, user, name, utf8);
    QDataStream msg(result);
    out.clear();
    msg >> out;
    return;
  }
  if (!FtpSettings::checkID(uid, true)) return;

  QString realpath =FtpSharedFolders::instance()->getRealPath(name);
  QDir qdir(realpath);
  QFileInfoList fileList = qdir.entryInfoList(QDir::AllEntries | QDir::Hidden, QDir::Name | QDir::DirsFirst | QDir::IgnoreCase);
  QList<QByteArray> replacementNames;

  if (realpath == separator)
  {
    fileList.clear();
    QMap<QString,FtpFolder> folders =FtpSharedFolders::instance()->getFolderList();
    for(QMap<QString,FtpFolder>::iterator i = folders.begin() ; i != folders.end() ; ++i)
    {
      QFileInfo info(i.value().getPath());
      fileList << info;
      if (utf8)
        replacementNames << i.key().toUtf8();
      else
        replacementNames << i.key().toLatin1();
    }
  }
  else if (QFileInfo(realpath).isFile())
  {
    fileList.clear();
    fileList << QFileInfo(realpath);
  }

  QString currentYear = QDateTime::currentDateTime().toString("yyyy");
  out.clear();
  int maxSizeLength(0);
  int maxGroupLength(0);
  int maxOwnerLength(0);
  foreach(QFileInfo info, fileList)
  {
    maxSizeLength = qMax(maxSizeLength, QString::number(info.size()).length());
    maxOwnerLength = qMax(maxOwnerLength, getOwner(info).length());
    maxGroupLength = qMax(maxGroupLength, getGroup(info).length());
  }
  const bool showHiddenFiles = FtpSettings::showHiddenFiles();
  foreach(QFileInfo info, fileList)
  {
    if (info.isHidden() && !showHiddenFiles && replacementNames.isEmpty())
      continue;
    QByteArray line;
    line += info.isDir() ? "d" : "-";
    line += (info.permissions() & QFile::ReadOwner) ? "r" : "-";
    line += (info.permissions() & QFile::WriteOwner) ? "w" : "-";
    line += (info.permissions() & QFile::ExeOwner) ? "x" : "-";
    line += (info.permissions() & QFile::ReadGroup) ? "r" : "-";
    line += (info.permissions() & QFile::WriteGroup) ? "w" : "-";
    line += (info.permissions() & QFile::ExeGroup) ? "x" : "-";
    line += (info.permissions() & QFile::ReadOther) ? "r" : "-";
    line += (info.permissions() & QFile::WriteOther) ? "w" : "-";
    line += (info.permissions() & QFile::ExeOther) ? "x" : "-";

    QString owner = getOwner(info);
    QString group = getGroup(info);
    line += "   1 ";
    line += owner.toUtf8() ;
    line += " ";
    QString size = QString::number(info.size());
    line += QString(maxOwnerLength - owner.length(), ' ').toUtf8();
    line += group.toUtf8() ;
    line += QString(maxGroupLength + maxSizeLength + 1 - group.length() - size.length(), ' ').toUtf8();
    line += size.toUtf8() ;
    line += " ";
    const char *cmonth[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    int monthID = qMin(qMax(info.created().toString("M").toInt() - 1, 0), 11);
    QString month = cmonth[monthID];
    QString day = info.created().toString("d");
    QString year = info.created().toString("yyyy") == currentYear
             ? info.created().toString("hh:mm")
             : info.created().toString("yyyy");
    line += month.toUtf8();
    line += QString(6 - day.length(), ' ').toUtf8();
    line += day.toUtf8();
    line += QString(6 - year.length(), ' ').toUtf8();
    line += year.toUtf8();
    line += " ";
    if (replacementNames.isEmpty())
    {
      if (utf8)
        line += info.fileName().toUtf8();
      else
        line += info.fileName().toLatin1();
    }
    else
    {
      line += replacementNames.front();
      replacementNames.pop_front();
    }

    out << line;
  }
}

void N::FtpSharedPath::nlst(const FtpUserID & uid,const QString & user,QList<QByteArray> & out,const QString & name,bool utf8)
{
  if (user != FtpSettings::User())
  {
    QByteArray result = remote("nlst", uid, user, name, utf8);
    QDataStream msg(result);
    out.clear();
    msg >> out;
    return;
  }
  if (!FtpSettings::checkID(uid, true)) return;

  QString realpath =FtpSharedFolders::instance()->getRealPath(name);
  QDir qdir(realpath);
  QFileInfoList fileList = qdir.entryInfoList(QDir::AllEntries | QDir::Hidden, QDir::Name | QDir::DirsFirst | QDir::IgnoreCase);

  out.clear();
  if (realpath == separator)
  {
    QMap<QString,FtpFolder> folders =FtpSharedFolders::instance()->getFolderList();
    for(QMap<QString,FtpFolder>::iterator i = folders.begin() ; i != folders.end() ; ++i)
    {
      if (utf8)
        out << i.key().toUtf8();
      else
        out << i.key().toLatin1();
    }
    return;
  }

  const bool showHiddenFiles = FtpSettings::showHiddenFiles();
  foreach(QFileInfo info, fileList)
  {
    if (info.isHidden() && !showHiddenFiles)
      continue;
    if (utf8)
      out << info.fileName().toUtf8();
    else
      out << info.fileName().toLatin1();
  }
}

bool N::FtpSharedPath::exists(const FtpUserID & uid,const QString & user,const QString & name)
{
  if ( user != FtpSettings::User() )                                 {
    QByteArray  result = remote ( "exists" , uid , user , name )     ;
    QDataStream msg             ( result                       )     ;
    bool   r                                                         ;
    msg >> r                                                         ;
    return r                                                         ;
  }                                                                  ;
  ////////////////////////////////////////////////////////////////////
  if ( ! FtpSettings :: checkID ( uid , true ) ) return false        ;
  QString realpath = FtpSharedFolders::instance()->getRealPath(name) ;
  QFileInfo info ( realpath )                                        ;
  return ( info.exists() && realpath != separator )                  ;
}

bool N::FtpSharedPath::isDir(const FtpUserID & uid,const QString & user,const QString & name)
{
  if ( user != FtpSettings::User() )                                 {
    QByteArray  result = remote ( "isDir" , uid , user , name )      ;
    QDataStream msg             ( result                      )      ;
    bool   r                                                         ;
    msg >> r                                                         ;
    return r                                                         ;
  }                                                                  ;
  ////////////////////////////////////////////////////////////////////
  if ( ! FtpSettings :: checkID ( uid , true ) ) return false        ;
  QString realpath = FtpSharedFolders::instance()->getRealPath(name) ;
  QFileInfo info ( realpath )                                        ;
  return ( info.exists() && info.isDir() && realpath != separator )  ;
}

bool N::FtpSharedPath::isFile(const FtpUserID & uid,const QString & user,const QString & name)
{
  if ( user != FtpSettings::User() )                                 {
    QByteArray  result = remote ( "isFile" , uid , user , name )     ;
    QDataStream msg             ( result                       )     ;
    bool   r                                                         ;
    msg >> r                                                         ;
    return r                                                         ;
  }                                                                  ;
  ////////////////////////////////////////////////////////////////////
  if ( ! FtpSettings :: checkID ( uid , true ) ) return false        ;
  QString realpath = FtpSharedFolders::instance()->getRealPath(name) ;
  QFileInfo info ( realpath )                                        ;
  return ( info.exists() && info.isFile() && realpath != separator ) ;
}

void N::FtpSharedPath::chmod(const FtpUserID & uid,const QString & user,const QString & name,QByteArray perms)
{
  if ( user != FtpSettings::User() )                                  {
    remote ( "chmod" , uid , user , name , perms )                    ;
    return                                                            ;
  }                                                                   ;
  /////////////////////////////////////////////////////////////////////
  if ( !FtpSettings     ::checkID(uid)                     ) return   ;
  if (  FtpSharedFolders::getParentPath(name) == separator ) return   ;
  if ( !FtpSharedFolders::instance()->isWritable(name)     ) return   ;
  /////////////////////////////////////////////////////////////////////
  QString realname                                                    ;
  realname = FtpSharedFolders :: instance ( ) -> getRealPath ( name ) ;
  if ( perms.size() <= 3 )                                            {
    perms = QByteArray(3 - perms.size(), '0') + perms                 ;
    QFile::Permissions rights                                         ;
    int code = perms[0] - '0'                                         ; // Owner rights
    if ( code & 1 ) rights |= QFile :: ExeOwner                       ;
    if ( code & 2 ) rights |= QFile :: WriteOwner                     ;
    if ( code & 4 ) rights |= QFile :: ReadOwner                      ;
    code = perms[1] - '0'                                             ; // Group rights
    if ( code & 1 ) rights |= QFile :: ExeGroup                       ;
    if ( code & 2 ) rights |= QFile :: WriteGroup                     ;
    if ( code & 4 ) rights |= QFile :: ReadGroup                      ;
    code = perms[2] - '0'                                             ; // Other rights
    if ( code & 1 ) rights |= QFile :: ExeOther                       ;
    if ( code & 2 ) rights |= QFile :: WriteOther                     ;
    if ( code & 4 ) rights |= QFile :: ReadOther                      ;
    QFile :: setPermissions ( realname , rights )                     ;
  }                                                                   ;
}

QByteArray N::FtpSharedPath::waitForID(const quint32 ID)
{
  QByteArray result                          ;
  QTime      timer                           ;
  timer . start ( )                          ;
  while ( ! messageQueue . contains ( ID ) ) {
    waitForData ( 10 )                       ;
    if (timer.elapsed() >= 10000)            {
      messageQueue . remove ( ID )           ;
      return result                          ;
    }                                        ;
  }                                          ;
  result       = messageQueue [ ID ]         ;
  messageQueue . remove       ( ID )         ;
  return result                              ;
}

qint64 N::FtpSharedPath::size(const FtpUserID & uid,const QString & user,const QString & name)
{
  if ( user != FtpSettings::User() )                                   {
    QByteArray  result = remote ( "size" , uid , user , name )         ;
    QDataStream msg             ( result                     )         ;
    qint64 r                                                           ;
    msg >> r                                                           ;
    return r                                                           ;
  }                                                                    ;
  //////////////////////////////////////////////////////////////////////
  if ( ! FtpSettings :: checkID ( uid , true ) ) return 0U             ;
  QString realpath = FtpSharedFolders::instance()->getRealPath(name)   ;
  QFileInfo info ( realpath )                                          ;
  return info . size ( )                                               ;
}

QDateTime N::FtpSharedPath::lastModified(const FtpUserID & uid,const QString & user,const QString & name)
{
  if ( user != FtpSettings::User() )                                   {
    QByteArray  result = remote ( "lastModified" , uid , user , name ) ;
    QDataStream msg             ( result                             ) ;
    QDateTime r                                                        ;
    msg    >> r                                                        ;
    return    r                                                        ;
  }                                                                    ;
  //////////////////////////////////////////////////////////////////////
  if ( ! FtpSettings :: checkID ( uid , true ) ) return QDateTime ( )  ;
  QString realpath = FtpSharedFolders::instance()->getRealPath(name)   ;
  QFileInfo info ( realpath )                                          ;
  return info . lastModified ( )                                       ;
}

bool N::FtpSharedPath::isReadable(const FtpUserID & uid,const QString & user,const QString & name)
{
  if ( user != FtpSettings::User() )                                 {
    QByteArray  result = remote ( "isReadable" , uid , user , name ) ;
    QDataStream msg             ( result                           ) ;
    bool   r                                                         ;
    msg >> r                                                         ;
    return r                                                         ;
  }                                                                  ;
  if ( ! FtpSettings :: checkID ( uid , true ) ) return false        ;
  QString realpath = FtpSharedFolders::instance()->getRealPath(name) ;
  QFileInfo info ( realpath )                                        ;
  return ( info.exists() && info.isReadable() )                      ;
}

bool N::FtpSharedPath::isWritable(const FtpUserID & uid, const QString & user, const QString & name)
{
  if ( user != FtpSettings::User() )                                 {
    QByteArray  result = remote ( "isWritable" , uid , user , name ) ;
    QDataStream msg             ( result                           ) ;
    bool   r                                                         ;
    msg >> r                                                         ;
    return r                                                         ;
  }                                                                  ;
  if ( ! FtpSettings :: checkID ( uid ) ) return false               ;
  return FtpSharedFolders :: instance ( ) -> isWritable ( name )     ;
}

QStringList N::FtpSharedPath::listDirs (
              const FtpUserID & uid    ,
              const QString   & user   ,
              const QString   & name   )
{
  if ( user != FtpSettings::User() )                                         {
    QByteArray  result = remote ( "listDirs" , uid , user , name )           ;
    QDataStream msg             ( result                         )           ;
    QStringList r                                                            ;
    msg      >> r                                                            ;
    return      r                                                            ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if ( ! FtpSettings :: checkID ( uid , true ) ) return QStringList ( )      ;
  const QString realPath = FtpSharedFolders::instance()->getRealPath(name)   ;
  ////////////////////////////////////////////////////////////////////////////
  if ( realPath == separator )                                               {
    QMap<QString,FtpFolder> folders                                          ;
    QStringList             dirs                                             ;
    folders = FtpSharedFolders :: instance ( ) -> getFolderList ( )          ;
    for (QMap<QString,FtpFolder>::iterator i = folders.begin()               ;
         i != folders.end()                                                  ;
         ++i                                                               ) {
      dirs . push_back ( i . key ( ) )                                       ;
    }                                                                        ;
    return dirs                                                              ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  QDir dir ( realPath )                                                      ;
  if (FtpSettings::showHiddenFiles())                                        {
    return dir . entryList ( QDir::AllDirs                                   |
                             QDir::NoSymLinks                                |
                             QDir::Hidden                                    |
                             QDir::System                                  ) ;
  }                                                                          ;
  return dir . entryList ( QDir::AllDirs | QDir::NoSymLinks | QDir::System ) ;
}

QStringList N::FtpSharedPath::listAll (
              const FtpUserID & uid   ,
              const QString   & user  ,
              const QString   & name  )
{
  if (user != FtpSettings::User())                                         {
    QByteArray  result = remote ( "listAll" , uid , user , name )          ;
    QDataStream msg             ( result                        )          ;
    QStringList r                                                          ;
    msg >>      r                                                          ;
    return      r                                                          ;
  }                                                                        ;
  //////////////////////////////////////////////////////////////////////////
  if ( ! FtpSettings :: checkID ( uid , true ) ) return QStringList()      ;
  const QString realPath = FtpSharedFolders::instance()->getRealPath(name) ;
  //////////////////////////////////////////////////////////////////////////
  if (realPath == separator)                                               {
    QMap<QString,FtpFolder> folders                                        ;
    folders = FtpSharedFolders::instance()->getFolderList()                ;
    QStringList dirs                                                       ;
    for (QMap<QString,FtpFolder>::iterator i = folders.begin()             ;
         i != folders.end()                                                ;
         ++i                                                             ) {
      dirs . push_back ( i.key() )                                         ;
    }                                                                      ;
    return dirs                                                            ;
  }                                                                        ;
  QDir dir ( realPath )                                                    ;
  if ( FtpSettings :: showHiddenFiles ( ) )                                {
    return dir . entryList(QDir::AllEntries | QDir::Hidden | QDir::System) ;
  }                                                                        ;
  return dir . entryList ( QDir::AllEntries | QDir::System )               ;
}
