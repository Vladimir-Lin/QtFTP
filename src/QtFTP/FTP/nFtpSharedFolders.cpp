#include <qtftp.h>

N::FtpSharedFolders * N::FtpSharedFolders::pInstance = NULL ;

const QString N::FtpSharedFolders::separator('/') ;

N::FtpSharedFolders * N::FtpSharedFolders::instance(void)
{
  if (NotNull(pInstance)) return pInstance ;
  return new FtpSharedFolders ( )          ;
}

N::FtpSharedFolders:: FtpSharedFolders (void             )
                    : QMutex           (QMutex::Recursive)
{
  pInstance = this ;
  loadFolders ( )  ;
}

N::FtpSharedFolders::~FtpSharedFolders(void)
{
  pInstance = NULL ;
}

void N::FtpSharedFolders::loadFolders(void)
{
  QMutexLocker mLock ( this    ) ;
  FtpSettings::Load  ( folders ) ;
}

void N::FtpSharedFolders::saveFolders(void)
{
  QMutexLocker mLock ( this    ) ;
  FtpSettings::Save  ( folders ) ;
}

void N::FtpSharedFolders::addFolder(const QString & name,const FtpFolder & folder)
{
  QMutexLocker mLock ( this ) ;
  folders[name] = folder      ;
  saveFolders        (      ) ;
}

void N::FtpSharedFolders::removeFolder(const QString & name)
{
  QMutexLocker mLock ( this ) ;
  folders . remove   ( name ) ;
  saveFolders        (      ) ;
}

QMap<QString,N::FtpFolder> N::FtpSharedFolders::getFolderList(void)
{
  QMutexLocker mLock ( this ) ;
  return folders              ;
}

QString N::FtpSharedFolders::getRealPath(const QString & path)
{
  QStringList keys = path.split(separator, QString::SkipEmptyParts) ;
  if (keys.isEmpty()) return separator                              ;
  QMutexLocker mLock ( this )                                       ;
  if (folders.contains(keys[0]))                                    {
    keys[0] = folders[keys[0]].getPath()                            ;
    return keys.join(separator)                                     ;
  }                                                                 ;
  return separator                                                  ;
}

bool N::FtpSharedFolders::isWritable(const QString & path)
{
  QStringList keys = path.split(separator, QString::SkipEmptyParts) ;
  if (keys.isEmpty()) return false                                  ;
  QMutexLocker mLock ( this )                                       ;
  if (folders.contains(keys[0]))                                    {
    return folders [ keys [ 0 ] ] . isWritable ( )                  ;
  }                                                                 ;
  return false                                                      ;
}

int N::FtpSharedFolders::nbFolders(void)
{
  return folders . size ( ) ;
}

QString N::FtpSharedFolders::getParentPath(const QString &path)
{
  QStringList keys = path.split(separator, QString::SkipEmptyParts) ;
  if (keys.size() <= 1) return separator                            ;
  keys . removeLast ( )                                             ;
  return separator + keys.join(separator)                           ;
}

QString N::FtpSharedFolders::getFileName(const QString & path)
{
  QStringList keys = path.split(separator,QString::SkipEmptyParts) ;
  if (keys.isEmpty()) return QString ( )                           ;
  return keys . last ( )                                           ;
}
