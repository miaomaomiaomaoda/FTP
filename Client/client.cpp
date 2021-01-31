#include<WinSock2.h>
#include<Windows.h>
#include<iostream>
#include<cstring>
#include<fstream>
#define BUF_SIZE 1024
#define RECV_PORT 3312
#define SEND_PORT 4302
using namespace std;
#pragma comment(lib,"ws2_32.lib")
class Client{
    WSADATA wsaData;
    sockaddr_in serverAddr;
    SOCKET sockClient;
    char ServerIP[20];
    bool log_Status;
public:
    Client(){
		log_Status=false;
		memset(ServerIP,0,sizeof(ServerIP));
	}     //初始化登录状态为未登录
    char rbuff[BUF_SIZE];
    char sbuff[BUF_SIZE];
    char operation[10];     //操作
    char name[20];          //文件名
    char order[30];         //输入的命令
    char buff[40];          //用来存储经过字符串格式化的order,被发送去服务器
    //函数
    int StartSock();   //初始化程序
    void init();          //初始化空间（发送接受缓存区等）
    int creatSocket();    //创建socket
    int callServer();     //发送连接请求
    int Send_command(char *data);  //发送执行命令到服务端
    int put(ifstream&);     //上传文件
    int get();     //下载文件
    void help();    //帮助菜单
    void list();    //列出远方当前目录
    int log();      //登录函数
    bool Status(){      //返回当前登录状态
        return log_Status;
    }
    void Set_Status(bool Status){    log_Status=Status;}
    void shutdown();
    void IntoOrder();        //将指令整合进order,并存放进buff
    SOCKET& Sock_return(){  return sockClient;    }     //返回套接字
};

int Client::StartSock(){  //启动winSock并初始化
    if(WSAStartup(MAKEWORD(2,2),&wsaData)){
        cout<<"sock初始化失败!"<<endl;
        return -1;
    }

    //服务器IP地址输入
    char a[20];
    memset(a,0,sizeof(a));
    if(!strncmp(ServerIP,a,sizeof(a))){
        cout<<"请输入要连接的服务器IP：";
        cin>>ServerIP;
    }

    //设置地址结构
    serverAddr.sin_family=AF_INET;      //通信方式
    serverAddr.sin_addr.s_addr=inet_addr(ServerIP);     //服务器IP地址
    serverAddr.sin_port=htons(RECV_PORT);   //设置端口号
    return 1;
}

void Client::init(){     //初始化缓存区
    memset(operation,0,sizeof(operation));
    memset(name,0,sizeof(name));
    memset(order,0,sizeof(order));
    memset(buff,0,sizeof(buff));
    memset(rbuff,0,sizeof(rbuff));
    memset(sbuff,0,sizeof(sbuff));
}

int Client::creatSocket(){    //创建Socket,创建一个套接字
    sockClient=socket(AF_INET,SOCK_STREAM,0);   //创建一个套接字，ipv4,tcp，
    if(sockClient==SOCKET_ERROR){
        cout<<"创建Socket失败"<<endl;
        shutdown();     //关闭套接字和删除DLL
        return -1;
    }
    return 1;
}

