#include<iostream>
#include<WinSock2.h>
#include<cstring>
#include<Windows.h>
#include<fstream>
#include<string>
using namespace std;
#define BUF_SIZE 1024
#define RECV_PORT 3312      //接受端口
#define SEND_PORT 4302      //发送端口
#pragma comment(lib,"ws2_32.lib")

class Server{
    SOCKET sockClient,sockServer;
    WSADATA wsaData;
    sockaddr_in serverAddr;     //服务器地址
    sockaddr_in ClientAddr;     //客户端地址
    int addrLen;        //地址长度
    char fileName[20];      //文件名
    char order[20];         //命令
    char rbuff[BUF_SIZE];   //接受缓冲区
    char sbuff[BUF_SIZE];   //输出缓冲区
    char User_password[60];   //需要初始化（构造函数或者其它的函数）
public:  
    Server(){
		strcpy(User_password,"loglrq+123456");
		memset(fileName,0,sizeof(fileName));
	};
    int startSock();
    int createSocket();
    int sendFileRecorder(WIN32_FIND_DATA *pfd);
    int sendFile(ifstream&);
    int sendFilelist();
    int connectProcess();
};

int Server::startSock(){
    if(WSAStartup(MAKEWORD(2,2),&wsaData)){
        cout<<"初始化失败"<<endl;
        return -1;
    }
    return 1;
};
int Server::createSocket(){
    sockClient=socket(AF_INET,SOCK_STREAM,0);
    if(sockClient==SOCKET_ERROR){
        cout<<"创建失败"<<endl;
        WSACleanup();     //清除套接字，DLL
        return -1;
    }
    serverAddr.sin_family=AF_INET;      //通信方式
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);    
    serverAddr.sin_port=htons(RECV_PORT);   //设置端口号
    if(bind(sockClient,(sockaddr*)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR){
        cout<<"绑定失败"<<endl;
        return -1;
    }       //绑定套接字
    return 1;
};

int Server::sendFilelist(){
    HANDLE  lrq;                    //建立线程
    WIN32_FIND_DATA fd;             //搜索文件
    lrq=FindFirstFile("*",&fd);     //查找错误来把待操作文件的相关属性读取到WIN32_FIND_DATA结构中去
    if(lrq==INVALID_HANDLE_VALUE){  //发生错误
        const char *errStr="列出文件列表时发生错误\n";
        cout<<*errStr<<endl;
        if(send(sockServer,errStr,strlen(errStr),0)==SOCKET_ERROR){
            cout<<"发送失败"<<endl;
        }
        
        return 0;
    }
    bool flag=TRUE;
    while(flag){    //发送文件信息
        if(!sendFileRecorder(&fd)){
            closesocket(sockServer);
            return 0;
        }
        flag=FindNextFile(lrq,&fd); //查找下一个文件
    }
    closesocket(sockServer);
    return 1;
};

int Server::sendFile(ifstream &f1){
    cout<<"正在发送文件..."<<endl;
    memset(sbuff,'\0',sizeof(sbuff));
    while(1){
        f1.read(sbuff,sizeof(sbuff));      //从文件中读取sbuff大小的数据到sbuff
        int len=f1.gcount();               //一共成功读取了多少个字节
        if(send(sockServer,sbuff,len,0)==SOCKET_ERROR){ //发送
            cout<<"连接失败"<<endl;
            closesocket(sockServer);
            return 0;
        }
        if(len<sizeof(sbuff))   //文件传输结束
            break;
    }
    closesocket(sockServer);
    cout<<"发送成功"<<endl;
    return 1;
};

