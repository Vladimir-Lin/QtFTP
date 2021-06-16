/****************************************************************************
 *                                                                          *
 * Copyright (C) 2015 Neutrino International Inc.                           *
 *                                                                          *
 * Author : Brian Lin <lin.foxman@gmail.com>, Skype: wolfram_lin            *
 *                                                                          *
 ****************************************************************************/

#ifndef QT_FTP_H
#define QT_FTP_H

#include <QtCore>
#include <QtNetwork>
#include <QtSql>
#include <QtScript>
#include <QtCURL>
#include <Essentials>
#include <NetProtocol>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#    if defined(QT_BUILD_QTFTP_LIB)
#      define Q_FTP_EXPORT Q_DECL_EXPORT
#    else
#      define Q_FTP_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define Q_FTP_EXPORT
#endif

class Q_FTP_EXPORT QUrlInfoPrivate  ;
class Q_FTP_EXPORT QUrlInfo         ;
class Q_FTP_EXPORT QFtpPI           ;
class Q_FTP_EXPORT QFtpDTP          ;
class Q_FTP_EXPORT QFtpCommand      ;
class Q_FTP_EXPORT QFtpPrivate      ;
class Q_FTP_EXPORT QFtp             ;

namespace N                         {

class Q_FTP_EXPORT Ftp              ;
class Q_FTP_EXPORT ScriptableFtp    ;
class Q_FTP_EXPORT FtpProtocol      ;
class Q_FTP_EXPORT FtpOptions       ;
class Q_FTP_EXPORT FtpEntry         ;
class Q_FTP_EXPORT FtpUser          ;
class Q_FTP_EXPORT FtpServer        ;
class Q_FTP_EXPORT BonjourRecord    ;
class Q_FTP_EXPORT BonjourRegistrar ;
class Q_FTP_EXPORT TcpSharing       ;
class Q_FTP_EXPORT NetDiscovery     ;
class Q_FTP_EXPORT IpFile           ;
class Q_FTP_EXPORT IpFileServer     ;
class Q_FTP_EXPORT IpcConnection    ;
class Q_FTP_EXPORT IpcSocket        ;
class Q_FTP_EXPORT STORcmd          ;
class Q_FTP_EXPORT RETRcmd          ;
class Q_FTP_EXPORT LISTcmd          ;
class Q_FTP_EXPORT FINDcmd          ;
class Q_FTP_EXPORT FtpUserID        ;
class Q_FTP_EXPORT FtpComputer      ;
class Q_FTP_EXPORT FtpSettings      ;
class Q_FTP_EXPORT FtpFolder        ;
class Q_FTP_EXPORT FtpSharedFolders ;
class Q_FTP_EXPORT FtpPath          ;
class Q_FTP_EXPORT FtpUsersPath     ;
class Q_FTP_EXPORT FtpSharedPath    ;
class Q_FTP_EXPORT FtpDelay         ;
class Q_FTP_EXPORT FtpSearch        ;
class Q_FTP_EXPORT FtpThread        ;
class Q_FTP_EXPORT FileSharing      ;
class Q_FTP_EXPORT FtpCommands      ;
class Q_FTP_EXPORT FtpClient        ;

/*****************************************************************************\
 *                                                                           *
 *                         File Transfer Protocol / FTP                      *
 *                                                                           *
\*****************************************************************************/

class Q_FTP_EXPORT Ftp : public QtCURL
{
  public:

    explicit Ftp (void) ;
    virtual ~Ftp (void) ;

  protected:

  private:

};

class Q_FTP_EXPORT ScriptableFtp : public QObject
                                 , public QScriptable
                                 , public Thread
                                 , public Enabler
                                 , public Ftp
{
  Q_OBJECT
  public:

    explicit        ScriptableFtp (QObject * parent) ;
    virtual        ~ScriptableFtp (void) ;

  protected:

    virtual void    run           (int type,ThreadData * data) ;

  private:

    virtual void    DownloadFile  (VarArgs & args) ;

  public slots:

    virtual bool    SetEnabled    (int Id,bool enable) ;
    virtual bool    IsEnabled     (int Id) ;
    virtual bool    SetEnabled    (QString Id,bool enable) ;
    virtual bool    IsEnabled     (QString Id) ;

    virtual QString HttpHeader    (void) ;

    virtual void    setRequest    (QString key,QString value) ;
    virtual bool    Header        (QString url,int mtimeout) ;
    virtual bool    Download      (QString url,QString filename,int mtimeout) ;
    virtual bool    Start         (QString command) ;

  protected slots:

  private slots:

  signals:

};

Q_FTP_EXPORT QScriptValue FtpAttachement(QScriptContext * context,QScriptEngine * engine) ;

class Q_FTP_EXPORT FtpProtocol : public NetProtocol
{ // FTP protocol interpreter
  public:

    explicit FtpProtocol (void) ;
    virtual ~FtpProtocol (void) ;

    virtual int  type    (void) const ; // RFC 959

    virtual bool In      (int size,char * data) ;
    virtual bool In      (QByteArray & data) ;
    virtual bool Out     (QByteArray & data) ;

  protected:

  private:

};

class Q_FTP_EXPORT FtpOptions
{ // FTP Server environment
  public:

    bool deleteBeforeClosed ;
    bool duplicate          ;

    explicit FtpOptions (void) ;
    virtual ~FtpOptions (void) ;

  protected:

  private:

};

class Q_FTP_EXPORT FtpEntry : public Thread
{ // FTP Transfer Session
  public:

    friend class FtpServer ;
    friend class FtpUser   ;

    Socket       Connector ;
    FtpOptions * Options   ;

    explicit FtpEntry (void) ;
    virtual ~FtpEntry (void) ;

  protected:

    virtual void run  (void) ;

  private:

};

class Q_FTP_EXPORT FtpUser : public Thread
{ // FTP User Session
  public:

    friend class FtpServer ;
    friend class FtpEntry  ;

    Socket       Connector ;
    FtpProtocol  Protocol  ;
    FtpOptions * Options   ;

    explicit FtpUser (void) ;
    virtual ~FtpUser (void) ;

  protected:

    virtual void run (void) ;

