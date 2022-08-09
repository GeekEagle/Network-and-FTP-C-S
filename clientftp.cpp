#include "config.h"
#include "password.h"

int data_connect_pasv(int port,char *ip)
{
    int datasockfd;
    struct sockaddr_in cli_addr;  
    if((datasockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        logger(ERROR, "system call", "socket", 0);
     printf("port:%d\n",port);
    /*if (port < 0 || port >60000)
		logger(ERROR, "Invalid port number (try 1->60000)", argv[1], 0);*/
    memset(&cli_addr, 0, sizeof(cli_addr));  
    cli_addr.sin_family = AF_INET;  
    cli_addr.sin_port = htons(port);  
    inet_aton(ip,&cli_addr.sin_addr);
    logger(LOG,"PASV child process,port is",ip,port);
     printf("waiting child process connect\n");
    if( connect(datasockfd, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) < 0){
        logger(ERROR,"client call","connect",0);
        cout<<"connect failed"<<endl;
    }
    cout<<"pasv here "<<port<<endl;
    return datasockfd;
}

int data_connect_port(int port,char *ip)
{
    static struct sockaddr_in cli_addr;
    int datalistenfd;
	int connect_fd;
	if((datalistenfd = socket(AF_INET,SOCK_STREAM,0))<0)
        logger(ERROR, "system call", "socket", 0);
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(port);
    logger(LOG, "nweb port starting", "127.0.0.1 pid:", getpid());
    if(bind(datalistenfd,(struct sockaddr*)&cli_addr,sizeof(cli_addr))<0)
        logger(ERROR,"system call","bind",0);
    if(listen(datalistenfd,64)<0)
       logger(ERROR,"system call","listen",0);
    return datalistenfd;
}

int data_accept(int datalistenfd)
{
    int connect_fd;
    struct sockaddr_in serv_addr;
    cout<<"port here"<<endl;
    socklen_t len = sizeof(serv_addr);
    printf("waiting service connect\n");
    connect_fd = accept(datalistenfd,(struct sockaddr*)&serv_addr,&len);
    if(connect_fd<=0){
        logger(ERROR, "system call", "accept", 0);	  
        printf("sysytem call accept error\n");
    }
	return connect_fd;
}

void put(char *msg , int fd,char *conway,char *ip,int port)
{
    char *dir;
    char buf[1024],info[100];
    int ret,data_port,listen;
    int filefd;
    cout<<"wait for sending..."<<endl;
    dir = getdir(msg);
    if(access(dir,F_OK) == -1){
        printf("%s not exsit\n",dir);
        send(fd,"file not exist",20,0);
        return;
    }
    send(fd,"file exist",20,0);
    
    if(!strncmp(conway,"port",4)) {
        listen = data_connect_port(port,ip);
        data_port = data_accept(listen);
        if(listen<0){
            printf("error create:\n");
            ret = 4096;
            send(fd,&ret,sizeof(int),0);
		    return;
        }
        ret = 2048;                          /*正确的连接建立*/
        send(fd,&ret,sizeof(int),0);
        recv(fd,&ret,sizeof(int),0);
        cout<<ret<<endl;
        if(ret!= 125) return;
    } 
    
    else if(!strcmp(conway,"pasv")){ 
        sleep(1);
        data_port = data_connect_pasv(port,ip);
        recv(fd,&ret,sizeof(ret),0);
        cout<<ret<<endl;
        if(ret==2048){
            if(data_port<0){
                printf("error connect:\n");
                ret = 425;
                send(fd,&ret,sizeof(int),0);
		        return;
            }
        }
        else {
            printf("server error\n");
            return;
        }
        ret = 125;
		send(fd,&ret,sizeof(int),0);
    }

    cout<<"file exist"<<endl;
    filefd = open(dir,O_RDWR);
    while((ret = read(filefd,buf,1024))>=1)
        send(data_port,buf,ret,0);
    close(filefd);
    if(!strncmp(conway,"port",4)) close(listen);
    close(data_port);
    
}

void cmd_handler(char *msg , int fd,char *conway,char *ip,int port)
{
    char *dir,dataway[10];
    char buf[1024],info[100];
    int ret,data_port,listen;
    int filefd,pidstatus=0;
    pid_t pid;
    if (strstr(msg,"put")!=NULL){
        send(fd,msg,256,0);
        put(msg,fd,conway,ip,port);
    }
    else if(!strcmp(msg,"quit")){
        strcpy(msg,"quit");
        send(fd,msg,sizeof(msg),0);
        close(fd);
        exit(-1);
    } 
    else send(fd,msg,256,0);
}

void get(int c_fd ,char *msg,int port,char *ip,char *conway)
{
    int  n_read,ret;
    char msgget[1024],chuan[100];
    int newfilefd,data_port,pidstatus=0,listen;
    char *p = getdir(msg);
    recv(c_fd,msgget,sizeof(msg),0);
    cout<<msgget<<endl;
    if(atoi(msgget) == NOTFOUND){
        printf("no this file\n");
        return;
    }
    
    if(!strncmp(conway,"port",4)){
        listen = data_connect_port(port,ip);
        data_port = data_accept(listen);
        if(data_port<0){
            perror("error connect:");
		    exit(1);
        }
    }
    else {
        data_port = data_connect_pasv(port,ip);
        if(data_port<0){
            perror("error connect:");
            exit(1);
        }
    }
    newfilefd = open(p,O_RDWR|O_CREAT,0600);
    while(1){
        n_read = recv(data_port,msgget,sizeof(msgget),0);
        write(newfilefd,msgget,n_read);
        if(n_read<1024) break;
    }
    recv(c_fd,chuan,sizeof(chuan),0);
    cout<<chuan<<endl;
    close(newfilefd);
    if(!strncmp(conway,"port",4)) close(listen);
    close(data_port); 
}

void handler_server_message(int c_fd ,char *msg,int port,char *ip,char *conway)
{
    int  n_read,ret;
    char msgget[1024];
    int newfilefd,data_port,pidstatus=0,listen;
    pid_t pid;
    if(strncmp(msg, "get", 3) == 0)
        get(c_fd,msg,port,ip,conway);
    else if (strncmp(msg,"help",4)==0)
        help();      
    else {
        recv(c_fd,msgget,1024,0);
        printf("%s\n",msgget);
    }
    putchar('>');
}

void login(int sockfd)
{
    char username[75],respond[75],password[75],qwq[25];
    char *new_pass,*p,*pas;
    int i,sign1;
    cin.get();
    for(i=0;i<5;i++){
        cout<<"请输入用户名 user [username]"<<endl;
        cin.get(username,sizeof(username)).get();
        send(sockfd,username,sizeof(username),0);
        recv(sockfd,respond,sizeof(respond),0);
        cout<<respond<<endl;
        if(!strcmp(respond,"no this command")) continue;
        cin.get(password,sizeof(password)).get();
        new_pass = encrypt(password);
        cout<<new_pass<<endl;
        send(sockfd,new_pass,sizeof(password),0);
        recv(sockfd,respond,sizeof(respond),0);
         if(!strcmp(respond,"no this command")) continue;
        if(!strcmp(respond,"right")){
            cout<<"登录成功,欢迎你，"<<username<<endl;
            return;
        }
        else cout<<"密码错误"<<endl;
    }
    if(i==5){
        cout<<"错误次数达上限"<<endl;
        exit(1);
    }
}

int main(int argc, char** argv)  
{  
    int    sockfd,port,mark;
    char msg[256],connway[10],ip[20],duankou[10],temport[6];
    char ftp[6];
    char *p;
    struct sockaddr_in cli_addr;  
    while(1){
        cout<<"请输入连接的IP和端口:[ftp ip port]";
        cin>>ftp>>ip>>duankou;
        if(strcmp(ftp,"ftp")!=0)
            cout<<"命令错误，重新输入"<<endl;
        else break;
    }
    port = atoi(duankou);
    (void)signal(SIGCLD, SIG_IGN); // ignore child death 
    (void)signal(SIGHUP, SIG_IGN); // ignore terminal hangups 
    logger(LOG, "nweb starting", ip, getpid());
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        logger(ERROR, "system call", "socket", 0);
    memset(&cli_addr, 0, sizeof(cli_addr));  
    cli_addr.sin_family = AF_INET;  
    cli_addr.sin_port = htons(port);  
    inet_aton(ip,&cli_addr.sin_addr);
    
    if( connect(sockfd, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) < 0)
        logger(ERROR,"client call","connect",0);
    login(sockfd);
    printf("send message to server: \n"); 
    cout<<"请选择连接方式port host|pasv"<<endl;
    while(1){
        cin.get(connway,sizeof(connway)).get();
        if(strncmp(connway,"port",4)&&strcmp(connway,"pasv"))
            cout<<"输入错误，请重新输入"<<endl;
        else break;
    }
    send(sockfd,connway,sizeof(connway),0);
    if(!strncmp(connway,"port",4)){
        strcpy(temport,"1030");
        send(sockfd,temport,sizeof(temport),0);
    }
    else if(!strcmp(connway,"pasv"))
		recv(sockfd,temport,sizeof(temport),0);
    port = atoi(temport);
    mark = 0;
    while(1){
        memset(msg,'\0',sizeof(msg));
        printf("please input command>");
        cin.get(msg,100).get();
        cmd_handler(msg,sockfd,connway,ip,port);  //发送命令到服务器
        //fflush(stdout);*/
        handler_server_message(sockfd,msg,port,ip,connway);
        port = port+2;
    }
    return 0;
}  