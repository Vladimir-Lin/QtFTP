#include <qtftp.h>

N::FtpUserID:: FtpUserID(void)
{
}

N::FtpUserID:: FtpUserID (const QString & login,const QString & password                              )
             : login     (                login                                                       )
             , password  (QCryptographicHash::hash(password.toUtf8(),QCryptographicHash::Sha1).toHex())
{
}

N::FtpUserID:: FtpUserID ( const QString    & login    ,
                           const QByteArray & password )
             : login     (                    login    )
             , password  (                    password )
{
}

N::FtpUserID:: FtpUserID (const FtpUserID & uid)
{
  login    = uid . login    ;
  password = uid . password ;
  home     = uid . home     ;
}

N::FtpUserID::~FtpUserID (void)
{
}

bool N::FtpUserID::operator == (const FtpUserID & id) const
{
  return ( login    == id.login    ) &&
         ( password == id.password )  ;
}

bool N::FtpUserID::equal(const FtpUserID & id) const
{
  return ( login    == id.login    ) &&
         ( password == id.password )  ;
}

const QString & N::FtpUserID::getLogin(void) const
{
  return login ;
}

const QByteArray & N::FtpUserID::getPassword(void) const
{
  return password ;
}

const QString & N::FtpUserID::getHome(void) const
{
  return home ;
}

void N::FtpUserID::setHome(QString homePath)
{
  home = homePath ;
}
