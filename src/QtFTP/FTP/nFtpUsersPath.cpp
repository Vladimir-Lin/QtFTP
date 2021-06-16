#include <qtftp.h>

#define HEAD(RET)                                                     \
  QStringList path = name.split(separator, QString::SkipEmptyParts) ; \
  if (path.isEmpty()) return RET                                    ; \
  const QSet<QString> &users = FileSharing::instance()->getUsers()  ; \
  if (!users.contains(path[0])) return RET

const QChar N::FtpUsersPath::separator('/');

N::FtpUsersPath:: FtpUsersPath(void)
{
}

N::FtpUsersPath::~FtpUsersPath(void)
{
}

bool N::FtpUsersPath::mkd(const FtpUserID & uid,const QString & name)
{
  HEAD ( false )                                                 ;
  const QString user = path[0]                                   ;
  path . removeFirst ( )                                         ;
  return FtpSharedPath::instance()->mkd(uid,user,makePath(path)) ;
}

bool N::FtpUsersPath::rmd(const FtpUserID & uid,const QString & name)
{
  HEAD ( false )                                                 ;
  const QString user = path[0]                                   ;
  path . removeFirst ( )                                         ;
  return FtpSharedPath::instance()->rmd(uid,user,makePath(path)) ;
}

bool N::FtpUsersPath::dele(const FtpUserID & uid,const QString & name)
{
  HEAD ( false )                                                  ;
  const QString user = path[0]                                    ;
  path . removeFirst ( )                                          ;
  return FtpSharedPath::instance()->dele(uid,user,makePath(path)) ;
}

bool N::FtpUsersPath::rename(const FtpUserID & uid,const QString & from,const QString & to)
{
  QStringList pathFrom = from . split ( separator , QString::SkipEmptyParts ) ;
  QStringList pathTo   = to   . split ( separator , QString::SkipEmptyParts ) ;
  const QSet<QString> & users = FileSharing::instance()->getUsers()           ;
  if ( pathFrom.isEmpty() || pathTo.isEmpty() ) return false                  ;
  if ( pathFrom[0]        != pathTo[0]        ) return false                  ;
  if ( !users.contains(pathFrom[0])           ) return false                  ;
  const QString user = pathFrom[0]                                            ;
  pathFrom . removeFirst ( )                                                  ;
  pathTo   . removeFirst ( )                                                  ;
  return FtpSharedPath::instance()->rename(uid,user,makePath(pathFrom),makePath(pathTo)) ;
}

void N::FtpUsersPath::list      (
       const FtpUserID   & uid  ,
       QList<QByteArray> & out  ,
       const QString     & name ,
       bool                utf8 )
{
  QStringList path = name.split(separator,QString::SkipEmptyParts)    ;
  const QSet<QString> & users = FileSharing::instance()->getUsers()   ;
  if (path.isEmpty())                                                 {
    ///////////////////////////////////////////////////////////////////
    const int maxSizeLength  ( 1 )                                    ;
    int       maxGroupLength ( 0 )                                    ;
    ///////////////////////////////////////////////////////////////////
    foreach (QString user,users)                                      {
      maxGroupLength = qMax ( maxGroupLength , user.length() )        ;
    }                                                                 ;
    ///////////////////////////////////////////////////////////////////
    int maxOwnerLength ( maxGroupLength )                             ;
    foreach (QString user,users) if (user.length()>0)                 {
      QByteArray line                                                 ;
      line += "dr-xr-xr-x   1 "                                       ;
      line += user.toUtf8()                                           ;
      line += " "                                                     ;
      line += QString(maxOwnerLength-user.length(),' ').toUtf8()      ;
      line += user.toUtf8()                                           ;
      line += QString(maxGroupLength+maxSizeLength-user.length(),' ').toUtf8() ;
      line += "1 Jan   01 00:00 "                                     ;
      if (utf8) line += user . toUtf8   ( )                           ;
           else line += user . toLatin1 ( )                           ;
      out << line                                                     ;
    }                                                                 ;
    return                                                            ;
  }                                                                   ;
  if ( ! users . contains ( path[0] ) ) return                        ;
  /////////////////////////////////////////////////////////////////////
  const QString user = path[0]                                        ;
  path.removeFirst()                                                  ;
  FtpSharedPath::instance()->list(uid,user,out,makePath(path),utf8)   ;
}

void N::FtpUsersPath::nlst      (
       const FtpUserID   & uid  ,
       QList<QByteArray> & out  ,
       const QString     & name ,
       bool                utf8 )
{
  QStringList path = name.split(separator, QString::SkipEmptyParts) ;
  const QSet<QString> & users = FileSharing::instance()->getUsers() ;
  if (path.isEmpty())                                               {
    foreach(QString user, users)                                    {
      out << ( utf8 ? user . toUtf8 ( ) : user . toLatin1 ( ) )     ;
    }                                                               ;
    return                                                          ;
  }                                                                 ;
  if ( !users.contains(path[0]) ) return                            ;
  const QString user = path[0]                                      ;
  path . removeFirst ( )                                            ;
  FtpSharedPath::instance()->nlst(uid,user,out,makePath(path),utf8) ;
}