  private:

};

class Q_FTP_EXPORT FtpServer : public Thread
                             , public TcpServer
{ // FTP Server Protocl
  public:

    friend class FtpEntry ;
    friend class FtpUser  ;

    RangeInt   Ports   ;
    FtpOptions Options ;

    explicit       FtpServer     (void) ;
    virtual       ~FtpServer     (void) ;

    virtual int type   (void) const ; // Normally, the lastest RFC number

    bool              isListening                 (void) const ;
    bool              isAccepting                 (void) const ;
    unsigned short    listeningPort               (void) const ;
    bool              isFXPEnabled                (void) const ;
    unsigned long int GetNoLoginTimeout           (void) const ;
    unsigned long int GetNoTransferTimeout        (void) const ;
    unsigned int      GetCheckPassDelay           (void) const ;
    unsigned int      GetMaxPasswordTries         (void) const ;
    unsigned int      GetTransferBufferSize       (void) const ;
    unsigned int      GetTransferSocketBufferSize (void) const ;
    unsigned int      GetNbClient                 (void) const ;
    unsigned int      GetNbUser                   (void) const ;

    void EnableFXP                   ( bool              bEnable        ) ;
    void SetNoLoginTimeout           ( unsigned long int ulSecond       ) ;
    void SetNoTransferTimeout        ( unsigned long int ulSecond       ) ;
    void SetCheckPassDelay           ( unsigned int      ulMilliSecond  ) ;
    void SetMaxPasswordTries         ( unsigned int      uiMaxPassTries ) ;
    void SetTransferBufferSize       ( unsigned int      uiSize         ) ;
    void SetTransferSocketBufferSize ( unsigned int      uiSize         ) ;

    bool StopListening               (void) ;

  protected:

    virtual void run                 (void) ;

  private:

    bool               bIsListening               ;
    bool               bIsAccepting               ;
    unsigned short int usListeningPort            ;
    unsigned int       uiNumberOfUser             ;
    unsigned int       uiNumberOfClient           ;
    unsigned int       uiMaxPasswordTries         ;
    unsigned int       uiCheckPassDelay           ;
    unsigned long int  ulNoTransferTimeout        ;
    unsigned long int  ulNoLoginTimeout           ;
    unsigned int       uiTransferBufferSize       ;
    unsigned int       uiTransferSocketBufferSize ;
    bool               bEnableFXP                 ;

};

/*****************************************************************************\
 *                                                                           *
 *                            File sharing system                            *
 *                                                                           *
\*****************************************************************************/

class Q_FTP_EXPORT BonjourRecord
{
  public:

    QString serviceName    ;
    QString registeredType ;
    QString replyDomain    ;

    explicit BonjourRecord (void) ;
    explicit BonjourRecord (const QString & name     ,
                            const QString & regType  ,
                            const QString & domain ) ;
    explicit BonjourRecord (const char    * name     ,
                            const char    * regType  ,
                            const char    * domain ) ;
    virtual ~BonjourRecord (void);

    bool operator ==       (const BonjourRecord & other) const ;

  protected:

  private:

};

class Q_FTP_EXPORT BonjourRegistrar : public QObject
{
  Q_OBJECT
  public:

    explicit BonjourRegistrar (QObject * parent = NULL) ;
    virtual ~BonjourRegistrar (void) ;

    void                  registerService  (const BonjourRecord & record       ,
                                            quint16               servicePort) ;
    const BonjourRecord & registeredRecord (void) const ;

  protected:

    BonjourRecord     finalRecord   ;
    QSocketNotifier * bonjourSocket ;

  private:

  public slots:

  protected slots:

  private slots:

   void bonjourSocketReadyRead(void) ;

  signals:

    void error             (int error) ;
    void serviceRegistered (const BonjourRecord & record) ;

};

class Q_FTP_EXPORT FtpUserID
{
  public:

    explicit FtpUserID (void) ;
    explicit FtpUserID (const QString & login,const QString    & password) ;
    explicit FtpUserID (const QString & login,const QByteArray & password) ;
             FtpUserID (const FtpUserID & uid) ;
    virtual ~FtpUserID (void) ;

    bool               operator == (const FtpUserID & id) const ;
    bool               equal       (const FtpUserID & id) const ;
    const QString    & getLogin    (void) const ;
    const QByteArray & getPassword (void) const ;
    const QString    & getHome     (void) const ;
    void               setHome     (QString homePath) ;

  protected:

    QString    login    ;
    QByteArray password ;
    QString    home     ;

  private:

};

class Q_FTP_EXPORT FtpComputer : public QObject
{
  Q_OBJECT
  public:

    explicit FtpComputer      (void) ;
    explicit FtpComputer      (const QByteArray  & data) ;
             FtpComputer      (const FtpComputer & c   ) ;
    virtual ~FtpComputer      (void) ;

    FtpComputer & operator  = (const FtpComputer & c) ;
    bool          operator != (const FtpComputer & c) ;

    QString       getHostname (void) const ;
    QString       getName     (void) const ;
    int           getPort     (void) const ;
    bool          timedOut    (void) const ;
    QByteArray    serialize   (void) const ;
    void          unserialize (const QByteArray & data) ;

  protected:

    QString hostname ;
    int     port     ;
    QString name     ;
    QTime * time     ;

  private:

  public slots:

    void open                 (const QString & path = QString()) ;
    void setHostname          (const QString & hostname) ;
    void setName              (const QString & name) ;
    void setPort              (const int port) ;

  protected slots:

  private slots:

  signals:

    void look                 (QUrl address) ;

};

class Q_FTP_EXPORT TcpSharing : public QObject
{
  Q_OBJECT
  public:

    explicit TcpSharing (const QString & name         ,
                         const quint16   port         ,
                         QObject       * parent = 0 ) ;
    virtual ~TcpSharing (void) ;

    bool isServing    (void) const ;
    void waitForData  (int msec) ;

  protected:

    virtual void receive (const QByteArray &message) = 0 ;

    void send (const QByteArray & message) ;
    void send (const QString    & message) ;
    void send (const char       * message) ;

    template<class T>
      void send ( const QString & message ,
                  const T       & arg0    )
      {
        QByteArray  data                                    ;
        QDataStream stream ( &data , QIODevice::WriteOnly ) ;
        stream << message << arg0                           ;
        send ( data )                                       ;
      }

    template<class T, class U>
      void send ( const QString & message ,
                  const T       & arg0    ,
                  const U       & arg1    )
      {
        QByteArray  data                                    ;
        QDataStream stream ( &data , QIODevice::WriteOnly ) ;
        stream << message << arg0 << arg1                   ;
        send ( data )                                       ;
      }

    template<class T, class U, class V>
      void send ( const QString & message ,
                  const T       & arg0    ,
                  const U       & arg1    ,
                  const V       & arg2    )
      {
        QByteArray  data                                    ;
        QDataStream stream ( &data , QIODevice::WriteOnly ) ;
        stream << message << arg0 << arg1 << arg2           ;
        send ( data )                                       ;
      }

    template<class T, class U, class V, class W>
      void send ( const QString & message ,
                  const T       & arg0    ,
                  const U       & arg1    ,
                  const V       & arg2    ,
                  const W       & arg3    )
      {
        QByteArray  data                                    ;
        QDataStream stream ( &data , QIODevice::WriteOnly ) ;
        stream << message << arg0 << arg1 << arg2 << arg3   ;
        send ( data )                                       ;
      }

    template<class T, class U, class V, class W, class X>
      void send ( const QString & message ,
                  const T       & arg0    ,
                  const U       & arg1    ,
                  const V       & arg2    ,
                  const W       & arg3    ,
                  const X       & arg4    )
      {
        QByteArray  data                                          ;
        QDataStream stream ( &data , QIODevice::WriteOnly )       ;
        stream << message << arg0 << arg1 << arg2 << arg3 << arg4 ;
        send ( data )                                             ;
      }

    template<class T, class U, class V, class W, class X, class Y>
      void send ( const QString & message ,
                  const T       & arg0    ,
                  const U       & arg1    ,
                  const V       & arg2    ,
                  const W       & arg3    ,
                  const X       & arg4    ,
                  const Y       & arg5    )
      {
        QByteArray  data                                                  ;
        QDataStream stream ( &data , QIODevice::WriteOnly )               ;
        stream << message << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 ;
        send ( data )                                                     ;
      }

  private:

