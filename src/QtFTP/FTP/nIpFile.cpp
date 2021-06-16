#include <qtftp.h>

N::IpFile:: IpFile ( const FtpUserID & uid      ,
                     const QString   & filename )
          : fileID ( 0                          )
          , bOpen  ( false                      )
          , uid    ( uid                        )
{
  QStringList path = filename.split(FtpPath::separator,QString::SkipEmptyParts) ;
  if (path.isEmpty())                                                           {
    this -> filename . clear ( )                                                ;
    this -> user     . clear ( )                                                ;
    return                                                                      ;
  }                                                                             ;
  const QSet<QString> & users = FileSharing::instance()->getUsers()             ;
  if ( ! users . contains ( path[0] ) )                                         {
    this -> filename . clear ( )                                                ;
    this -> user     . clear ( )                                                ;
    return                                                                      ;
  }                                                                             ;
  user = path [ 0 ]                                                             ;
  path . removeFirst ( )                                                        ;
  this -> filename = FtpPath::separator + path . join ( FtpPath::separator )    ;
  fullname         = filename                                                   ;
}

N::IpFile::~IpFile(void)
{
  close ( ) ;
}

bool N::IpFile::exists(void) const
{
  if (user.isEmpty()) return false          ;
  return FtpUsersPath::exists(uid,fullname) ;
}

bool N::IpFile::isReadable(void) const
{
  if (!bOpen) return false                                 ;
  return IpFileServer::instance()->isReadable(user,fileID) ;
}

bool N::IpFile::isWritable(void) const
{
  if (!bOpen) return false                                 ;
  return IpFileServer::instance()->isWritable(user,fileID) ;
}

qint64 N::IpFile::size(void) const
{
  if (user.isEmpty()) return 0            ;
  return FtpUsersPath::size(uid,fullname) ;
}

bool N::IpFile::isOpen(void) const
{
  return bOpen ;
}

bool N::IpFile::seek(qint64 offset)
{
  if (!bOpen) return true                                   ;
  return IpFileServer::instance()->seek(user,fileID,offset) ;
}

void N::IpFile::open(QFile::OpenMode mode)
{
  if (user.isEmpty()) return                                      ;
  if (bOpen         ) close ( )                                   ;
  bOpen  = true                                                   ;
  fileID = IpFileServer::instance()->open(uid,user,filename,mode) ;
}

void N::IpFile::close(void)
{
  if (!bOpen) return                           ;
  bOpen = false                                ;
  IpFileServer::instance()->close(user,fileID) ;
}

void N::IpFile::flush(void)
{
  if (bOpen) IpFileServer::instance()->flush(user,fileID) ;
}

QByteArray N::IpFile::read(qint64 maxlen)
{
  if (!bOpen) return QByteArray ( )                         ;
  return IpFileServer::instance()->read(user,fileID,maxlen) ;
}

void N::IpFile::write(const QByteArray & data)
{
  if (bOpen) IpFileServer::instance()->write(user,fileID,data) ;
}