bool N::FtpUsersPath::exists(const FtpUserID & uid,const QString & name)
{
  if (name == "/") return true                                      ;
  HEAD ( false )                                                    ;
  if (path.size() == 1) return true                                 ;
  const QString user = path[0]                                      ;
  path . removeFirst ( )                                            ;
  return FtpSharedPath::instance()->exists(uid,user,makePath(path)) ;
}

bool N::FtpUsersPath::isDir(const FtpUserID & uid,const QString & name)
{
  if ( name == "/" ) return true                                   ;
  HEAD ( false )                                                   ;
  if (path.size() == 1) return true                                ;
  const QString user = path[0]                                     ;
  path . removeFirst ( )                                           ;
  return FtpSharedPath::instance()->isDir(uid,user,makePath(path)) ;
}

bool N::FtpUsersPath::isFile(const FtpUserID & uid, const QString & name)
{
  HEAD ( false )                                                    ;
  if (path.size() == 1) return false                                ;
  const QString user = path[0]                                      ;
  path . removeFirst ( )                                            ;
  return FtpSharedPath::instance()->isFile(uid,user,makePath(path)) ;
}

void N::FtpUsersPath::chmod(const FtpUserID & uid,const QString & name,QByteArray perms)
{
  QStringList path = name.split(separator,QString::SkipEmptyParts) ;
  if (path.isEmpty()) return                                       ;
  const QSet<QString> &users = FileSharing::instance()->getUsers() ;
  if (!users.contains(path[0])) return                             ;
  if (path.size() == 1) return                                     ;
  const QString user = path[0]                                     ;
  path . removeFirst ( )                                           ;
  FtpSharedPath::instance()->chmod(uid,user,makePath(path),perms)  ;
}

QString N::FtpUsersPath::makePath(const QStringList & keys)
{
  return separator + keys . join ( separator ) ;
}

qint64 N::FtpUsersPath::size(const FtpUserID & uid, const QString & name)
{
  HEAD ( 0 )                                                      ;
  if (path.size() == 1) return 0                                  ;
  const QString user = path[0]                                    ;
  path . removeFirst ( )                                          ;
  return FtpSharedPath::instance()->size(uid,user,makePath(path)) ;
}

QDateTime N::FtpUsersPath::lastModified(const FtpUserID & uid, const QString & name)
{
  HEAD ( QDateTime() )                                                    ;
  if (path.size() == 1) return QDateTime()                                ;
  const QString user = path[0]                                            ;
  path.removeFirst()                                                      ;
  return FtpSharedPath::instance()->lastModified(uid,user,makePath(path)) ;
}

bool N::FtpUsersPath::isReadable(const FtpUserID & uid,const QString & name)
{
  HEAD ( false )                                                        ;
  if (path.size() == 1) return false                                    ;
  const QString user = path[0]                                          ;
  path . removeFirst ( )                                                ;
  return FtpSharedPath::instance()->isReadable(uid,user,makePath(path)) ;
}

bool N::FtpUsersPath::isWritable(const FtpUserID & uid,const QString & name)
{
  HEAD ( false )                                                        ;
  if (path.size() == 1) return false                                    ;
  const QString user = path[0]                                          ;
  path . removeFirst ( )                                                ;
  return FtpSharedPath::instance()->isWritable(uid,user,makePath(path)) ;
}

QStringList N::FtpUsersPath::listDirs(const FtpUserID & uid,const QString & name)
{
  QStringList path = name.split(separator,QString::SkipEmptyParts)      ;
  if (path.isEmpty())                                                   {
    return QStringList::fromSet(FileSharing::instance()->getUsers())    ;
  }                                                                     ;
  ///////////////////////////////////////////////////////////////////////
  const QSet<QString> & users = FileSharing::instance()->getUsers()     ;
  if (!users.contains(path[0])) return QStringList()                    ;
  const QString user = path[0]                                          ;
  path . removeFirst ( )                                                ;
  return FtpSharedPath::instance()->listDirs(uid, user, makePath(path)) ;
}

QStringList N::FtpUsersPath::listAll(const FtpUserID & uid,const QString & name)
{
  QStringList path = name.split(separator,QString::SkipEmptyParts)      ;
  if (path.isEmpty())                                                   {
    return QStringList::fromSet(FileSharing::instance()->getUsers())    ;
  }                                                                     ;
  ///////////////////////////////////////////////////////////////////////
  const QSet<QString> & users = FileSharing::instance()->getUsers()     ;
  if (!users.contains(path[0])) return QStringList()                    ;
  const QString user = path[0]                                          ;
  path . removeFirst ( )                                                ;
  return FtpSharedPath::instance()->listAll(uid, user, makePath(path))  ;
}