    const quint16      port            ;
    QTcpServer       * server          ;
    QTcpSocket       * client          ;
    QSystemSemaphore * lock            ;
    QMutex             mutex           ;
    bool               bLinkInProgress ;

  public slots:

    void link                (void) ;

  protected slots:

    void processConnection   (void) ;
    void broadcast           (void) ;
    void receive             (void) ;
    void handleDisconnection (void) ;

  private slots:

  signals:

    void enterServerMode     (void) ;
    void enterClientMode     (void) ;
    void clientLeft          (void) ;

};

class Q_FTP_EXPORT IpcConnection : public QObject
{
  Q_OBJECT
  public:

    explicit IpcConnection (QLocalSocket * parent) ;
    virtual ~IpcConnection (void) ;

    QLocalSocket * parent  (void) ;

  protected:

  private:

  public slots:

    void sendMessage       (const QByteArray & msg) ;

  protected slots:

    void readLine          (void);

  signals:

    void message           (const QByteArray & msg) ;

};

class Q_FTP_EXPORT IpcSocket : public QObject
{
  Q_OBJECT
  public:

    enum State      {
      NotConnected  ,
      Client        ,
      Server      } ;

    explicit IpcSocket        (QObject * parent = NULL) ;
    explicit IpcSocket        (const QString & name,QObject * parent = 0) ;
    virtual ~IpcSocket        (void) ;

    void     connectTo        (const QString & name) ;
    State    state            (void) ;

  protected:

  private:

    QLocalServer * sock         ;
    State          currentState ;

  public slots:

    void sendMessage          (const QByteArray & msg) ;

  protected slots:

    void processConnection    (void) ;
    void dispatchMessage      (const QByteArray & msg) ;
    void resetState           (void) ;
    void relayConnectedSignal (void) ;

  private slots:

  signals:

    void message              (const QByteArray & msg) ;
    void sigSendMessage       (const QByteArray & msg) ;
    void disconnecting        (void) ;
    void connected            (void) ;

};

class Q_FTP_EXPORT FtpSettings
{
  public:

    explicit             FtpSettings     (void) ;
    virtual             ~FtpSettings     (void) ;

    static FtpSettings * instance        (void) ;
    static FileSharing * server          (void) ;
    void                 setInstance     (FtpSettings * settings) ;
    static bool          isGui           (void) ;
    static QString       User            (void) ;
    static bool          connect         (FileSharing * sharing) ;
    static bool          connect         (FtpThread   * thread ) ;

    static bool          isAnonymous     (void) ;
    static bool          showHiddenFiles (void) ;
    static bool          checkPassword   (const QString & passwd) ;
    static QString       getLogin        (void) ;
    static QString       getHostname     (void) ;
    static int           getServerPort   (void) ;
    static FtpUserID     getUID          (void) ;

    static bool          checkID         (const FtpUserID & uid,const bool read = false) ;
    static bool          isAnonymousReadAllowed (void) ;

    static bool          Load            (QMap<QString,FtpFolder> & folders) ;
    static bool          Save            (QMap<QString,FtpFolder> & folders) ;

    static bool          addFolder       (QString source,QString ftp,bool writable) ;
    virtual bool         appendFolder    (QString source,QString ftp,bool writable) ;

  protected:

    bool    Gui         ;
    bool    Anonymous   ;
    bool    ReadOnly    ;
    bool    HiddenFiles ;
    int     ServerPort  ;
    QString SystemUser  ;
    QMutex  Mutex       ;

    QString              DefaultUser    (void) ;

    virtual bool         connectSharing (FileSharing * sharing) ;
    virtual bool         connectThread  (FtpThread   * thread ) ;
    virtual bool         verifyPassword (const QString & password) ;
    virtual bool         Password       (QByteArray    & password) ;
    virtual QByteArray   Encrypted      (void) ;
    virtual QString      Login          (void) ;
    virtual QString      Hostname       (void) ;
    virtual FtpUserID    ActualUID      (void) ;

  private:

    static FtpSettings * pInstance ;

};

class Q_FTP_EXPORT FtpFolder
{
  friend class FtpSharedFolders ;
  public:

    explicit FtpFolder          (const QString & path     = QString()  ,
                                 bool            writable = false    ) ;
    virtual ~FtpFolder          (void) ;

    bool            isWritable  (void) const ;
    const QString & getPath     (void) const ;
    void            setWritable (bool writable) ;
    void            setPath     (const QString & path) ;

  protected:

    bool    writable ;
    QString path     ;

  private:


};

class Q_FTP_EXPORT FtpSharedFolders : public QObject
                                    , public QMutex
{
  Q_OBJECT
  public:

   explicit FtpSharedFolders (void) ;
   virtual ~FtpSharedFolders (void) ;

   QMap<QString,FtpFolder> getFolderList (void) ;
   QString                 getRealPath   (const QString & path) ;
   bool                    isWritable    (const QString & path) ;
   int                     nbFolders     (void) ;

   static QString          getParentPath (const QString & path);
   static QString          getFileName   (const QString & path);

   static FtpSharedFolders * instance    (void) ;

   static const QString separator;

  protected:

  private:

    QMap<QString,FtpFolder>   folders   ;
    static FtpSharedFolders * pInstance ;

    void loadFolders  (void) ;
    void saveFolders  (void) ;

  public slots:

    void addFolder    (const QString & name,const FtpFolder & folder) ;
    void removeFolder (const QString & name);

  protected slots:

  private slots:

  signals:

};

class Q_FTP_EXPORT FtpPath
{
  public:

    static const QString separator;

    explicit  FtpPath      (void) ;
    virtual  ~FtpPath      (void) ;

    QString   pwd          (void) ;
    bool      cwd          (const QString & path) ;
    bool      mkd          (const QString & name) ;
    bool      rmd          (const QString & name) ;
    bool      dele         (const QString & name) ;
    bool      rename       (const QString & from,const QString & to) ;
    bool      cdup         (void) ;
    void      list         (QList<QByteArray> & sList,bool utf8 = false) ;
    void      list         (QList<QByteArray> & sList,const QString & filename,bool utf8 = false) ;
    void      nlst         (QList<QByteArray> & sList,bool utf8 = false) ;
    bool      exists       (const QString     & path) ;
    QString   absolute     (const QString     & path) ;
    QString   realPath     (const QString     & path) ;
    void      chmod        (const QString     & path,QByteArray perms) ;
    qint64    size         (const QString     & name) ;
    QDateTime lastModified (const QString     & name) ;
    bool      isReadable   (const QString     & name) ;
    bool      isWritable   (const QString     & name) ;
    void      SetUID       (const FtpUserID   & uid ) ;

  protected:

    QString   currentPath ;
    FtpUserID uid         ;

  private:

};

class Q_FTP_EXPORT FtpUsersPath
{
  public:

    explicit           FtpUsersPath (void) ;
    virtual           ~FtpUsersPath (void) ;

