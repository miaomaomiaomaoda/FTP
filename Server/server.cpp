#include<iostream>
#include<WinSock2.h>
#include<cstring>
#include<Windows.h>
#include<fstream>
#include<string>
using namespace std;
#define BUF_SIZE 1024
#define RECV_PORT 3312      //���ܶ˿�
#define SEND_PORT 4302      //���Ͷ˿�
#pragma comment(lib,"ws2_32.lib")

class Server{
    SOCKET sockClient,sockServer;
    WSADATA wsaData;
    sockaddr_in serverAddr;     //��������ַ
    sockaddr_in ClientAddr;     //�ͻ��˵�ַ
    int addrLen;        //��ַ����
    char fileName[20];      //�ļ���
    char order[20];         //����
    char rbuff[BUF_SIZE];   //���ܻ�����
    char sbuff[BUF_SIZE];   //���������
    char User_password[60];   //��Ҫ��ʼ�������캯�����������ĺ�����
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
        cout<<"��ʼ��ʧ��"<<endl;
        return -1;
    }
    return 1;
};
int Server::createSocket(){
    sockClient=socket(AF_INET,SOCK_STREAM,0);
    if(sockClient==SOCKET_ERROR){
        cout<<"����ʧ��"<<endl;
        WSACleanup();     //����׽��֣�DLL
        return -1;
    }
    serverAddr.sin_family=AF_INET;      //ͨ�ŷ�ʽ
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);    
    serverAddr.sin_port=htons(RECV_PORT);   //���ö˿ں�
    if(bind(sockClient,(sockaddr*)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR){
        cout<<"��ʧ��"<<endl;
        return -1;
    }       //���׽���
    return 1;
};

int Server::sendFilelist(){
    HANDLE  lrq;                    //�����߳�
    WIN32_FIND_DATA fd;             //�����ļ�
    lrq=FindFirstFile("*",&fd);     //���Ҵ������Ѵ������ļ���������Զ�ȡ��WIN32_FIND_DATA�ṹ��ȥ
    if(lrq==INVALID_HANDLE_VALUE){  //��������
        const char *errStr="�г��ļ��б�ʱ��������\n";
        cout<<*errStr<<endl;
        if(send(sockServer,errStr,strlen(errStr),0)==SOCKET_ERROR){
            cout<<"����ʧ��"<<endl;
        }
        
        return 0;
    }
    bool flag=TRUE;
    while(flag){    //�����ļ���Ϣ
        if(!sendFileRecorder(&fd)){
            closesocket(sockServer);
            return 0;
        }
        flag=FindNextFile(lrq,&fd); //������һ���ļ�
    }
    closesocket(sockServer);
    return 1;
};

int Server::sendFile(ifstream &f1){
    cout<<"���ڷ����ļ�..."<<endl;
    memset(sbuff,'\0',sizeof(sbuff));
    while(1){
        f1.read(sbuff,sizeof(sbuff));      //���ļ��ж�ȡsbuff��С�����ݵ�sbuff
        int len=f1.gcount();               //һ���ɹ���ȡ�˶��ٸ��ֽ�
        if(send(sockServer,sbuff,len,0)==SOCKET_ERROR){ //����
            cout<<"����ʧ��"<<endl;
            closesocket(sockServer);
            return 0;
        }
        if(len<sizeof(sbuff))   //�ļ��������
            break;
    }
    closesocket(sockServer);
    cout<<"���ͳɹ�"<<endl;
    return 1;
};

int Server::sendFileRecorder(WIN32_FIND_DATA *pfd){
	char fileRecord[MAX_PATH + 32];
	
	FILETIME ft;						//�ļ��Ľ���ʱ��
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
		//ͨ��sockserver�ӿڷ���fileRecord���ݣ��ɹ����ط��͵��ֽ���   
		cout << "����ʧ��" << endl;
		return 0;
	}
	return 1;  
};
int Server::connectProcess(){
    addrLen=sizeof(ClientAddr);     //�����ַ�ĳ���
    if(listen(sockClient,10)<0){
        cout<<"����ʧ��"<<endl;
        return -1;
    }
    cout<<"���������ڼ�����..."<<endl;
    while(1){   //ѭ��ȥ���ܿͻ��˵���������
        sockServer=accept(sockClient,(sockaddr*)&ClientAddr,&addrLen);      //ȡ���������ͷ��������
        while(1){
            memset(rbuff,0,sizeof(rbuff));
            memset(sbuff,0,sizeof(sbuff));
            if(recv(sockServer,rbuff,sizeof(rbuff),0)<=0)
                break;
            cout<<"��ȡ��ִ�е�����:"<<rbuff<<endl;
            //����д��������ʵ��rbuff��ָ���ƥ�䣬�������֣���switch����ѡ��
            if(!strncmp(rbuff,"get",3)){    //get����
                strcpy(fileName,rbuff+4);   //��ȡ�ļ���
                ifstream f1;
                f1.open(fileName,ios::binary);
                if(f1){    //�򿪳ɹ�
                    sprintf(sbuff,"get %s",fileName);
                    if(!send(sockServer,sbuff,sizeof(sbuff),0)){    //ָ���ȡ�ɹ�������ָ��ؿͻ���,ʧ��ִ��
                        f1.close();
						cout<<"����ָ��ؿͻ���ʧ��"<<endl;
                        continue;
                    }
                    else{   //���������������Ӵ�������
                        if(sendFile(f1))
                            continue;
                        f1.close();
                    }
                }
                else{
                    strcpy(sbuff,"�޷����ļ�\n");
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
                    cout<<"�޷����ļ�"<<fileName<<endl;
                    continue;
                }
                sprintf(sbuff,"put %s",fileName);
                if(!send(sockServer,sbuff,sizeof(sbuff),0)){
                    f2.close();
                    continue;
                }       //ָ�������ϣ���ָ��ؿͻ���
                memset(sbuff,'\0',sizeof(sbuff));
                int cnt;
                while(cnt=recv(sockServer,rbuff,sizeof(rbuff),0)>0){
                    f2.write(rbuff,sizeof(rbuff));     //д���ļ�
                }
                cout<<"�ɹ�����ļ�"<<fileName<<endl;
                f2.close();
            }
            else if(!strncmp(rbuff,"pwd",3)){
                char path[1000];
                GetCurrentDirectory(sizeof(path),path);     //����ǰĿ¼װ��path������
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
                    cout<<"��¼�ɹ�"<<endl;
                    send(sockServer,"right\0",sizeof(sbuff),0);
                }
                else
                    send(sockServer,"wrong\0",sizeof(sbuff),0);
            }
            else if(!strncmp(rbuff,"quit",4)){
                closesocket(sockServer);
                WSACleanup();
                break;;
            }       //�˳�����
            closesocket(sockServer);
        }
    }
	return 1;
};

int main(){
    Server test;
	if(test.startSock()==-1||test.createSocket()==-1||test.connectProcess()==-1)
        return 1;		//�������˳�   
    return 0;
}

//���ڳ������������û��ʲô���⣬�����ڳ������⣬�쳣�Ĵ�����ܻ��е�����