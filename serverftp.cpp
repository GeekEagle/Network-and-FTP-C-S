#include "config.h"
#include "password.h"

char *password = "123456";
char *ip = "127.0.0.1";


int data_connect_pasv(int port)
{
	struct sockaddr_in serv_addr;
	int datalistenfd;
	if((datalistenfd = socket(AF_INET,SOCK_STREAM,0))<0)
        logger(ERROR, "system call", "socket", 0);
	//port = temport;
	printf("port:%d\n",port);
	logger(LOG,"PASV service process,port is","socket",port);
    /*if (port < 0 || port >60000)
		logger(ERROR, "Invalid port number (try 1->60000)", argv[1], 0);*/
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //inet_aton(argv[1],&serv_addr.sin_addr);
    serv_addr.sin_port = htons(port);
    if(bind(datalistenfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
        logger(ERROR,"system call","bind",0);
    if(listen(datalistenfd,64)<0)
       logger(ERROR,"system call","listen",0);
	return datalistenfd;
}

int data_accept(int datalistenfd){
	struct sockaddr_in cli_addr;
	int connect_fd;
    socklen_t len = sizeof(cli_addr);
     printf("waiting child process connecting\n");
    connect_fd = accept(datalistenfd,(struct sockaddr*)&cli_addr,&len);
    printf("qwqqwq\n");
    if(connect_fd<=0){
        logger(ERROR, "system call", "accept", 0);
		printf("sysytem call accept error\n");
    }
    printf("waiting child process connect\n");
	return connect_fd;
}

int data_connect_port(int cliport)
{
    struct sockaddr_in serv_addr;  
	int datasockfd;
    if((datasockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        logger(ERROR, "system call", "socket", 0);
    //printf("hhahhahah");
    int port = cliport;
    memset(&serv_addr, 0, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_port = htons(port);  
    inet_aton("127.0.0.1",&serv_addr.sin_addr);
	logger(LOG, "PORT SERV PROCESS", "socket", 0);
    /*if(bind(sockfd,(struct sockaddr*)&cli_addr,sizeof(struct sockaddr_in)) < 0)
        logger(ERROR,"client call","bind",0);*/
	printf("connect to client by child process\n");
	printf("port here %d\n",port);
    if( connect(datasockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        logger(ERROR,"client call port ","connect",0);
		printf("connect failed\n");
    }
    return datasockfd;
}

void put(char *msg,int fd,char *conway,int port)
{
	char dataBuf[1024],chuan[100];
	char *file;
	char temp[1024],info1[100],info2[100];
	int fdfile,n_rec,data_port,listen;
	int ret;
	if(!strncmp(conway,"port",4)) {
	       sleep(2);
	       data_port = data_connect_port(port);
		recv(fd,&ret,sizeof(int),0);
		printf("%d\n",ret);
		if(ret==4096){
			printf("create failed\n");
			return;
		}
		else if(ret==2048){
			
		    if(data_port<0){
                printf("error connect:\n");
				ret = 425;
			    send(fd,&ret,sizeof(int),0);
		        return;
            }
			ret = 125;
			send(fd,&ret,sizeof(int),0);
		}
	}
    
	else{  
		listen = data_connect_pasv(port);
		data_port = data_accept(listen);
		if(listen<0){
            printf("error create:\n");
			ret = 425;
			send(fd,&ret,sizeof(int),0);
	        return;
        }
		ret = 2048;                          /*正确的连接建立*/
        send(fd,&ret,sizeof(int),0);
        printf("%d\n",ret);
        recv(fd,&ret,sizeof(int),0);
        printf("%d\n",ret);
        if(ret!= 125) return;
	}

	file = getdir(msg);
	fdfile = open(file,O_RDWR|O_CREAT,0666);
	while((n_rec = recv(data_port,dataBuf,1024,0))>=1024)
		write(fdfile,dataBuf,n_rec);
	write(fdfile,dataBuf,n_rec);
	close(fdfile);    
	strcpy(chuan,"426 load to server successfully.");
	send(fd,chuan,sizeof(chuan),0);
	printf("load to server successfully.\n");
	close(data_port);
	if(!strcmp(conway,"pasv")) close(listen);
}

void get(char *msg,int fd,char *conway,int port)
{
	char dataBuf[1024],chuan[100];
	char *file;
	char temp[1024];
	int fdfile,n_rec,data_port,listen;
	int ret;
    file = getdir(msg);
	printf("%s\n",file);
	if(access(file,F_OK)==-1){
		strcpy(msg,"404");
		send(fd,msg,sizeof(msg),0);
		return;
	}
	else{
	       strcpy(temp,"hello");
		send(fd,temp,sizeof(msg),0);
		if(!strncmp(conway,"port",4)){
		sleep(1);	
            data_port = data_connect_port(port);
			if(data_port<0){
                perror("error connect:");
		        exit(1);
            }
		}
        else{
			listen = data_connect_pasv(port);
			data_port = data_accept(listen);
			if(data_port<0){
                perror("error connect:");
		        exit(1);
            }
		}
		fdfile = open(file,O_RDWR);
		while((ret = read(fdfile,dataBuf,1024))>=1)
			send(data_port,dataBuf,ret,0);
		close(fdfile);	    
		strcpy(chuan,"426 get from server successfully.");
		send(fd,chuan,sizeof(chuan),0);
		printf("get from server successfully.\n");
		close(data_port);
		if(!strcmp(conway,"pasv")) close(listen);
	}
}


void msg_handler(char *msg,int fd,char *conway,int port)
{
	char chuan[1024];
	char *file,*orgname,*newname;
	char temp[1024];
	int fdfile,n_rec,data_port,pidstatus=0,listen;
	int ret = get_cmd(msg);
	pid_t pid;
	struct dirent *mydir;
	struct dirent *myitem;
	switch (ret)
	{
	    case PWD: 
		    file = getcwd(NULL,0);
		    strcpy(temp,file);
		    send(fd,temp,sizeof(temp),0);
		    break;
		case CD:
			file = getdir(msg);
			//printf("%s\n",file);
			ret = chdir(file);  /*切换到当前目录下*/
			if(ret==-1){
				printf("failed cd\n");
				//memset(temp,'\0',sizeof(temp),0);
				strcpy(temp,"failed cd");
				send(fd,temp,sizeof(temp),0);
			}
			else if(!ret){
				printf("change successfully\n");
			    strcpy(temp,"change successfully");
				send(fd,temp,sizeof(temp),0);
			}
			break;
	    case GET:
            //memset(file,'0',sizeof(file));
			get(msg,fd,conway,port);
			break;
		case PUT:
		    memset(temp,0,sizeof(temp));
		    recv(fd,temp,20,0);
		    printf("%s",temp);
			if(!strcmp(temp,"file not exist")) 
			    return;
			put(msg,fd,conway,port);
		    break;
		case QUIT:
		    printf("client quit\n");
			//exit(1);
			break;		
		case DIR:
		    file = getcwd(NULL,0);
			memset(temp,0,sizeof(temp));
			if((mydir = opendir(file))!=NULL){
				while((myitem = readdir(mydir))!=NULL){
				    strcat(temp,myitem->d_name);
					strcat(temp," ");
				}
			    send(fd,temp,sizeof(temp),0);
			}
			break;
		case DELETE:                              /*删除文件*/
		    file = getdir(msg);
			if(!access(file,F_OK)){
			      ret = remove(file);
			    if(!ret) strcpy(chuan,"delete file successfully.");
			    else strcpy(chuan,"failed to create file.");
			}
			else strcpy(chuan,"file not exist.");
			send(fd,chuan,sizeof(chuan),0);
		    break;
		case MKDIR:                               /*在远程主机建目录*/
		    file = getdir(msg);
			ret = mkdir(file,0755);
			if(!ret)
				strcpy(chuan,"create file successfully.");
			else strcpy(chuan,"failed to create file.");
			send(fd,chuan,sizeof(chuan),0);
		    break;
	    default:
	        strcpy(chuan, "no this command\0");
		    send(fd,chuan,sizeof(chuan),0);
		    break;
	}
}

int verify(int fd)
{
    int sign1,sign2;
    char recuser[75],recpass[75],right[6],wrong[6];
	char org_pass[50],*p;
    strcpy(right,"right\0");
    strcpy(wrong,"wrong\0");
	while(1){
	    sign1 = recv(fd,recuser,sizeof(recuser),0);
		if(strncmp(recuser,"user",4)){
	        send(fd,"no this command\0",sizeof(recuser),0);
		    continue;
		}
	    printf("%s requests logging\n",recuser);
	    strcat(recuser,", please input password (pass [password])");
	    send(fd,recuser,sizeof(recuser),0);
	    sign2 = recv(fd,recpass,sizeof(recpass),0);
	    printf("%s\n",recpass);
	    decrypt(recpass,&org_pass);
		if(strncmp(org_pass,"pass",4)){
	        send(fd,"no this command\0",sizeof(recpass),0);
	        printf("errno pass\n");
		    continue;
		}
		p = getdir(org_pass);
		if(sign1<=0 || sign2<=0)
			return 0;
 	    if(!strcmp(p,password)){
                    send(fd,right,sizeof(right),0);
			return 1;
		}
	    else
	        send(fd,wrong,sizeof(wrong),0);
	}
}

int main(int argc,char** argv)
{
    int listenfd,socketfd,hit,connect_fd,n_read,ver;
    static struct sockaddr_in cli_addr,serv_addr;
	char *temp,msg[256],conway[10],temport[6];
	int pasvport=1030;
	pid_t pid;
    (void)signal(SIGCLD, SIG_IGN); // ignore child death 
    (void)signal(SIGHUP, SIG_IGN); // ignore terminal hangups 
    
    if((listenfd = socket(AF_INET,SOCK_STREAM,0))<0)
        logger(ERROR, "system call", "socket", 0);
    int port = 21;
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //inet_aton(argv[1],&serv_addr.sin_addr);
    serv_addr.sin_port = htons(port);
    logger(LOG, "nweb starting", "127.0.0.1", getpid());
    if(bind(listenfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
        logger(ERROR,"system call","bind",0);
    if(listen(listenfd,64)<0)
       logger(ERROR,"system call","listen",0);
    socklen_t len = sizeof(cli_addr);
    for(hit=1;;hit++){
		//pasvport = pasvport+2;
		//itoa(pasvport,temport,10);
        printf("waiting connect\n");
        connect_fd = accept(listenfd,(struct sockaddr*)&cli_addr,&len);
        if(connect_fd<=0){
            logger(ERROR, "system call", "accept", 0);
	    continue;
        }
		ver = verify(connect_fd);
		if(!ver) continue;
		pid = fork();
		if(pid==0){
			printf("connected successfully,IP is %s\n",inet_ntoa(cli_addr.sin_addr));
            recv(connect_fd,conway,sizeof(conway),0);
		    if(!strncmp(conway,"port",4))
                recv(connect_fd,temport,sizeof(temport),0);
		    else if(!strcmp(conway,"pasv")){
		            strcpy(temport,"1030");
			    send(connect_fd,temport,sizeof(temport),0);
            }		    
		    port = atoi(temport);
		    while(1){
                //memset(msg,'\0',sizeof(msg));
                n_read = recv(connect_fd,msg,sizeof(msg),0);
                if(n_read <= 0){
                    printf("customer out\n");
                    break;
                }
                printf("the command is: %s\n",msg);
                msg_handler(msg,connect_fd,conway,port);
				port = port+2;
            }  
		}
		else if(pid>0) {
			close(connect_fd);
		}
		else{
            perror("create childProcess error"); 
            exit(1);
        } 
    }
    close(listenfd);
}