int Server::sendFileRecorder(WIN32_FIND_DATA *pfd){
	char fileRecord[MAX_PATH + 32];
	
	FILETIME ft;						//文件的建立时间
	FileTimeToLocalFileTime(&pfd -> ftLastWriteTime, &ft);//Converts a file time to a local file time.
	
	SYSTEMTIME lastWriteTime;
	FileTimeToSystemTime(&ft, &lastWriteTime);
	
	const char *dir = pfd -> dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? "<DIR>" : " ";
	sprintf(fileRecord, "%04d-%02d-%02d %02d:%02d %5s %10d   %-20s\n",
		lastWriteTime.wYear,
		lastWriteTime.wMonth,
		lastWriteTime.wDay,
		lastWriteTime.wHour,
		lastWriteTime.wMinute,
		dir,
		pfd -> nFileSizeLow,
		pfd -> cFileName
	);
	if (send(sockServer, fileRecord, strlen(fileRecord), 0) == SOCKET_ERROR) {
		//通过sockserver接口发送fileRecord数据，成功返回发送的字节数   
		cout << "发送失败" << endl;
		return 0;
	}
	return 1;  
};
int Server::connectProcess(){
    addrLen=sizeof(ClientAddr);     //对象地址的长度
    if(listen(sockClient,10)<0){
        cout<<"监听失败"<<endl;
        return -1;
    }
    cout<<"服务器正在监听中..."<<endl;
    while(1){   //循环去接受客户端的连接请求
        sockServer=accept(sockClient,(sockaddr*)&ClientAddr,&addrLen);      //取出请求队列头部的请求
        while(1){
            memset(rbuff,0,sizeof(rbuff));
            memset(sbuff,0,sizeof(sbuff));
            if(recv(sockServer,rbuff,sizeof(rbuff),0)<=0)
                break;
            cout<<"获取并执行的命令:"<<rbuff<<endl;
            //考虑写个函数，实现rbuff与指令的匹配，返回数字，用switch进行选择
            if(!strncmp(rbuff,"get",3)){    //get命令
                strcpy(fileName,rbuff+4);   //获取文件名
                ifstream f1;
                f1.open(fileName,ios::binary);
                if(f1){    //打开成功
                    sprintf(sbuff,"get %s",fileName);
                    if(!send(sockServer,sbuff,sizeof(sbuff),0)){    //指令读取成功，发送指令回客户端,失败执行
                        f1.close();
						cout<<"发送指令回客户端失败"<<endl;
                        continue;
                    }
                    else{   //创建额外数据连接传送数据
                        if(sendFile(f1))
                            continue;
                        f1.close();
                    }
                }
                else{
                    strcpy(sbuff,"无法打开文件\n");
                    if(send(sockServer,sbuff,sizeof(sbuff),0)){
                        continue;
                    }
                }
            }
            else if(!strncmp(rbuff,"put",3)){
                strcpy(fileName,rbuff+4);
                ofstream f2;
                f2.open(fileName,ios::binary);
                if(!f2){
                    cout<<"无法打开文件"<<fileName<<endl;
                    continue;
                }
                sprintf(sbuff,"put %s",fileName);
                if(!send(sockServer,sbuff,sizeof(sbuff),0)){
                    f2.close();
                    continue;
                }       //指令接受完毕，将指令发回客户端
                memset(sbuff,'\0',sizeof(sbuff));
                int cnt;
                while(cnt=recv(sockServer,rbuff,sizeof(rbuff),0)>0){
                    f2.write(rbuff,sizeof(rbuff));     //写入文件
                }
                cout<<"成功获得文件"<<fileName<<endl;
                f2.close();
            }
            else if(!strncmp(rbuff,"pwd",3)){
                char path[1000];
                GetCurrentDirectory(sizeof(path),path);     //将当前目录装入path缓存区
                strcpy(sbuff,path);
                send(sockServer,sbuff,sizeof(sbuff),0);
            }
            else if(!strncmp(rbuff,"dir",3)){
                strcpy(sbuff,rbuff);
                send(sockServer,sbuff,sizeof(sbuff),0);
                sendFilelist();
            }
            else if(!strncmp(rbuff,"cd",2)){
                strcpy(fileName,rbuff+3);
                strcpy(sbuff,rbuff);
                send(sockServer,sbuff,sizeof(sbuff),0);
                SetCurrentDirectory(fileName);
            }
            else if(!strncmp(rbuff,"log",3)){
                if(!strcmp(rbuff,User_password)){
                    cout<<"登录成功"<<endl;
                    send(sockServer,"right\0",sizeof(sbuff),0);
                }
                else
                    send(sockServer,"wrong\0",sizeof(sbuff),0);
            }
            else if(!strncmp(rbuff,"quit",4)){
                closesocket(sockServer);
                WSACleanup();
                break;;
            }       //退出程序
            closesocket(sockServer);
        }
    }
	return 1;
};

int main(){
    Server test;
	if(test.startSock()==-1||test.createSocket()==-1||test.connectProcess()==-1)
        return 1;		//非正常退出   
    return 0;
}

//对于程序的正常操作没有什么问题，但对于出现问题，异常的处理可能会有点问题