    static bool        mkd          (const FtpUserID   & uid,const QString & name) ;
    static bool        rmd          (const FtpUserID   & uid,const QString & name) ;
    static bool        dele         (const FtpUserID   & uid,const QString & name) ;
    static bool        rename       (const FtpUserID   & uid,const QString & from,const QString & to) ;
    static void        list         (const FtpUserID   & uid,QList<QByteArray> & out,const QString & name, bool utf8 = false) ;
    static void        nlst         (const FtpUserID   & uid,QList<QByteArray> & out,const QString & name, bool utf8 = false) ;
    static bool        exists       (const FtpUserID   & uid,const QString & path) ;
    static void        chmod        (const FtpUserID   & uid,const QString & path,QByteArray perms) ;
    static bool        isDir        (const FtpUserID   & uid,const QString & path) ;
    static bool        isFile       (const FtpUserID   & uid,const QString & path) ;
    static bool        isReadable   (const FtpUserID   & uid,const QString & path) ;
    static bool        isWritable   (const FtpUserID   & uid,const QString & path) ;
    static qint64      size         (const FtpUserID   & uid,const QString & path) ;
    static QDateTime   lastModified (const FtpUserID   & uid,const QString & name) ;
    static QStringList listDirs     (const FtpUserID   & uid,const QString & name) ;
    static QStringList listAll      (const FtpUserID   & uid,const QString & name) ;
    static QString     makePath     (const QStringList & keys) ;

  protected:

    static const QChar separator;

  private:

};

class Q_FTP_EXPORT FtpSharedPath : public TcpSharing
{
  Q_OBJECT
  public:

    explicit FtpSharedPath (void) ;
    virtual ~FtpSharedPath (void) ;

    static FtpSharedPath * instance (void) ;

    bool        mkd          (const FtpUserID & uid,const QString & user,const QString & name) ;
    bool        rmd          (const FtpUserID & uid,const QString & user,const QString & name) ;
    bool        dele         (const FtpUserID & uid,const QString & user,const QString & name) ;
    bool        rename       (const FtpUserID & uid,const QString & user,const QString & from,const QString & to);
    void        list         (const FtpUserID & uid,const QString & user,QList<QByteArray> & out,const QString & name,bool utf8 = false) ;
    void        nlst         (const FtpUserID & uid,const QString & user,QList<QByteArray> & out,const QString & name,bool utf8 = false) ;
    bool        exists       (const FtpUserID & uid,const QString & user,const QString & name) ;
    bool        isDir        (const FtpUserID & uid,const QString & user,const QString & name) ;
    bool        isFile       (const FtpUserID & uid,const QString & user,const QString & name) ;
    void        chmod        (const FtpUserID & uid,const QString & user,const QString & name,QByteArray perms);
    qint64      size         (const FtpUserID & uid,const QString & user,const QString & name) ;
    QDateTime   lastModified (const FtpUserID & uid,const QString & user,const QString & name) ;
    bool        isReadable   (const FtpUserID & uid,const QString & user,const QString & name) ;
    bool        isWritable   (const FtpUserID & uid,const QString & user,const QString & name) ;
    QStringList listDirs     (const FtpUserID & uid,const QString & user,const QString & name) ;
    QStringList listAll      (const FtpUserID & uid,const QString & user,const QString & name) ;

  protected:

    virtual void receive     (const QByteArray & message) ;
    QByteArray   waitForID   (const quint32 ID) ;

    template<class T>
    QByteArray remote(const QString & type, const T & arg0)
    {
      const quint32 ID = nextTransactionID++          ;
      send ( type , FtpSettings::User() , ID , arg0 ) ;
      return waitForID ( ID )                         ;
    }

    template<class T, class V>
    QByteArray remote(const QString & type, const T & arg0, const V & arg1)
    {
      const quint32 ID = nextTransactionID++                 ;
      send ( type , FtpSettings::User() , ID , arg0 , arg1 ) ;
      return waitForID ( ID )                                ;
    }

    template<class T, class V, class U>
    QByteArray remote(const QString & type,const T & arg0,const V & arg1,const U & arg2)
    {
      const quint32 ID = nextTransactionID++                        ;
      send ( type , FtpSettings::User() , ID , arg0 , arg1 , arg2 ) ;
      return waitForID ( ID )                                       ;
    }

    template<class T, class V, class U, class W>
    QByteArray remote(const QString & type,const T & arg0,const V & arg1,const U & arg2,const W & arg3)
    {
      const quint32 ID = nextTransactionID++                               ;
      send ( type , FtpSettings::User() , ID , arg0 , arg1 , arg2 , arg3 ) ;
      return waitForID ( ID )                                              ;
    }

  private:

    quint32                   nextTransactionID ;
    QHash<quint32,QByteArray> messageQueue      ;
    static FtpSharedPath *    pInstance         ;
    static const QChar        separator         ;

};

class Q_FTP_EXPORT FtpDelay : public QThread
{
  Q_OBJECT
  public:

    static void msleep (unsigned long ms) ;
    static void usleep (unsigned long us) ;
    static void sleep  (unsigned long s ) ;

};

class Q_FTP_EXPORT NetDiscovery : public TcpSharing
                                , public QRunnable
{
  Q_OBJECT
  public:

    explicit              NetDiscovery  (void) ;
    virtual              ~NetDiscovery  (void) ;

    void                  run           (void) ;
    static NetDiscovery * instance      (void) ;
    QList<FtpComputer>    getServerList (void) ;
    FtpComputer           getComputer   (const QString & name) ;
    bool                  isRunning     (void) ;

  protected:

    virtual void          receive       (const QByteArray &message) ;
    bool                  hasToStop     (void) ;

  private:

    QMutex                    mutex              ;
    bool                      bRefreshForced     ;
    bool                      bStop              ;
    bool                      bRunning           ;
    bool                      bSendNotifyRequest ;
    QMap<QString,FtpComputer> mServer            ;
    BonjourRegistrar        * registrar          ;
    static NetDiscovery     * pInstance          ;

  public slots:

    void setOrUpdate                (const QString      & data   ,
                                     const QHostAddress & host ) ;
    void terminate                  (void) ;
    void forceRefresh               (void) ;
    void remove                     (const QHostAddress & host) ;
    void restart                    (void) ;

  protected slots:

  private slots:

    void propagateServerListChanged (void) ;
    void start                      (void) ;

  signals:

    void serverListChanged          (void) ;

};

class Q_FTP_EXPORT IpFileServer : public TcpSharing
{
  public:

    explicit IpFileServer  (void) ;
    virtual ~IpFileServer  (void) ;

    static IpFileServer * instance (void) ;

    bool       seek        (const QString   & user,quint32 ID,qint64 offset) ;
    bool       isReadable  (const QString   & user,quint32 ID) ;
    bool       isWritable  (const QString   & user,quint32 ID) ;
    quint32    open        (const FtpUserID & uid ,const QString & user,const QString & filename,QFile::OpenMode mode) ;
    void       close       (const QString   & user,quint32 ID) ;
    void       flush       (const QString   & user,quint32 ID) ;
    QByteArray read        (const QString   & user,quint32 ID,qint64 maxlen) ;
    void       write       (const QString   & user,quint32 ID,const QByteArray & data) ;

  protected:

    virtual void receive   (const QByteArray & message) ;
    QByteArray   waitForID (const quint32 ID) ;

    template<class T>
    QByteArray remote(const QString & type,const T & arg0)
    {
      const quint32 ID = nextTransactionID++          ;
      send ( type , FtpSettings::User() , ID , arg0 ) ;
      return waitForID ( ID )                         ;
    }

