#include <qtftp.h>

N::BonjourRegistrar:: BonjourRegistrar (QObject * parent)
                    : QObject          (          parent)
                    , bonjourSocket    (0               )
{
}

N::BonjourRegistrar::~BonjourRegistrar(void)
{
}

const N::BonjourRecord & N::BonjourRegistrar::registeredRecord(void) const
{
  return finalRecord ;
}

void N::BonjourRegistrar::registerService(const BonjourRecord & bonjour,quint16)
{ Q_UNUSED ( bonjour ) ;
}

void N::BonjourRegistrar::bonjourSocketReadyRead(void)
{
}
