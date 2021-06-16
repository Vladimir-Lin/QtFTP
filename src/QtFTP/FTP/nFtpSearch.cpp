#include <qtftp.h>

N::FtpSearch:: FtpSearch ( const QString & computer ,
                           const QString & request  ,
                           QObject       * parent   )
             : QObject   (                 parent   )
{
  this -> computer = computer                                             ;
  this -> request  = request                                              ;
  sock             = new QTcpSocket ( this )                              ;
  nConnect ( sock , SIGNAL (error      (QAbstractSocket::SocketError) )   ,
             this , SLOT   (stopSearch (                            ) ) ) ;
}

N::FtpSearch::~FtpSearch(void)
{
}

void N::FtpSearch::start(void)
{
  nConnect ( sock , SIGNAL(connected()) , this , SLOT(connected   ()) )     ;
  nConnect ( sock , SIGNAL(readyRead()) , this , SLOT(processReply()) )     ;
  ///////////////////////////////////////////////////////////////////////////
  FtpComputer server = NetDiscovery::instance()->getComputer(computer)      ;
  sock -> connectToHost ( server . getHostname ( ) , server . getPort ( ) ) ;
}

void N::FtpSearch::connected(void)
{
  sock -> write ( "USER anonymous\r\n" ) ;
  sock -> write ( "PASV\r\n"           ) ;
  sock -> flush (                      ) ;
}

void N::FtpSearch::processReply(void)
{
  while(sock->canReadLine())                                                   {
    QByteArray line = sock->readLine(65536)                                    ;
    int code = line.left(3).toInt()                                            ;
    QByteArray details = line.right(line.size() - 3).trimmed()                 ;
    if (code == 227)                                                           { // Enter passive mode
      QList<QByteArray> args = line.split(',')                                 ;
      QList<QByteArray> last = args.last().split(')')                          ;
      int port = 256 * args.at(args.size() - 2).toInt() + last.first().toInt() ;
      dtp = new QTcpSocket ( this )                                            ;
      nConnect ( dtp  , SIGNAL (readyRead  ())                                 ,
                 this , SLOT   (readResults())                               ) ;
      dtp  -> connectToHost ( sock->peerAddress(),port       )                 ;
      sock -> write ( ("FIND " + request + "\r\n").toUtf8()  )                 ;
    } else
    if (code == 226)                                                           { // We're done
      stopSearch ( )                                                           ;
    } else
    if (code / 100 > 2)                                                        { // There is an error
      stopSearch ( )                                                           ;
    }                                                                          ;
  }                                                                            ;
}

void N::FtpSearch::readResults(void)
{
  while (dtp->canReadLine())                                        {
    QByteArray line = dtp->readLine(65536).trimmed()                ;
    emit found(computer,QString::fromUtf8(line.data(),line.size())) ;
  }                                                                 ;
}

void N::FtpSearch::stopSearch(void)
{
  if (sock->isOpen()) sock -> close ( ) ;
  deleteLater ( )                       ;
}