    template<class T, class V>
    QByteArray remote(const QString & type,const T & arg0,const V & arg1)
    {
      const quint32 ID = nextTransactionID++                 ;
      send ( type , FtpSettings::User() , ID , arg0 , arg1 ) ;
      return waitForID ( ID )                                ;
    }

    template<class T, class V, class U>
    QByteArray remote ( const QString & type ,
                        const T       & arg0 ,
                        const V       & arg1 ,
                        const U       & arg2 )
    {
      const quint32 ID = nextTransactionID++                        ;
      send ( type , FtpSettings::User() , ID , arg0 , arg1 , arg2 ) ;
      return waitForID ( ID )                                       ;
    }

    template<class T, class V, class U, class W>
    QByteArray remote ( const QString & type ,
                        const T       & arg0 ,
                        const V       & arg1 ,
                        const U       & arg2 ,
                        const W       & arg3 )
    {
      const quint32 ID = nextTransactionID++                               ;
      send ( type , FtpSettings::User() , ID , arg0 , arg1 , arg2 , arg3 ) ;
      return waitForID ( ID )                                              ;
    }

  private:

    quint32                   nextTransactionID ;
    quint32                   fileID            ;
    QHash<quint32,QByteArray> messageQueue      ;
    QHash<quint32,QFile    *> files             ;
    static IpFileServer    *  pInstance         ;

};

class Q_FTP_EXPORT IpFile
{
  public:

    explicit   IpFile     (const FtpUserID & uid,const QString & filename) ;
    virtual   ~IpFile     (void) ;

    bool       exists     (void) const ;
    bool       isReadable (void) const ;
    bool       isWritable (void) const ;
    qint64     size       (void) const ;
    bool       isOpen     (void) const ;
    bool       seek       (qint64 offset) ;
    void       open       (QFile::OpenMode mode) ;
    void       close      (void) ;
    void       flush      (void) ;
    QByteArray read       (qint64 maxlen) ;
    void       write      (const QByteArray & data) ;

  protected:

    QString         fullname ;
    QString         filename ;
    QString         user     ;
    quint32         fileID   ;
    bool            bOpen    ;
    const FtpUserID uid      ;

  private:

};

class Q_FTP_EXPORT FtpSearch : public QObject
{
  Q_OBJECT
  public:

    explicit FtpSearch (const QString & computer     ,
                        const QString & request      ,
                        QObject       * parent = 0 ) ;
    virtual ~FtpSearch (void) ;

  protected:

  private:

    QString      computer ;
    QString      request  ;
    QTcpSocket * sock     ;
    QTcpSocket * dtp      ;

  public slots:

    void stopSearch    (void) ;
    void processReply  (void) ;
    void readResults   (void) ;
    void start         (void) ;
    void connected     (void) ;

  protected slots:

  private slots:

  signals:

    void found         (const QString & computer,const QString & file) ;

};

class Q_FTP_EXPORT STORcmd : public QObject
{
  Q_OBJECT
  public:

    explicit   STORcmd (const FtpUserID & uid          ,
                        const QString   & filename     ,
                        bool              append       ,
                        FtpThread       * parent = 0 ) ;
    virtual   ~STORcmd (void) ;

    FtpThread * parent (void) const ;

  protected:

   IpFile file ;
   int    k    ;
   bool   dead ;

  private:

  public slots:

    void readData      (void) ;
    void disconnected  (void) ;

  protected slots:

  private slots:

  signals:

    void finished      (void) ;
    void activity      (void) ;

};

class Q_FTP_EXPORT RETRcmd : public QObject
{
  Q_OBJECT
  public:

    explicit RETRcmd   (const FtpUserID & uid       ,
                        const QString   & filename  ,
                        FtpThread       * parent  ) ;
    virtual ~RETRcmd   (void) ;

    FtpThread * parent (void) const ;

  protected:

    IpFile file ;
    int    k    ;
    bool   dead ;

  private:

  public slots:

    void sendMore      (void) ;
    void finishing     (void) ;
    void error         (bool killDTP = true) ;

  protected slots:

  private slots:

  signals:

    void activity      (void) ;
    void finished      (void) ;

};

class Q_FTP_EXPORT LISTcmd : public QObject
{
  Q_OBJECT
  public:

    explicit LISTcmd   (const QString & path      ,
                        bool            nlstMode  ,
                        FtpThread     * parent  ) ;
    virtual ~LISTcmd   (void) ;

    FtpThread * parent (void) const ;

  protected:

  private:

    QList<QByteArray> list ;
    bool              dead ;
    int               k    ;

  public slots:

    void sendMore      (void) ;
    void finishing     (void) ;
    void error         (bool killDTP = true) ;

  protected slots:

  private slots:

  signals:

    void activity     (void) ;
    void finished     (void) ;

};

class Q_FTP_EXPORT FINDcmd : public QObject
{
  Q_OBJECT
  public:

    explicit FINDcmd   (const FtpUserID & uid      ,
                        const QString   & request  ,
                        FtpThread       * parent ) ;
    virtual ~FINDcmd   (void) ;

    FtpThread * parent (void) const ;

  protected:

    QQueue<QString> workQueue ;
    QString         req       ;
    int             k         ;
    bool            dead      ;
    const FtpUserID uid       ;
    QTimer        * timer     ;

  private:

  public slots:

    void process       (void) ;
    void finish        (void) ;

  protected slots:

  private slots:

  signals:

    void activity      (void) ;
    void finished      (void) ;

};

class Q_FTP_EXPORT FtpThread : public QObject
{
  Q_OBJECT
  friend class STORcmd ;
  friend class FINDcmd ;
  friend class RETRcmd ;
  friend class LISTcmd ;
  public:

    enum DTP_MODE { NONE, RETR, STOR, APPE, FIND } ;

    explicit FtpThread             (QTcpSocket * s, QObject * parent) ;
    virtual ~FtpThread             (void) ;

    bool     isConnected           (void) ;
    DTP_MODE getMode               (void) ;
    QString  getLogin              (void) ;
    QString  getFilename           (void) ;
    QString  getAddress            (void) ;
    bool     UTF8mode              (void) const ;

  protected:

    void     send                  (const QString & str) ;
    void     openPasv              (void) ;
    void     openEpsv              (void) ;
    QString  toStringGuessEncoding (const QByteArray & data) ;

  private:

    bool         waitingForDTP  ;
    QString      cmdParam       ;
    QTcpSocket * sock           ;
    QTcpSocket * passv          ;
    QTcpServer * serv           ;
    QFile        file           ;
    QString      login          ;
    QString      renameFrom     ;
    qint64       skip           ;
    bool         logged         ;
    bool         login_accepted ;
    bool         pass_accepted  ;
    FtpPath      path           ;
    QString      filename       ;
    DTP_MODE     mode           ;
    bool         utf8Enabled    ;
    FtpUserID    uid            ;

  public slots:

    void processCommand            (void) ;
    void connection                (void) ;
    void pasvConnection            (void) ;
    void portConnection            (void) ;
    void killDTP                   (void) ;
    void errorDTP                  (void) ;
    void start                     (void) ;
    void activate                  (void) ;
    void list                      (void) ;
    void nlst                      (void) ;

  signals:

