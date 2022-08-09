#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define VERSION    23
#define BUFSIZE  8096
#define ERROR      42
#define LOG        44
#define FORBIDDEN 403
#define NOTFOUND  404
#define GET    1
#define PUT    2
#define PWD    3
#define DIR    4
#define CD     5
#define QUIT   6
#define HELP   7
#define RENAME 8
#define DELETE    9
#define MKDIR    10

char *getdir(char *cmd)
{
    char *p;
    p = strtok(cmd," ");
    p = strtok(NULL," ");
    return p;
}

int get_cmd(char *cmd)
{
	if(!strcmp("quit",cmd))        return QUIT;
    if(!strcmp("pwd",cmd))         return PWD;
	if(!strcmp("dir",cmd))         return DIR;
	if(!strcmp("help",cmd))        return HELP;
    if(strstr(cmd,"cd")!=NULL)     return CD;
    if(strstr(cmd,"get")!=NULL)    return GET;
    if(strstr(cmd,"put")!=NULL)    return PUT;
	if(strstr(cmd,"rename")!=NULL) return RENAME;
	if(strstr(cmd,"lcd")!=NULL)    return DELETE;
	if(strstr(cmd,"mkdir")!=NULL)  return MKDIR;
    return 100;
}

void help() 
{ //帮助菜单
	cout << "        ___________________________________________  " << endl
		 << "       |                FTP帮助菜单                |   " << endl
		 << "       | 1、get 下载文件 [输入格式: get 文件名 ]    |   " << endl
		 << "       | 2、put 上传文件 [输入格式：put 文件名]     |   " << endl
		 << "       | 3、pwd 显示当前文件夹的绝对路径            |   " << endl
		 << "       | 4、dir 显示远方当前目录的文件              |   " << endl
		 << "       | 5、cd  改变远方当前目录和路径              |   " << endl
		 << "       |         进入下级目录: cd 路径名           |   " << endl
		 << "       |         进入上级目录: cd ..               |   " << endl
		 << "       | 6、? 或者 help 进入帮助菜单               |   " << endl
		 << "       | 7、quit 退出FTP                          |   " << endl
		 <<"        | 8、delete       删除文件                 |   "<<endl
		 <<"        | 9、mkdir       在远程主机建目录          |    "<<endl
		 <<"        |__________________________________________|    " << endl;
}

void logger(int type, char* s1, char* s2, int socket_fd)
{
	int fd;
	char logbuffer[BUFSIZE * 2];

	// 获取时间信息
	struct tm* ptr;
	time_t lt;
	lt = time(NULL);
	ptr = localtime(&lt);
	char timebuf[48] = { 0 };
	strftime(timebuf, sizeof(timebuf), "%H:%M:%S %a %d/%m/%Y", ptr);

	/*将消息写入logger，或直接将消息通过socket通道返回给客户端*/
	switch (type) {
	case ERROR: (void)sprintf(logbuffer, "[%s] ERROR: %s:%s Errno=%d exiting pid=%d\n", timebuf, s1, s2, errno, getpid());
		break;
	case FORBIDDEN:
		(void)write(socket_fd, "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n", 271);
		(void)sprintf(logbuffer, "[%s] FORBIDDEN: %s:%s:%d\n", timebuf, s1, s2, socket_fd);
		break;
	case NOTFOUND:
		(void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n", 224);
		(void)sprintf(logbuffer, "[%s] NOT FOUND: %s:%s:%d\n", timebuf, s1, s2, socket_fd);
		break;

	case LOG: (void)sprintf(logbuffer, "[%s] INFO: %s:%s:%d", timebuf, s1, s2, socket_fd); break;
	}
	/* No checks here, nothing can be done with a failure anyway */
	if ((fd = open("mylog.log", O_CREAT | O_WRONLY | O_APPEND, 0644)) >= 0) {
		(void)write(fd, logbuffer, strlen(logbuffer));
		(void)write(fd, "\n", 1);
		(void)close(fd);
	}
}