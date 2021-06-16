#include <qtftp.h>

N::BonjourRecord:: BonjourRecord(void)
{
}

N::BonjourRecord:: BonjourRecord             (
                     const QString & name    ,
                     const QString & regType ,
                     const QString & domain  )
                 : serviceName    ( name     )
                 , registeredType ( regType  )
                 , replyDomain    ( domain   )
{
}

N::BonjourRecord:: BonjourRecord          (
                     const char * name    ,
                     const char * regType ,
                     const char * domain  )
{
  serviceName    = QString :: fromUtf8 ( name    ) ;
  registeredType = QString :: fromUtf8 ( regType ) ;
  replyDomain    = QString :: fromUtf8 ( domain  ) ;
}

N::BonjourRecord::~BonjourRecord(void)
{
}

bool N::BonjourRecord::operator == (const BonjourRecord & other) const
{
  return ( serviceName    == other.serviceName    &&
           registeredType == other.registeredType &&
           replyDomain    == other.replyDomain   ) ;
}
