#include <qtftp.h>

N::FtpFolder:: FtpFolder (const QString & path,bool writable)
             : writable  (                          writable)
             , path      (                path              )
{
}

N::FtpFolder::~FtpFolder(void)
{
}

bool N::FtpFolder::isWritable(void) const
{
  return writable ;
}

const QString & N::FtpFolder::getPath(void) const
{
  return path ;
}

void N::FtpFolder::setWritable(bool writable)
{
  Iamv ( writable ) = writable ;
}

void N::FtpFolder::setPath(const QString & path)
{
  Iamv ( path ) = path ;
}
