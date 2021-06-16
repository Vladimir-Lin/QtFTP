#include <qtftp.h>

N::FINDcmd:: FINDcmd (const FtpUserID & uid     ,
                      const QString   & request ,
                      FtpThread       * parent  )
           : QObject (                  parent  )
           , uid     (                  uid     )
{
  this->parent()->send("150 Opening BINARY mode data connection for FIND " + request) ;
  nConnect ( this->parent()->passv , SIGNAL (error(QAbstractSocket::SocketError)) ,
             this                  , SLOT   (finish      ())                    ) ;
  nConnect ( this->parent()->passv , SIGNAL (disconnected())                      ,
             this                  , SLOT   (finish      ())                    ) ;
  //////////////////////////////////////////////////////////////////////////////
  workQueue . enqueue       ( QString() )                                      ;
  dead      = false                                                            ;
  req       = request                                                          ;
  k         = 0                                                                ;
  timer     = new QTimer    ( this      )                                      ;
  timer    -> setInterval   ( 10        )                                      ;
  timer    -> setSingleShot ( false     )                                      ;
  nConnect ( timer , SIGNAL(timeout()) , this , SLOT(process()) )              ;
  timer    -> start         (           )                                      ;
}

N::FINDcmd::~FINDcmd(void)
{
  disconnect ( this )        ;
  dead = true                ;
  if (NotNull(timer))        {
    timer -> stop        ( ) ;
    timer -> deleteLater ( ) ;
  }                          ;
}

N::FtpThread * N::FINDcmd::parent(void) const
{
  return static_cast<FtpThread *> ( QObject :: parent ( ) ) ;
}

void N::FINDcmd::finish(void)
{
  dead = true                                       ;
  disconnect            ( this                    ) ;
  parent ( ) -> killDTP (                         ) ;
  parent ( ) -> send    ( "226 Transfer complete" ) ;
  emit activity         (                         ) ;
  emit finished         (                         ) ;
  deleteLater           (                         ) ;
}

void N::FINDcmd::process(void)
{
  if (dead) return                                               ;
  if (workQueue.isEmpty() || parent()->mode != FtpThread::FIND)  {
    finish ( )                                                   ;
    return                                                       ;
  }                                                              ;
  ////////////////////////////////////////////////////////////////
  const QRegExp test    = QRegExp ( req                          ,
                                   Qt::CaseInsensitive           ,
                                   QRegExp::Wildcard          )  ;
  const QString cur     = workQueue     . dequeue  (          )  ;
  QStringList   entries = FtpUsersPath :: listDirs ( uid, cur )  ;
  ////////////////////////////////////////////////////////////////
  foreach ( QString entry , entries )                            {
    if ( entry == "." || entry == ".." ) continue                ;
    workQueue . enqueue ( cur + "/" + entry )                    ;
  }                                                              ;
  ////////////////////////////////////////////////////////////////
  entries = FtpUsersPath :: listAll ( uid , cur )                ;
  foreach ( QString entry , entries )                            {
    if ( dead                          ) return                  ;
    if ( entry == "." || entry == ".." ) continue                ;
    if ( !test.exactMatch(entry)       ) continue                ;
    QByteArray data                                              ;
    data = ( cur + FtpPath::separator + entry).toUtf8() + "\r\n" ;
    int written = 0                                              ;
    while (written != -1                                        &&
           written < data.size()                                &&
           parent()->mode == FtpThread::FIND                    &&
           parent()->passv->isValid()                           &&
           parent()->passv->isOpen ()                          ) {
      data    . remove ( 0 , written )                           ;
      written = parent()->passv->write(data)                     ;
      if ( dead ) return                                         ;
      while ( parent()->passv->isValid()                        &&
              parent()->mode == parent()->FIND                  &&
              parent()->passv->isOpen()                         &&
              parent()->passv->bytesToWrite() > 0              ) {
        parent ( ) -> passv -> waitForBytesWritten ( 10 )        ;
        if ( dead ) return                                       ;
      }                                                          ;
      FileSharing :: pInstance -> statsSend ( data . size ( ) )  ;
      if (k > 100)                                               {
        emit activity ( )                                        ;
        k = 0                                                    ;
      }                                                          ;
      ++k                                                        ;
    }                                                            ;
    if ( parent ( ) -> mode != FtpThread::FIND ) break           ;
  }                                                              ;
}