int Client::callServer(){
    creatSocket();      //创建套接字
    if(connect(sockClient,(sockaddr*)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR){
        cout<<"连接失败"<<endl;
        memset(ServerIP,0,sizeof(ServerIP));        //清除服务器IP
        return -1; 
    };//connect()创建与指定外部端口的连接
    return 1;
}

void Client::help(){        //帮助菜单
    cout<<"         ____________________________________________________"<<endl
        <<"         |                   FTP程序设计使用说明              "<<endl
        <<"         |        1.get 下载文件[输入格式：get filename  ]     "<<endl
        <<"         |        2.put 上传文件[输入格式：put filename  ]     "<<endl
        <<"         |        3.pwd 显示当前文件夹的绝对路径[输入格式：pwd] "<<endl
        <<"         |        4.dir 显示服务器当前目录的文件[输入格式：dir] "<<endl
        <<"         |        5.cd 改变远方当前目录和路径                  "<<endl
        <<"         |               进入下级目录:cd path                 "<<endl
        <<"         |               进入上级目录:cd ..                   "<<endl
        <<"         |        6.?/help进入帮助菜单                        "<<endl
        <<"         |        7.quit 退出Ftp                             "<<endl
        <<"         |___________________________________________________"<<endl;
}

int Client::Send_command(char* data){
    int length=send(sockClient,data,strlen(data),0);        //将data的数据copy进入发送缓存区内
    if(length<=0){
        cout<<"发送命令到服务器端失败"<<endl;
        shutdown();
        return -1;
    }
    return 1;
}

int Client::put(ifstream &f2){
    cout<<"正在传输文件..."<<endl;
    memset(sbuff,'\0',sizeof(sbuff));
    while(1){
        f2.read(sbuff,sizeof(sbuff));      //从文件中读取sbuff大小的数据到sbuff
        int len=f2.gcount();               //一共成功读取了多少个字节
        if(send(sockClient,sbuff,len,0)==SOCKET_ERROR){ //发送
            cout<<"与客户端的连接中断"<<endl;
            shutdown();
            return 0;
        }
        if(len<sizeof(sbuff))
            break;
    }
    shutdown();
    cout<<"传输完成"<<endl;
    return 1;
}

void Client::list(){        //列出服务器目录文件
    int nRead;
    memset(sbuff,'\0',sizeof(sbuff));
    while(1){
        nRead=recv(sockClient,rbuff,sizeof(rbuff),0);
        //recv通过SockClient接受数据存入rbuff缓冲区，返回接收到的数据
        if(nRead==SOCKET_ERROR){
            cout<<"读取时发生错误"<<endl;
            exit(1);
        }
        if(!nRead)
            break;      //数据读取完毕

        //显示数据
        rbuff[nRead]='\0';
        cout<<rbuff<<endl;      //输出当前目录文件
    }
}

//按命令log+username+password格式送到服务器,服务器端接受截取账户密码去检验
int Client::log(){          //登录函数(将账户密码送到服务器去检验)
    char username[20],password[20];
    cout<<"Please enter your username:";
    cin>>username;
    cout<<"Please enter your password:";
    cin>>password;
    strcat(order,"log");
    strcat(order,username);
    strcat(order,"+");
    strcat(order,password);
    sprintf(buff,order);
    Send_command(buff);
    recv(sockClient,rbuff,sizeof(rbuff),0);     //接受信息
    cout<<rbuff<<endl;
    if(strcmp(rbuff,"wrong")==0){
        return 0;
    }
    else return 1;  
    //还有的修改：适用于多账户多密码，存入服务器的一个文件，然后在服务器通过一个文件流对象去读取，去检验是否正确
}
void Client::shutdown(){//关闭套接字，删除DLL
    closesocket(sockClient);            //关闭套接字
    WSACleanup();                       //删除DLL
}
void Client::IntoOrder(){   //将指令整合进order
    strcat(order,operation);
    strcat(order," ");
    strcat(order,name);
    sprintf(buff,order);        //写入buff
    Send_command(buff);         //发送指令
    recv(sockClient,rbuff,sizeof(rbuff),0);     //接受信息
    cout<<rbuff<<endl;
}

//程序主体
int  main(){
    Client test;
	int help_point=0;	//实现第一次输出help帮助界面;
    while(1){
        test.StartSock();       //调用winsock并初始化
        if(!test.callServer())  //连接失败
            continue;
        test.init();            //初始化缓冲区等
        if(!test.Status()){         //未登录
            if(test.log()){
                test.Set_Status(TRUE);
				help_point=1;
                continue;       
            }
            continue;
        }

        //指令输入部分
		if(help_point==1){		//登录指令已经加1
			test.help();
			help_point=0;
		}
        cout<<"请输入要执行的指令:";
        cin>>test.operation;
        if(!strncmp(test.operation,"get",3)||!strncmp(test.operation,"put",3)||!strncmp(test.operation,"cd",2)){
            cin>>test.name;
        }   
        else if(!strncmp(test.operation,"quit",4)){
            cout<<"感谢您的使用"<<endl;
            send(test.Sock_return(),test.operation,sizeof(test.operation),0);       //发送quit指令到服务器
            test.shutdown();        //关闭套接字和清除DLL
            break;
        }
        else if(!strncmp(test.operation,"?",1)||!strncmp(test.operation,"help",4)){
            test.help();
        }

        test.IntoOrder();       //将指令整合进Order
        
        int cnt;
        //rbuff（IntoOrder返回的指令）
        if(!strncmp(test.rbuff,"get",3)){ //下载功能
			ofstream f1;
            f1.open(test.name,ios::binary);
            if(f1.fail()){
                cout<<"打开或新建"<<test.name<<"文件失败"<<endl;
				cout<<"请重新输入指令"<<endl;
                continue;
            }				
            memset(test.rbuff,'\0',sizeof(test.rbuff));   //清空缓存区
            while(cnt=recv(test.Sock_return(),test.rbuff,sizeof(test.rbuff),0)>0){
                f1.write(test.rbuff,sizeof(test.rbuff));     //写入文件
            }
            f1.close();
        }
        else if(!strncmp(test.rbuff,"put",3)){ //上传功能
            char filename[40];
            strcpy(filename,test.rbuff+4);
			ifstream f2;
            f2.open(filename,ios::binary);
            if(!f2.fail()){
                if(!test.put(f2)){      //发送文件，发送失败
                    cout<<"文件发送失败"<<endl;
					cout<<"请重新输入指令"<<endl;
                    continue;
                }
                f2.close();
            }
            else{
                strcpy(test.sbuff,"无法打开文件\n");
                if(send(test.Sock_return(),test.sbuff,sizeof(test.sbuff),0))
                    continue;
            }
        }
        else if(!strncmp(test.operation,"dir",3)){
            test.list();
        }
        test.shutdown();
    }
    return 0;
}