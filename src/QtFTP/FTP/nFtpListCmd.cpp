#include <qtftp.h>

N::LISTcmd:: LISTcmd ( const QString & path     ,
                       bool            nlstMode ,
                       FtpThread     * parent   )
           : QObject (                 parent   )
{
  QString extracted_path                                                       ;
  //////////////////////////////////////////////////////////////////////////////
  for ( int i = 0 ; i < path.size() ; ++i )                                    {
    if (path[i] == '-')                                                        {
      while ( i < path.size() && !path[i].isSpace() ) ++i                      ;
      bool b_space_skipped = false                                             ;
      while ( i < path.size() && path[i].isSpace() )                           {
        ++i                                                                    ;
        b_space_skipped = true                                                 ;
      }                                                                        ;
      if (b_space_skipped) --i                                                 ;
    } else                                                                     {
      extracted_path += path[i]                                                ;
    }                                                                          ;
  }                                                                            ;
  //////////////////////////////////////////////////////////////////////////////
  if (nlstMode)                                                                {
    if (extracted_path.isEmpty())                                              {
      this -> parent ( ) -> path . nlst ( list,parent->UTF8mode() )            ;
    } else                                                                     {
      const QString & cwd = this->parent()->path.pwd()                         ;
      this -> parent ( ) -> path . cwd  ( extracted_path          )            ;
      this -> parent ( ) -> path . nlst ( list,parent->UTF8mode() )            ;
      this -> parent ( ) -> path . cwd  ( cwd                     )            ;
    }                                                                          ;
  } else                                                                       {
    if (extracted_path.isEmpty())                                              {
      this->parent()->path.list(list, parent->UTF8mode())                      ;
    } else                                                                     {
      this->parent()->path.list(list, extracted_path, parent->UTF8mode())      ;
    }                                                                          ;
  }                                                                            ;
  //////////////////////////////////////////////////////////////////////////////
  dead                 = false                                                 ;
  k                    = 0                                                     ;
  this->parent()->skip = 0                                                     ;
  this->parent()->send("150 Opening ASCII mode data connection for file list.");
  //////////////////////////////////////////////////////////////////////////////
  nConnect ( this->parent()->passv , SIGNAL (bytesWritten(qint64))             ,
             this                  , SLOT   (sendMore    ())                 ) ;
  nConnect ( this->parent()->passv , SIGNAL (error(QAbstractSocket::SocketError)),
             this                  , SLOT   (error       ())                 ) ;
  nConnect ( this->parent()->passv , SIGNAL (disconnected())                   ,
             this                  , SLOT   (error       ())                 ) ;
  //////////////////////////////////////////////////////////////////////////////
  if (this->parent()->passv->state() == QTcpSocket::ConnectedState)            {
    sendMore ( )                                                               ;
  } else                                                                       {
    nConnect ( this->parent()->passv , SIGNAL ( connected() )                  ,
               this                  , SLOT   ( sendMore () )                ) ;
  }                                                                            ;
}

N::LISTcmd::~LISTcmd(void)
{
}

N::FtpThread * N::LISTcmd::parent(void) const
{
  return static_cast<FtpThread *> ( QObject :: parent ( ) ) ;
}

void N::LISTcmd::sendMore(void)
{
  if (dead) return                                              ;
  if (parent()->mode != FtpThread::NONE)                        {
    finishing ( )                                               ;
    return                                                      ;
  }                                                             ;
  if (parent()->passv->bytesToWrite() > 0) return               ;
  if (list.isEmpty())                                           {
    finishing()                                                 ;
    return                                                      ;
  }                                                             ;
  ///////////////////////////////////////////////////////////////
  const QByteArray data = list.takeFirst() + QByteArray("\r\n") ;
  parent()->passv->write(data)                                  ;
  FileSharing::pInstance->statsSend(data.size())                ;
  if (k > 100)                                                  {
    emit activity ( )                                           ;
    k = 0                                                       ;
  }                                                             ;
  ++k                                                           ;
}

void N::LISTcmd::error(bool killDTP)
{
  disconnect    ( this->parent()->passv , 0 , this , 0                     ) ;
  list . clear  (                                                          ) ;
  if (killDTP) parent ( ) -> killDTP ( )                                     ;
          else parent ( ) -> mode = FtpThread::NONE                          ;
  parent()->send("451 Requested action aborted. Local error in processing.") ;
  emit activity (                                                          ) ;
  emit finished (                                                          ) ;
  deleteLater   (                                                          ) ;
  dead = true                                                                ;
}

void N::LISTcmd::finishing(void)
{
  disconnect            ( this->parent()->passv , 0 , this , 0 ) ;
  list . clear          (                                      ) ;
  parent ( ) -> killDTP (                                      ) ;
  parent ( ) -> send    ( "226 Transfer complete"              ) ;
  emit activity         (                                      ) ;
  emit finished         (                                      ) ;
  deleteLater           (                                      ) ;
  dead = true                                                    ;
}
