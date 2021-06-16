#include <qtftp.h>

N::IpcConnection:: IpcConnection (QLocalSocket * parent)
                 : QObject       (               parent)
{
  nConnect ( parent , SIGNAL ( readyRead    ())                       ,
             this   , SLOT   ( readLine     ())                     ) ;
  nConnect ( parent , SIGNAL ( error(QLocalSocket::LocalSocketError)) ,
             parent , SLOT   ( deleteLater  ())                     ) ;
  nConnect ( parent , SIGNAL ( disconnected ())                       ,
             parent , SLOT   ( deleteLater  ())                     ) ;
}

N::IpcConnection::~IpcConnection(void)
{
}

QLocalSocket * N::IpcConnection::parent(void)
{
  return static_cast<QLocalSocket *>(QObject::parent());
}

void N::IpcConnection::readLine(void)
{
  while (parent()->canReadLine())                     {
    emit message(parent()->readLine(65536).trimmed()) ;
  }                                                   ;
}

void N::IpcConnection::sendMessage(const QByteArray & msg)
{
  parent ( ) -> write ( msg + '\n' ) ;
}
