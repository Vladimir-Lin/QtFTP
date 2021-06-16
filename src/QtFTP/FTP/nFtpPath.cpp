#include <qtftp.h>

const QString N::FtpPath::separator('/') ;

N::FtpPath:: FtpPath(void)
{
  currentPath = "/" ;
}

N::FtpPath::~FtpPath(void)
{
}

QString N::FtpPath::pwd(void)
{
  return currentPath ;
}

bool N::FtpPath::cwd(const QString & path)
{
  const QString absolutePath = absolute(path)  ;
  if (!FtpUsersPath::isDir (uid,absolutePath) ||
      !FtpUsersPath::exists(uid,absolutePath)) {
    return false                               ;
  }                                            ;
  currentPath = absolutePath                   ;
  return true                                  ;
}

bool N::FtpPath::mkd(const QString & name)
{
  const QString absolutePath = absolute(name) ;
  return FtpUsersPath::mkd(uid,absolutePath)  ;
}

void N::FtpPath::chmod(const QString & path,QByteArray perms)
{
  const QString absolutePath = absolute(path)        ;
  return FtpUsersPath::chmod(uid,absolutePath,perms) ;
}

bool N::FtpPath::rmd(const QString & name)
{
  const QString absolutePath = absolute(name) ;
  return FtpUsersPath::rmd(uid,absolutePath)  ;
}

bool N::FtpPath::dele(const QString & name)
{
  const QString absolutePath = absolute(name) ;
  return FtpUsersPath::dele(uid,absolutePath) ;
}

bool N::FtpPath::cdup(void)
{
  if ( currentPath == separator ) return false               ;
  currentPath = FtpSharedFolders::getParentPath(currentPath) ;
  return true                                                ;
}

bool N::FtpPath::rename(const QString & from, const QString & to)
{
  const QString absolutePathFrom = absolute ( from )               ;
  const QString absolutePathTo   = absolute ( to   )               ;
  return FtpUsersPath::rename(uid,absolutePathFrom,absolutePathTo) ;
}

void N::FtpPath::list(QList<QByteArray> & sList,const QString & filename,bool utf8)
{
  if (!filename.isEmpty() && filename[0] == '-')     {
    list ( sList , utf8 )                            ;
    return                                           ;
  }                                                  ;
  const QString absolutePath = absolute ( filename ) ;
  FtpUsersPath::list (uid,sList,absolutePath,utf8)   ;
}

void N::FtpPath::list(QList<QByteArray> & sList,bool utf8)
{
  const QString absolutePath = absolute(currentPath) ;
  FtpUsersPath::list (uid,sList,absolutePath,utf8)   ;
}

void N::FtpPath::nlst(QList<QByteArray> & sList,bool utf8)
{
  const QString absolutePath = absolute(currentPath) ;
  FtpUsersPath::nlst (uid,sList,absolutePath,utf8)   ;
}

QString N::FtpPath::absolute(const QString & path)
{
  QString absolutePath = path                                  ;
  if ( !path.isEmpty() && path[0] != '/' )                     {
    absolutePath = currentPath + separator + path              ;
  }                                                            ;
  QStringList keys                                             ;
  keys = absolutePath.split(separator,QString::SkipEmptyParts) ;
  if (keys.empty()) return separator                           ;
  QStringList out                                              ;
  foreach ( QString elt , keys )                               {
    if (elt == "..")                                           {
      if (!out.isEmpty()) out . removeLast ( )                 ;
    } else out << elt                                          ;
  }                                                            ;
  return separator + out . join ( separator )                  ;
}

QString N::FtpPath::realPath(const QString & path)
{
  return FtpSharedFolders::instance()->getRealPath(absolute(path)) ;
}

bool N::FtpPath::exists(const QString & path)
{
  return FtpUsersPath::exists(uid,absolute(path)) ;
}

qint64 N::FtpPath::size(const QString & name)
{
  return FtpUsersPath::size(uid,absolute(name)) ;
}

QDateTime N::FtpPath::lastModified(const QString & name)
{
  return FtpUsersPath::lastModified(uid,absolute(name)) ;
}

bool N::FtpPath::isReadable(const QString & name)
{
  return FtpUsersPath::isReadable(uid,absolute(name)) ;
}

bool N::FtpPath::isWritable(const QString & name)
{
  return FtpUsersPath::isWritable(uid,absolute(name)) ;
}

void N::FtpPath::SetUID(const FtpUserID & uid)
{
  Iamv(uid) = uid ;
}
