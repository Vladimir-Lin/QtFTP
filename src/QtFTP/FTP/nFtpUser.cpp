#include <qtftp.h>

#if defined(Q_OS_WIN)
//#define WIN32_LEAN_AND_MEAN
//#include <io.h>
//#include <direct.h>
//#include <winsock2.h>
//#include <process.h>
#else
#include <limits.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#endif

N::FtpUser:: FtpUser (void   )
           : Thread  (0,false)
{
}

N::FtpUser::~FtpUser (void)
{
}

void N::FtpUser::run(void)
{
}