    void activity                  (void) ;
    void DTPconnected              (void) ;

};

class Q_FTP_EXPORT FileSharing : public TcpSharing
{
  Q_OBJECT
  friend class RETRcmd ;
  friend class FINDcmd ;
  friend class STORcmd ;
  friend class LISTcmd ;
  public:

    struct ConnectionInfo {
      QString user        ;
      QString address     ;
      QString activity    ;
    }                     ;

    explicit FileSharing (void) ;
    virtual ~FileSharing (void) ;

    QList<ConnectionInfo> getInfo       (void) ;
    const QSet<QString> & getUsers      (void) const ;
    int                   nbConnections (void) ;
    bool                  checkID       (const FtpUserID & uid  ) const ;
    bool                  knownLogin    (const QString   & login) const ;
    bool                  assureUser    (QString login,QString password) ;

    static void                  startServer    (void) ;
    static void                  stopServer     (void) ;
    static void                  restartServer  (void) ;
    static bool                  serverRunning  (void) ;
    static QList<ConnectionInfo> getClientsInfo (void) ;
    static QStringList           getServerInfo  (void) ;
    static FileSharing         * instance       (void) ;

  protected:

    virtual void receive       (const QByteArray & message) ;

  private:

    QTcpServer         * sock             ;
    quint64              totalSent        ;
    quint64              totalReceived    ;
    int                  maxConnections   ;
    int                  totalConnections ;
    QSet <QString  >     users            ;
    QList<FtpUserID>     validIDs         ;
    static FileSharing * pInstance        ;

  public slots:

  protected slots:

  private slots:

    void addConnection         (void) ;
    void statsSend             (int ) ;
    void statsRecv             (int ) ;
    void killThread            (void) ;
    void startService          (void) ;
    void broadcastNewThread    (void) ;
    void broadcastRemoveThread (void) ;
    void removeClient          (void) ;

  signals:

    void newThread             (void) ;
    void removeThread          (void) ;
    void closing               (void) ;

};

/*****************************************************************************\
 *                                                                           *
 *                            File client interface                          *
 *                                                                           *
\*****************************************************************************/

class Q_FTP_EXPORT FtpCommands
{
  public:

    enum FtpState                               {
      Control = 0                               ,
      Read    = 1                               ,
      Write   = 2
    }                                           ;

    enum FtpClientOptions                       {
      BufferSize     = 8192                     ,
      ResponseSize   = 1024                     ,
      TempBufferSize = 1024                     ,
      AcceptTimeout  = 30
    }                                           ;

    enum FtpTransferMode                        {
      Passive = 1                               ,
      Active  = 2
    }                                           ;

    enum FtpTransferType                        {
      NLST =  1                                 ,
      LIST =  2                                 ,
      RETR =  3                                 ,
      STOR =  4                                 ,
      USER =  5                                 ,
      PASS =  6                                 ,
      SITE =  7                                 ,
      SysT =  8                                 ,
      MKD  =  9                                 ,
      CWD  = 10                                 ,
      CDUP = 11                                 ,
      RMD  = 12                                 ,
      Pwd  = 13                                 ,
      TYPE = 14                                 ,
      SIZE = 15                                 ,
      MDTM = 16                                 ,
      RNFR = 17                                 ,
      RNTO = 18                                 ,
      DELE = 19                                 ,
      QUIT = 20                                 ,
      PASV = 21                                 ,
      PORT = 22
    }                                           ;

    int               handle                    ;
    int               cavail                    ;
    int               cleft                     ;
    int               dir                       ;
    int               cmode                     ;
    int               CmdID                     ;
    unsigned long int xfered                    ;
    unsigned long int cbbytes                   ;
    unsigned long int xfered1                   ;
    struct timeval    idletime                  ;
    char           *  cput                      ;
    char           *  cget                      ;
    char           *  buf                       ;
    void           *  idlearg                   ;
    FtpCommands    *  ctrl                      ;
    FtpCommands    *  data                      ;
    char              response [ ResponseSize ] ;

    explicit      FtpCommands      (void) ;
    explicit      FtpCommands      (void * parent) ;
    virtual      ~FtpCommands      (void) ;

    void          setParent        (void * parent) ;
    void          setIdle          (int ms) ;
    void          setTransfer      (FtpTransferMode mode) ;
    char *        Response         (void) ;

    virtual bool  connectTo        (const char * host,int port) ;
    virtual bool  Login            (const char * user,const char * pass) ;
    virtual bool  Site             (const char * cmd) ;
    virtual bool  SYST             (char * buffer,int max) ;
    virtual bool  MakeDir          (const char * path) ;
    virtual bool  ChangeDir        (const char * path) ;
    virtual bool  CdUp             (void) ;
    virtual bool  RemoveDir        (const char * path) ;
    virtual bool  PWD              (char * path,int max) ;
    virtual bool  Size             (const char * path,quint64 & size,char mode) ;
    virtual bool  ChangedDate      (const char * path,char * dt,int max) ;
    virtual bool  Rename           (const char * src,const char * dst) ;
    virtual bool  Delete           (const char * filename) ;
    virtual bool  Quit             (void) ;

    virtual bool  Transfer         (QIODevice  & io     ,
                                    const char * path   ,
                                    int          Type   ,
                                    int          Mode ) ;
    virtual bool nLst              (QIODevice & io,const char * path) ;
    virtual bool List              (QIODevice & io,const char * path) ;
    virtual bool Get               (QIODevice & io,const char * path,char mode) ;
    virtual bool Put               (QIODevice & io,const char * path,char mode) ;

  protected:

    virtual int   ftpRead          (int fd,      char * buffer,size_t length) ;
    virtual int   ftpWrite         (int fd,const char * buffer,size_t length) ;
    virtual int   socketWait       (void) ;
    virtual int   readLine         (char * buffer,int max) ;
    virtual int   writeLine        (const char * buffer,int length) ;

    virtual int   ReadResp         (char c) ;
    virtual int   SendCMD          (const char * cmd,char expresp) ;

    FtpCommands * DataPort         (int mode,int dir) ;
    FtpCommands * Access           (const char * path,int Type,int Mode) ;
    virtual bool  AcceptConnection (void) ;
    virtual int   UdpRead          (      void * buffer,int max) ;
    virtual int   UdpWrite         (const void * buffer,int len) ;
    virtual bool  UdpClose         (void) ;

  private:

    void * Parent ;

    void          doError          (int code) ;
    void          Progress         (quint64 index) ;
    void          CallResponse     (void) ;
    int           Callback         (void) ;
    int           CloseHandle      (void) ;

};

class Q_FTP_EXPORT FtpClient : public QObject
{
  Q_OBJECT
  public:

    int         ProgressIndex ;
    int         UserAnswer    ;
    QStringList Loggings      ;
    QMutex      LogMutex      ;

    explicit           FtpClient       (QObject * parent = NULL) ;
    virtual           ~FtpClient       (void) ;

    virtual QByteArray Response        (void) ;
    virtual int        Callback        (FtpCommands * commands) ;
    virtual void       Error           (int code,FtpCommands * commands) ;
    virtual void       Sending         (QString cmd,FtpCommands * commands) ;
    virtual void       doProgress      (quint64 index,FtpCommands * commands) ;
    virtual void       acceptResponse  (FtpCommands * commands) ;

    virtual QString    SystemType      (void) ;
    virtual QString    PWD             (void) ;

  protected:

