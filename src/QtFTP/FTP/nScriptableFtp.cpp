#include <qtftp.h>

N::ScriptableFtp:: ScriptableFtp ( QObject * parent )
                 : QObject       (           parent )
                 , QScriptable   (                  )
                 , Thread        ( 0 , false        )
                 , Ftp           (                  )
{
  setEnabled ( "Explain" , true ) ;
}

N::ScriptableFtp::~ScriptableFtp (void)
{
}

bool N::ScriptableFtp::SetEnabled(int Id,bool enable)
{
  return N::Enabler::setEnabled ( Id , enable ) ;
}

bool N::ScriptableFtp::IsEnabled(int Id)
{
  return N::Enabler::isEnabled ( Id ) ;
}

bool N::ScriptableFtp::SetEnabled(QString Id,bool enable)
{
  return N::Enabler::setEnabled ( Id , enable ) ;
}

bool N::ScriptableFtp::IsEnabled(QString Id)
{
  return N::Enabler::isEnabled ( Id ) ;
}

QString N::ScriptableFtp::HttpHeader (void)
{
  return QString::fromUtf8 ( dlHeader ) ;
}

void N::ScriptableFtp::setRequest(QString key,QString value)
{
  Requests [ key ] = value ;
}

bool N::ScriptableFtp::Header (QString url,int t)
{
  QUrl       u       ( url   ) ;
  QByteArray b                 ;
  b = QtCURL::header ( u , t ) ;
  return ( b.size() > 0 )      ;
}

bool N::ScriptableFtp::Download (QString url,QString filename,int t)
{
  QUrl  u                 ( url       ) ;
  QFile f                 ( filename  ) ;
  return QtCURL::download ( u , f , t ) ;
}

void N::ScriptableFtp::DownloadFile(VarArgs & args)
{
  QString url                                                       ;
  QString filename                                                  ;
  int     t = 60 * 1000                                             ;
  if ( args     . count  ( ) >  0 ) url      = args[0].toString ( ) ;
  if ( args     . count  ( ) >  1 ) filename = args[1].toString ( ) ;
  if ( args     . count  ( ) >  2 ) t        = args[2].toInt    ( ) ;
  if ( url      . length ( ) <= 0 ) return                          ;
  if ( filename . length ( ) <= 0 ) return                          ;
  ///////////////////////////////////////////////////////////////////
  Download ( url , filename , t )                                   ;
}

void N::ScriptableFtp::run(int type,ThreadData * data)
{
  switch ( type )                      {
    case 10001                         :
      DownloadFile ( data->Arguments ) ;
    break                              ;
  }                                    ;
}

bool N::ScriptableFtp::Start(QString command)
{
  QStringList s = CommandTokens ( command )       ;
  QString     p                                   ;
  QString     l                                   ;
  VarArgs     args                                ;
  if ( s . count ( ) <= 0 ) return false          ;
  p = s . first   (   )                           ;
  s . takeAt      ( 0 )                           ;
  l = p . toLower (   )                           ;
  /////////////////////////////////////////////////
  if ( "download" == l && ( s  .count ( ) > 1 ) ) {
    args << s                                     ;
    start ( 10001 , args )                        ;
    return true                                   ;
  }                                               ;
  /////////////////////////////////////////////////
  return false                                    ;
}

QScriptValue N::FtpAttachement(QScriptContext * context,QScriptEngine * engine)
{
  N::ScriptableFtp * ftp = new N::ScriptableFtp ( engine ) ;
  return engine -> newQObject ( ftp )                      ;
}
