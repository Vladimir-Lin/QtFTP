#include <qtftp.h>

#define DTP_BUFFER_SIZE 0x100000

N::RETRcmd:: RETRcmd ( const FtpUserID & uid      ,
                       const QString   & filename ,
                       FtpThread       * parent   )
           : QObject (                   parent   )
           , file    ( uid , filename             )
{
  file . open ( QFile::ReadOnly )                                     ;
  dead = false                                                        ;
  if (file.exists() && file.isReadable() && file.isOpen())            {
    file  . seek           ( this->parent()->skip )                   ;
    this -> parent()->skip = 0                                        ;
    this -> parent()->send ( "150 Opening BINARY mode data connection for " + filename + " (" + QString::number(file.size()) + " bytes)") ;
    k     = 0                                                         ;
    ///////////////////////////////////////////////////////////////////
    nConnect ( this->parent()->passv , SIGNAL(bytesWritten(qint64))   ,
               this                  , SLOT  (sendMore    ())       ) ;
    nConnect ( this->parent()->passv , SIGNAL(error(QAbstractSocket::SocketError)),
               this                  , SLOT  (error())              ) ;
    nConnect ( this->parent()->passv , SIGNAL(disconnected())         ,
               this                  , SLOT  (error())              ) ;
    ///////////////////////////////////////////////////////////////////
    if (this->parent()->passv->state() == QTcpSocket::ConnectedState) {
      sendMore ( )                                                    ;
    } else                                                            {
      nConnect ( this->parent()->passv , SIGNAL (connected())         ,
                 this                  , SLOT   (sendMore ())       ) ;
    }                                                                 ;
  } else                                                              {
    error ( false )                                                   ;
  }                                                                   ;
}

N::RETRcmd::~RETRcmd(void)
{
}

N::FtpThread * N::RETRcmd::parent(void) const
{
  return static_cast<FtpThread *> ( QObject :: parent ( ) ) ;
}

void N::RETRcmd::sendMore(void)
{
  if (dead) return                                  ;
  if ( parent()->mode != FtpThread::RETR )          {
    finishing ( )                                   ;
    return                                          ;
  }                                                 ;
  if (parent()->passv->bytesToWrite() > 0) return   ;
  QByteArray data = file . read ( DTP_BUFFER_SIZE ) ;
  if ( data.isEmpty() )                             {
    finishing ( )                                   ;
    return                                          ;
  }                                                 ;
  parent() -> passv -> write ( data )               ;
  FileSharing::pInstance->statsSend(data.size())    ;
  if (k > 100)                                      {
    emit activity ( )                               ;
    k = 0                                           ;
  }                                                 ;
  ++k                                               ;
}

void N::RETRcmd::error(bool killDTP)
{
  disconnect     ( this->parent()->passv , 0 , this , 0                       ) ;
  file . close   (                                                            ) ;
  if (killDTP) parent()->killDTP()                                              ;
          else parent()->mode = FtpThread::NONE                                 ;
  parent()->send ( "451 Requested action aborted. Local error in processing." ) ;
  emit activity  (                                                            ) ;
  emit finished  (                                                            ) ;
  deleteLater    (                                                            ) ;
  dead = true                                                                   ;
}

void N::RETRcmd::finishing(void)
{
  disconnect            ( this->parent()->passv , 0 , this , 0 ) ;
  file.close            (                                      ) ;
  parent ( ) -> killDTP (                                      ) ;
  parent ( ) -> send    ( "226 Transfer complete"              ) ;
  emit activity         (                                      ) ;
  emit finished         (                                      ) ;
  deleteLater           (                                      ) ;
  dead = true                                                    ;
}