    FtpCommands * commands ;
    QMutex        mutex    ;

  private:

    void FixupDateTime                 (QDateTime         & dateTime ) ;
    bool ParseUnixDir                  (const QStringList & tokens     ,
                                        const QString     & userName   ,
                                        QUrlInfo          & info     ) ;
    bool ParseDosDir                   (const QStringList & tokens     ,
                                        const QString     & userName   ,
                                        QUrlInfo          & info     ) ;
    bool ParseDir                      (const QString     & buffer     ,
                                        const QString     & userName   ,
                                        QUrlInfo          & info     ) ;

  public slots:

    virtual bool       connectFtp      (QString hostname,int port = 21) ;
    virtual bool       Login           (QString username,QString password) ;
    virtual bool       Quit            (void) ;
    virtual bool       exists          (void) ;

    virtual bool       Site            (QString command) ;
    virtual bool       MakeDirectory   (QString path) ;
    virtual bool       ChangeDirectory (QString path) ;
    virtual bool       CdUp            (void) ;
    virtual bool       RemoveDirectory (QString path) ;
    virtual bool       Filesize        (QString path,quint64 & size) ;
    virtual bool       LastModified    (QString path,QDateTime & modification) ;
    virtual bool       Rename          (QString source,QString target) ;
    virtual bool       Delete          (QString filename) ;
    virtual bool       NLST            (QIODevice & io,QString path) ;
    virtual bool       List            (QIODevice & io,QString path) ;
    virtual bool       List            (QString path,QList<QUrlInfo> & files,QString username) ;
    virtual bool       Get             (QIODevice & io,QString path) ;
    virtual bool       Put             (QIODevice & io,QString path) ;

  protected slots:

  private slots:

    void acceptMessage                 (QString message) ;

  signals:

    void DataProgress                  (quint64 index) ;
    void PostMessage                   (QString message) ;
    void Message                       (QString message) ;

};

}

inline bool operator!=(const N::FtpComputer & a,const N::FtpComputer & b)
{
  return a . getHostname ( ) != b . getHostname ( ) ||
         a . getName     ( ) != b . getName     ( ) ||
         a . getPort     ( ) != b . getPort     ( )  ;
}

inline QDataStream & operator << (QDataStream & out,const N::FtpComputer & in)
{
  return out << in.serialize() ;
}

inline QDataStream & operator >> (QDataStream & in,N::FtpComputer & out)
{
  QByteArray data            ;
  in >> data                 ;
  out . unserialize ( data ) ;
  return in                  ;
}

inline QDataStream & operator << (QDataStream & out,const N::FtpUserID & id)
{
  return out << id.getLogin() << id.getPassword() ;
}

inline QDataStream & operator >> (QDataStream & out,N::FtpUserID & id)
{
  QString    login                       ;
  QByteArray password                    ;
  out >> login >> password               ;
  id = N::FtpUserID ( login , password ) ;
  return out                             ;
}

class Q_FTP_EXPORT QUrlInfoPrivate
{
public:
    QUrlInfoPrivate() :
        permissions(0),
        size(0),
        isDir(false),
        isFile(true),
        isSymLink(false),
        isWritable(true),
        isReadable(true),
        isExecutable(false)
    {}

    QString name;
    int permissions;
    QString owner;
    QString group;
    qint64 size;

    QDateTime lastModified;
    QDateTime lastRead;
    bool isDir;
    bool isFile;
    bool isSymLink;
    bool isWritable;
    bool isReadable;
    bool isExecutable;
};

class Q_FTP_EXPORT QUrlInfo
{
public:
    enum PermissionSpec {
        ReadOwner = 00400, WriteOwner = 00200, ExeOwner = 00100,
        ReadGroup = 00040, WriteGroup = 00020, ExeGroup = 00010,
        ReadOther = 00004, WriteOther = 00002, ExeOther = 00001 };

    QUrlInfo();
    QUrlInfo(const QUrlInfo &ui);
    QUrlInfo(const QString &name, int permissions, const QString &owner,
             const QString &group, qint64 size, const QDateTime &lastModified,
             const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
             bool isWritable, bool isReadable, bool isExecutable);
    QUrlInfo(const QUrl &url, int permissions, const QString &owner,
             const QString &group, qint64 size, const QDateTime &lastModified,
             const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
             bool isWritable, bool isReadable, bool isExecutable);
    QUrlInfo &operator=(const QUrlInfo &ui);
    virtual ~QUrlInfo();

    virtual void setName(const QString &name);
    virtual void setDir(bool b);
    virtual void setFile(bool b);
    virtual void setSymLink(bool b);
    virtual void setOwner(const QString &s);
    virtual void setGroup(const QString &s);
    virtual void setSize(qint64 size);
    virtual void setWritable(bool b);
    virtual void setReadable(bool b);
    virtual void setPermissions(int p);
    virtual void setLastModified(const QDateTime &dt);
    void setLastRead(const QDateTime &dt);

    bool isValid() const;

    QString name() const;
    int permissions() const;
    QString owner() const;
    QString group() const;
    qint64 size() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;
    bool isDir() const;
    bool isFile() const;
    bool isSymLink() const;
    bool isWritable() const;
    bool isReadable() const;
    bool isExecutable() const;

    static bool greaterThan(const QUrlInfo &i1, const QUrlInfo &i2,
                             int sortBy);
    static bool lessThan(const QUrlInfo &i1, const QUrlInfo &i2,
                          int sortBy);
    static bool equal(const QUrlInfo &i1, const QUrlInfo &i2,
                       int sortBy);

    bool operator==(const QUrlInfo &i) const;
    inline bool operator!=(const QUrlInfo &i) const
    { return !operator==(i); }

private:
    QUrlInfoPrivate *d;
};

/*
    The QFtpDTP (DTP = Data Transfer Process) controls all client side
    data transfer between the client and server.
*/
class Q_FTP_EXPORT QFtpDTP : public QObject
{
    Q_OBJECT

public:
    enum ConnectState {
        CsHostFound,
        CsConnected,
        CsClosed,
        CsHostNotFound,
        CsConnectionRefused
    };

    QFtpDTP(QFtpPI *p, QObject *parent = 0);

    void setData(QByteArray *);
    void setDevice(QIODevice *);
    void writeData();
    void setBytesTotal(qint64 bytes);

    bool hasError() const;
    QString errorMessage() const;
    void clearError();

    void connectToHost(const QString & host, quint16 port);
    int setupListener(const QHostAddress &address);
    void waitForConnection();

    QTcpSocket::SocketState state() const;
    qint64 bytesAvailable() const;
    qint64 read(char *data, qint64 maxlen);
    QByteArray readAll();

    void abortConnection();

    static bool parseDir(const QByteArray &buffer, const QString &userName, QUrlInfo *info);

signals:
    void listInfo(const QUrlInfo&);
    void readyRead();
    void dataTransferProgress(qint64, qint64);

    void connectState(int);

private slots:
    void socketConnected();
    void socketReadyRead();
    void socketError(QAbstractSocket::SocketError);
    void socketConnectionClosed();
    void socketBytesWritten(qint64);
    void setupSocket();

    void dataReadyRead();

private:
    void clearData();

    QTcpSocket *socket;
    QTcpServer listener;

