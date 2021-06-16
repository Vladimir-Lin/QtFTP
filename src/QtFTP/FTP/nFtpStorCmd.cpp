#include <qtftp.h>

N::STORcmd:: STORcmd ( const FtpUserID & uid      ,
                       const QString   & filename ,
                       bool              append   ,
                       FtpThread       * parent   )
           : QObject (                   parent   )
           , file    ( uid , filename             )
{
  dead = false                                                                 ;
  if (!append) file . open ( QFile::WriteOnly                 )                ;
          else file . open ( QFile::WriteOnly | QFile::Append )                ;
  k = 0                                                                        ;
  if ( file.exists() && file.isWritable() && file.isOpen() )                   {
    nConnect(this->parent()->passv,SIGNAL(readyRead())                         ,
             this                 ,SLOT  (readData ())                       ) ;
    nConnect(this->parent()->passv                                             ,
             SIGNAL(error(QAbstractSocket::SocketError))                       ,
             this                                                              ,
             SLOT(deleteLater())                                             ) ;
    nConnect(this->parent()->passv,SIGNAL(disconnected())                      ,
             this                 ,SLOT  (disconnected())                    ) ;
    this->parent()->send("150 File status okay; about to open data connection.") ;
  } else                                                                       {
    this->parent()->send("451 Requested action aborted. Local error in processing.");
    this->parent()->killDTP ( )                                                ;
    dead = true                                                                ;
    emit activity ( )                                                          ;
    deleteLater   ( )                                                          ;
  }                                                                            ;
}

N::STORcmd::~STORcmd(void)
{
}

void N::STORcmd::disconnected(void)
{
  if (this->parent()->passv->bytesAvailable() > 0) readData( ) ;
  disconnect          ( this->parent()->passv , 0 , this , 0 ) ;
  file . flush        (                                      ) ;
  file . close        (                                      ) ;
  parent() -> killDTP (                                      ) ;
  parent() -> send    ( "250 Transfer complete"              ) ;
  emit activity       (                                      ) ;
  emit finished       (                                      ) ;
  deleteLater         (                                      ) ;
  dead = true                                                  ;
}

N::FtpThread * N::STORcmd::parent(void) const
{
  return static_cast<FtpThread *>(QObject::parent()) ;
}

void N::STORcmd::readData(void)
{
  if (dead) return                               ;
  QByteArray data = parent()->passv->readAll()   ;
  ////////////////////////////////////////////////
  FileSharing::pInstance->statsRecv(data.size()) ;
  ////////////////////////////////////////////////
  if (k > 100)                                   {
    emit activity ( )                            ;
    k = 0                                        ;
  }                                              ;
  ////////////////////////////////////////////////
  ++k                                            ;
  file . write ( data )                          ;
  ////////////////////////////////////////////////
  if (parent()->mode != FtpThread::STOR         &&
      parent()->mode != FtpThread::APPE        ) {
    this->parent()->send("451 Requested action aborted. Local error in processing.");
    this->parent()->killDTP()                    ;
    deleteLater ()                               ;
    dead = true                                  ;
  }                                              ;
}
