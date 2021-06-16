#include <qtftp.h>

N::IpFileServer * N::IpFileServer::pInstance = NULL ;

N::IpFileServer:: IpFileServer      ( void                 )
                : TcpSharing        ( "IpFileServer",16845 )
                , nextTransactionID ( 0                    )
                , fileID            ( 0                    )
{
}

N::IpFileServer::~IpFileServer(void)
{
}

N::IpFileServer * N::IpFileServer::instance(void)
{
  if (!pInstance) pInstance = new IpFileServer() ;
  return pInstance                               ;
}

QByteArray N::IpFileServer::waitForID(const quint32 ID)
{
  QByteArray result                 ;
  QTime      timer                  ;
  timer . start ( )                 ;
  while(!messageQueue.contains(ID)) {
    waitForData ( 10 )              ;
    if (timer.elapsed() >= 10000)   {
      messageQueue . remove ( ID )  ;
      return result                 ;
    }                               ;
  }                                 ;
  result = messageQueue [ ID ]      ;
  messageQueue . remove ( ID )      ;
  return result                     ;
}

void N::IpFileServer::receive(const QByteArray & message)
{
  QDataStream msg ( message )                                                ;
  QString     type                                                           ;
  msg  >>     type                                                           ;
  ////////////////////////////////////////////////////////////////////////////
  if (type == "answer")                                                      {
    quint32 ID                                                               ;
    QString sender                                                           ;
    msg >> sender >> ID                                                      ;
    if (sender == FtpSettings::User())                                       {
      messageQueue[ID] = message.right(message.length()-msg.device()->pos()) ;
    }                                                                        ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if (type == "seek")                                                        {
    quint32 ID                                                               ;
    QString sender                                                           ;
    QString user                                                             ;
    quint32 fID                                                              ;
    qint64  offset                                                           ;
    msg >> sender >> ID >> user >> fID >> offset                             ;
    if (user != FtpSettings::User()) return                                  ;
    send ( "answer" , sender , ID , seek ( user , fID , offset ) )           ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if (type == "open")                                                        {
    quint32   ID                                                             ;
    QString   sender                                                         ;
    QString   user                                                           ;
    QString   filename                                                       ;
    int       mode                                                           ;
    FtpUserID uid                                                            ;
    msg >> sender >> ID >> uid >> user >> filename >> mode                   ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer"                                                          ,
           sender                                                            ,
           ID                                                                ,
           open ( uid , user , filename , QFile::OpenMode(mode) )          ) ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if (type == "close")                                                       {
    quint32 ID                                                               ;
    QString sender                                                           ;
    QString user                                                             ;
    quint32 fID                                                              ;
    msg >> sender >> ID >> user >> fID                                       ;
    if ( user != FtpSettings::User() ) return                                ;
    close ( user     , fID         )                                         ;
    send  ( "answer" , sender , ID )                                         ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if (type == "read")                                                        {
    quint32 ID                                                               ;
    QString sender                                                           ;
    QString user                                                             ;
    quint32 fID                                                              ;
    qint64  maxlen                                                           ;
    msg >> sender >> ID >> user >> fID >> maxlen                             ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , read ( user , fID , maxlen ) )           ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if (type == "write")                                                       {
    quint32    ID                                                            ;
    QString    sender                                                        ;
    QString    user                                                          ;
    quint32    fID                                                           ;
    QByteArray data                                                          ;
    msg >> sender >> ID >> user >> fID >> data                               ;
    if ( user != FtpSettings::User() ) return                                ;
    write ( user     , fID    , data )                                       ;
    send  ( "answer" , sender , ID   )                                       ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if (type == "flush")                                                       {
    quint32 ID                                                               ;
    QString sender                                                           ;
    QString user                                                             ;
    quint32 fID                                                              ;
    msg >> sender >> ID >> user >> fID                                       ;
    if ( user != FtpSettings::User() ) return                                ;
    flush ( user     , fID         )                                         ;
    send  ( "answer" , sender , ID )                                         ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if (type == "isReadable")                                                  {
    quint32 ID                                                               ;
    QString sender                                                           ;
    QString user                                                             ;
    quint32 fID                                                              ;
    msg >> sender >> ID >> user >> fID                                       ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , isReadable ( user , fID ) )              ;
    return                                                                   ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if (type == "isWritable")                                                  {
    quint32 ID                                                               ;
    QString sender                                                           ;
    QString user                                                             ;
    quint32 fID                                                              ;
    msg >> sender >> ID >> user >> fID                                       ;
    if ( user != FtpSettings::User() ) return                                ;
    send ( "answer" , sender , ID , isWritable ( user , fID ) )              ;
    return                                                                   ;
  }                                                                          ;
}

bool N::IpFileServer::seek(const QString & user,quint32 ID,qint64 offset)
{
  if (user != FtpSettings::User())                              {
    QByteArray  result = remote ( "seek" , user , ID , offset ) ;
    QDataStream reader          ( result                      ) ;
    bool      b                                                 ;
    reader >> b                                                 ;
    return    b                                                 ;
  }                                                             ;
  if ( ! files . contains ( ID ) ) return true                  ;
  return files [ ID ] -> seek ( offset )                        ;
}

bool N::IpFileServer::isReadable(const QString & user,quint32 ID)
{
  if ( user != FtpSettings::User() )                         {
    QByteArray  result = remote ( "isReadable" , user , ID ) ;
    QDataStream reader          ( result                   ) ;
    bool      b                                              ;
    reader >> b                                              ;
    return    b                                              ;
  }                                                          ;
  if (!files.contains(ID)) return false                      ;
  return files [ ID ] -> isReadable ( )                      ;
}

bool N::IpFileServer::isWritable(const QString & user,quint32 ID)
{
  if ( user != FtpSettings::User() )                         {
    QByteArray  result = remote ( "isWritable" , user , ID ) ;
    QDataStream reader          ( result                   ) ;
    bool b                                                   ;
    reader >> b                                              ;
    return b                                                 ;
  }                                                          ;
  if (!files.contains(ID)) return false                      ;
  return files [ ID ] -> isWritable ( )                      ;
}

quint32 N::IpFileServer::open ( const FtpUserID & uid      ,
                                const QString   & user     ,
                                const QString   & filename ,
                                QFile::OpenMode   mode     )
{
  if (user != FtpSettings::User())                                            {
    QByteArray result = remote ( "open" , uid , user , filename , int(mode) ) ;
    QDataStream reader         ( result                                     ) ;
    quint32   ID                                                              ;
    reader >> ID                                                              ;
    return    ID                                                              ;
  }                                                                           ;
  if ( ! FtpSettings :: checkID ( uid , true ) ) return 0U                    ;
  if ( ! FtpSettings :: checkID ( uid        ) && mode & QFile::WriteOnly )   {
    return 0U                                                                 ;
  }                                                                           ;
  /////////////////////////////////////////////////////////////////////////////
  ++fileID                                                                    ;
  quint32 ID = fileID                                                         ;
  QString realFilename                                                        ;
  realFilename = FtpSharedFolders::instance()->getRealPath(filename)          ;
  files [ ID ]  = new QFile ( realFilename , this )                           ;
  files [ ID ] -> open      ( mode                )                           ;
  return  ID                                                                  ;
}

void N::IpFileServer::close(const QString & user,quint32 ID)
{
  if ( user != FtpSettings::User() ) {
    remote ( "close" , user , ID )   ;
    return                           ;
  }                                  ;
  if (!files.contains(ID)) return    ;
  files [ ID ] -> close ( )          ;
  delete files   [ ID ]              ;
  files . remove ( ID )              ;
}

void N::IpFileServer::flush(const QString & user,quint32 ID)
{
  if ( user != FtpSettings::User() ) {
    remote ( "flush" , user , ID )   ;
    return                           ;
  }                                  ;
  if (!files.contains(ID)) return    ;
  files [ ID ] -> flush ( )          ;
}

QByteArray N::IpFileServer::read(const QString & user,quint32 ID,qint64 maxlen)
{
  if ( user != FtpSettings::User() )                            {
    QByteArray  result = remote ( "read" , user , ID , maxlen ) ;
    QDataStream reader          ( result                      ) ;
    QByteArray data                                             ;
    reader  >> data                                             ;
    return     data                                             ;
  }                                                             ;
  if (!files.contains(ID)) return QByteArray ( )                ;
  return files [ ID ] -> read ( maxlen )                        ;
}

void N::IpFileServer::write(const QString & user,quint32 ID,const QByteArray & data)
{
  if ( user != FtpSettings::User() )      {
    remote ( "write" , user , ID , data ) ;
    return                                ;
  }                                       ;
  if (!files.contains(ID)) return         ;
  files [ ID ] -> write ( data )          ;
}
