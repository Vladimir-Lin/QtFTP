#include <qtftp.h>

N::FtpProtocol:: FtpProtocol (void)
               : NetProtocol (    )
{
}

N::FtpProtocol::~FtpProtocol (void)
{
}

int N::FtpProtocol::type(void) const
{
  return 959 ;
}

bool N::FtpProtocol::In(int size,char * data)
{
  return true ;
}

bool N::FtpProtocol::In(QByteArray & data)
{
  return true ;
}

bool N::FtpProtocol::Out(QByteArray & data)
{
  return true ;
}