    QFtpPI *pi;
    QString err;
    qint64 bytesDone;
    qint64 bytesTotal;
    bool callWriteData;

    // If is_ba is true, ba is used; ba is never 0.
    // Otherwise dev is used; dev can be 0 or not.
    union {
        QByteArray *ba;
        QIODevice *dev;
    } data;
    bool is_ba;

    QByteArray bytesFromSocket;
};

/**********************************************************************
 *
 * QFtpPI - Protocol Interpreter
 *
 *********************************************************************/

class Q_FTP_EXPORT QFtpPI : public QObject
{
    Q_OBJECT

public:
    QFtpPI(QObject *parent = 0);

    void connectToHost(const QString &host, quint16 port);

    bool sendCommands(const QStringList &cmds);
    bool sendCommand(const QString &cmd)
        { return sendCommands(QStringList(cmd)); }

    void clearPendingCommands();
    void abort();

    QString currentCommand() const
        { return currentCmd; }

    bool rawCommand;
    bool transferConnectionExtended;

    QFtpDTP dtp; // the PI has a DTP which is not the design of RFC 959, but it
                 // makes the design simpler this way
signals:
    void connectState(int);
    void finished(const QString&);
    void error(int, const QString&);
    void rawFtpReply(int, const QString&);

private slots:
    void hostFound();
    void connected();
    void connectionClosed();
    void delayedCloseFinished();
    void readyRead();
    void error(QAbstractSocket::SocketError);

    void dtpConnectState(int);

private:
    // the states are modelled after the generalized state diagram of RFC 959,
    // page 58
    enum State {
        Begin,
        Idle,
        Waiting,
        Success,
        Failure
    };

    enum AbortState {
        None,
        AbortStarted,
        WaitForAbortToFinish
    };

    bool processReply();
    bool startNextCmd();

    QTcpSocket commandSocket;
    QString replyText;
    char replyCode[3];
    State state;
    AbortState abortState;
    QStringList pendingCommands;
    QString currentCmd;

    bool waitForDtpToConnect;
    bool waitForDtpToClose;

    QByteArray bytesFromSocket;

    friend class QFtpDTP;
};

class Q_FTP_EXPORT QFtp : public QObject
{
    Q_OBJECT

public:
    explicit QFtp(QObject *parent = 0);
    virtual ~QFtp();

    enum State {
        Unconnected,
        HostLookup,
        Connecting,
        Connected,
        LoggedIn,
        Closing
    };
    enum Error {
        NoError,
        UnknownError,
        HostNotFound,
        ConnectionRefused,
        NotConnected
    };
    enum Command {
        None,
        SetTransferMode,
        SetProxy,
        ConnectToHost,
        Login,
        Close,
        List,
        Cd,
        Get,
        Put,
        Remove,
        Mkdir,
        Rmdir,
        Rename,
        RawCommand
    };
    enum TransferMode {
        Active,
        Passive
    };
    enum TransferType {
        Binary,
        Ascii
    };

    int setProxy(const QString &host, quint16 port);
    int connectToHost(const QString &host, quint16 port=21);
    int login(const QString &user = QString(), const QString &password = QString());
    int close();
    int setTransferMode(TransferMode mode);
    int list(const QString &dir = QString());
    int cd(const QString &dir);
    int get(const QString &file, QIODevice *dev=0, TransferType type = Binary);
    int put(const QByteArray &data, const QString &file, TransferType type = Binary);
    int put(QIODevice *dev, const QString &file, TransferType type = Binary);
    int remove(const QString &file);
    int mkdir(const QString &dir);
    int rmdir(const QString &dir);
    int rename(const QString &oldname, const QString &newname);

    int rawCommand(const QString &command);

    qint64 bytesAvailable() const;
    qint64 read(char *data, qint64 maxlen);
    QByteArray readAll();

    int currentId() const;
    QIODevice* currentDevice() const;
    Command currentCommand() const;
    bool hasPendingCommands() const;
    void clearPendingCommands();

    State state() const;

    Error error() const;
    QString errorString() const;

public Q_SLOTS:
    void abort();

Q_SIGNALS:
    void stateChanged(int);
    void listInfo(const QUrlInfo&);
    void readyRead();
    void dataTransferProgress(qint64, qint64);
    void rawCommandReply(int, const QString&);

    void commandStarted(int);
    void commandFinished(int, bool);
    void done(bool);

private:
    Q_DISABLE_COPY(QFtp)
    QScopedPointer<QFtpPrivate> d;

    Q_PRIVATE_SLOT(d, void _q_startNextCommand())
    Q_PRIVATE_SLOT(d, void _q_piFinished(const QString&))
    Q_PRIVATE_SLOT(d, void _q_piError(int, const QString&))
    Q_PRIVATE_SLOT(d, void _q_piConnectState(int))
    Q_PRIVATE_SLOT(d, void _q_piFtpReply(int, const QString&))
};

/**********************************************************************
 *
 * QFtpCommand implemenatation
 *
 *********************************************************************/

class Q_FTP_EXPORT QFtpCommand
{
public:
    QFtpCommand(QFtp::Command cmd, QStringList raw, const QByteArray &ba);
    QFtpCommand(QFtp::Command cmd, QStringList raw, QIODevice *dev = 0);
    ~QFtpCommand();

    int id;
    QFtp::Command command;
    QStringList rawCmds;

    // If is_ba is true, ba is used; ba is never 0.
    // Otherwise dev is used; dev can be 0 or not.
    union {
        QByteArray *ba;
        QIODevice *dev;
    } data;
    bool is_ba;

    static QBasicAtomicInt idCounter;
};

/**********************************************************************
 *
 * QFtpPrivate
 *
 *********************************************************************/
class Q_FTP_EXPORT QFtpPrivate
{
    Q_DECLARE_PUBLIC(QFtp)
public:

    inline QFtpPrivate(QFtp *owner) : close_waitForStateChange(false), state(QFtp::Unconnected),
        transferMode(QFtp::Passive), error(QFtp::NoError), q_ptr(owner)
    { }

    ~QFtpPrivate() { while (!pending.isEmpty()) delete pending.takeFirst(); }

    // private slots
    void _q_startNextCommand();
    void _q_piFinished(const QString&);
    void _q_piError(int, const QString&);
    void _q_piConnectState(int);
    void _q_piFtpReply(int, const QString&);

    int addCommand(QFtpCommand *cmd);

    QFtpPI pi;
    QList<QFtpCommand *> pending;
    bool close_waitForStateChange;
    QFtp::State state;
    QFtp::TransferMode transferMode;
    QFtp::Error error;
    QString errorString;

    QString host;
    quint16 port;
    QString proxyHost;
    quint16 proxyPort;
    QFtp *q_ptr;
};

Q_DECLARE_METATYPE(QUrlInfo)
Q_DECLARE_METATYPE(N::Ftp)
Q_DECLARE_METATYPE(N::FtpProtocol)
Q_DECLARE_METATYPE(N::FtpOptions)
Q_DECLARE_METATYPE(N::FtpEntry)
Q_DECLARE_METATYPE(N::FtpUser)
Q_DECLARE_METATYPE(N::FtpServer)
Q_DECLARE_METATYPE(N::FtpUserID)
Q_DECLARE_METATYPE(N::FtpComputer)

QT_END_NAMESPACE

#endif
