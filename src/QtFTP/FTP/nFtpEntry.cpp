#include <qtftp.h>

#if defined(Q_OS_WIN)
#ifdef IPV6_SUPPORTS
#define WIN32_LEAN_AND_MEAN
#include <io.h>
#include <direct.h>
#include <winsock2.h>
#include <process.h>
#include <ws2tcpip.h>
#endif
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

N::FtpEntry:: FtpEntry (void   )
            : Thread   (0,false)
{
}

N::FtpEntry::~FtpEntry (void)
{
}

void N::FtpEntry::run(void)
{
}
