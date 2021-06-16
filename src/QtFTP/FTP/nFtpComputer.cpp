#include <qtftp.h>

#define TIME_OUT_DELAY 20000

N::FtpComputer:: FtpComputer (      )
               : hostname    (      )
               , port        ( 2000 )
               , name        (      )
{
  time  = new QTime ( ) ;
  time -> start     ( ) ;
}

N::FtpComputer:: FtpComputer(const QByteArray & data)
{
  time  = new QTime (      ) ;
  time -> start     (      ) ;
  unserialize       ( data ) ;
}

N::FtpComputer:: FtpComputer (const FtpComputer & c)
               : QObject     (                     )
               , hostname    (c . hostname         )
               , port        (c . port             )
               , name        (c . name             )
{
   time = new QTime ( ) ;
  *time = *c.time       ;
}

N::FtpComputer::~FtpComputer(void)
{
}

N::FtpComputer & N::FtpComputer::operator=(const FtpComputer & c)
{
  hostname =  c . hostname ;
  name     =  c . name     ;
  port     =  c . port     ;
  *time    = *c . time     ;
  return ME                ;
}

bool N::FtpComputer::operator!=(const FtpComputer & c)
{
  return ( c.port != port || c.name != name || c.hostname != hostname ) ;
}

void N::FtpComputer::open(const QString & path)
{
  QUrl u    ( "ftp://" + hostname + ":" + QString::number(port) + path ) ;
  emit look ( u                                                        ) ;
}

QString N::FtpComputer::getHostname(void) const
{
  return hostname ;
}

QString N::FtpComputer::getName(void) const
{
  return name ;
}

int N::FtpComputer::getPort(void) const
{
  return port ;
}

void N::FtpComputer::setPort(const int port)
{
  this -> port = port ;
  time -> start ( )   ;
}

void N::FtpComputer::setHostname(const QString & hostname)
{
  this -> hostname = hostname ;
  time -> start ( )           ;
}

void N::FtpComputer::setName(const QString & name)
{
  this -> name = name ;
  time -> start ( )   ;
}

bool N::FtpComputer::timedOut(void) const
{
  return ( time->elapsed() > TIME_OUT_DELAY ) ;
}

QByteArray N::FtpComputer::serialize(void) const
{
  QByteArray  data                                    ;
  QDataStream stream ( &data , QIODevice::WriteOnly ) ;
  stream  << hostname << port << name                 ;
  return data                                         ;
}

void N::FtpComputer::unserialize(const QByteArray & data)
{
  QDataStream stream ( data )        ;
  stream >> hostname >> port >> name ;
}
