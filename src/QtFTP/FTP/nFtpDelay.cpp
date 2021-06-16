#include <qtftp.h>

void N::FtpDelay::msleep(unsigned long ms)
{
  QThread :: currentThread ( ) -> msleep ( ms ) ;
}

void N::FtpDelay::usleep(unsigned long us)
{
  QThread :: currentThread ( ) -> usleep ( us ) ;
}

void N::FtpDelay::sleep(unsigned long s)
{
  QThread :: currentThread ( ) -> sleep  ( s  ) ;
}
