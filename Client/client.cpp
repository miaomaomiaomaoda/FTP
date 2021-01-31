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
	}     //��ʼ����¼״̬Ϊδ��¼
    char rbuff[BUF_SIZE];
    char sbuff[BUF_SIZE];
    char operation[10];     //����
    char name[20];          //�ļ���
    char order[30];         //���������
    char buff[40];          //�����洢�����ַ�����ʽ����order,������ȥ������
    //����
    int StartSock();   //��ʼ������
    void init();          //��ʼ���ռ䣨���ͽ��ܻ������ȣ�
    int creatSocket();    //����socket
    int callServer();     //������������
    int Send_command(char *data);  //����ִ����������
    int put(ifstream&);     //�ϴ��ļ�
    int get();     //�����ļ�
    void help();    //�����˵�
    void list();    //�г�Զ����ǰĿ¼
    int log();      //��¼����
    bool Status(){      //���ص�ǰ��¼״̬
        return log_Status;
    }
    void Set_Status(bool Status){    log_Status=Status;}
    void shutdown();
    void IntoOrder();        //��ָ�����Ͻ�order,����Ž�buff
    SOCKET& Sock_return(){  return sockClient;    }     //�����׽���
};

int Client::StartSock(){  //����winSock����ʼ��
    if(WSAStartup(MAKEWORD(2,2),&wsaData)){
        cout<<"sock��ʼ��ʧ��!"<<endl;
        return -1;
    }

    //������IP��ַ����
    char a[20];
    memset(a,0,sizeof(a));
    if(!strncmp(ServerIP,a,sizeof(a))){
        cout<<"������Ҫ���ӵķ�����IP��";
        cin>>ServerIP;
    }

    //���õ�ַ�ṹ
    serverAddr.sin_family=AF_INET;      //ͨ�ŷ�ʽ
    serverAddr.sin_addr.s_addr=inet_addr(ServerIP);     //������IP��ַ
    serverAddr.sin_port=htons(RECV_PORT);   //���ö˿ں�
    return 1;
}

void Client::init(){     //��ʼ��������
    memset(operation,0,sizeof(operation));
    memset(name,0,sizeof(name));
    memset(order,0,sizeof(order));
    memset(buff,0,sizeof(buff));
    memset(rbuff,0,sizeof(rbuff));
    memset(sbuff,0,sizeof(sbuff));
}

int Client::creatSocket(){    //����Socket,����һ���׽���
    sockClient=socket(AF_INET,SOCK_STREAM,0);   //����һ���׽��֣�ipv4,tcp��
    if(sockClient==SOCKET_ERROR){
        cout<<"����Socketʧ��"<<endl;
        shutdown();     //�ر��׽��ֺ�ɾ��DLL
        return -1;
    }
    return 1;
}

int Client::callServer(){
    creatSocket();      //�����׽���
    if(connect(sockClient,(sockaddr*)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR){
        cout<<"����ʧ��"<<endl;
        memset(ServerIP,0,sizeof(ServerIP));        //���������IP
        return -1; 
    };//connect()������ָ���ⲿ�˿ڵ�����
    return 1;
}

void Client::help(){        //�����˵�
    cout<<"         ____________________________________________________"<<endl
        <<"         |                   FTP�������ʹ��˵��              "<<endl
        <<"         |        1.get �����ļ�[�����ʽ��get filename  ]     "<<endl
        <<"         |        2.put �ϴ��ļ�[�����ʽ��put filename  ]     "<<endl
        <<"         |        3.pwd ��ʾ��ǰ�ļ��еľ���·��[�����ʽ��pwd] "<<endl
        <<"         |        4.dir ��ʾ��������ǰĿ¼���ļ�[�����ʽ��dir] "<<endl
        <<"         |        5.cd �ı�Զ����ǰĿ¼��·��                  "<<endl
        <<"         |               �����¼�Ŀ¼:cd path                 "<<endl
        <<"         |               �����ϼ�Ŀ¼:cd ..                   "<<endl
        <<"         |        6.?/help��������˵�                        "<<endl
        <<"         |        7.quit �˳�Ftp                             "<<endl
        <<"         |___________________________________________________"<<endl;
}

int Client::Send_command(char* data){
    int length=send(sockClient,data,strlen(data),0);        //��data������copy���뷢�ͻ�������
    if(length<=0){
        cout<<"���������������ʧ��"<<endl;
        shutdown();
        return -1;
    }
    return 1;
}

int Client::put(ifstream &f2){
    cout<<"���ڴ����ļ�..."<<endl;
    memset(sbuff,'\0',sizeof(sbuff));
    while(1){
        f2.read(sbuff,sizeof(sbuff));      //���ļ��ж�ȡsbuff��С�����ݵ�sbuff
        int len=f2.gcount();               //һ���ɹ���ȡ�˶��ٸ��ֽ�
        if(send(sockClient,sbuff,len,0)==SOCKET_ERROR){ //����
            cout<<"��ͻ��˵������ж�"<<endl;
            shutdown();
            return 0;
        }
        if(len<sizeof(sbuff))
            break;
    }
    shutdown();
    cout<<"�������"<<endl;
    return 1;
}

void Client::list(){        //�г�������Ŀ¼�ļ�
    int nRead;
    memset(sbuff,'\0',sizeof(sbuff));
    while(1){
        nRead=recv(sockClient,rbuff,sizeof(rbuff),0);
        //recvͨ��SockClient�������ݴ���rbuff�����������ؽ��յ�������
        if(nRead==SOCKET_ERROR){
            cout<<"��ȡʱ��������"<<endl;
            exit(1);
        }
        if(!nRead)
            break;      //���ݶ�ȡ���

        //��ʾ����
        rbuff[nRead]='\0';
        cout<<rbuff<<endl;      //�����ǰĿ¼�ļ�
    }
}

//������log+username+password��ʽ�͵�������,�������˽��ܽ�ȡ�˻�����ȥ����
int Client::log(){          //��¼����(���˻������͵�������ȥ����)
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
    recv(sockClient,rbuff,sizeof(rbuff),0);     //������Ϣ
    cout<<rbuff<<endl;
    if(strcmp(rbuff,"wrong")==0){
        return 0;
    }
    else return 1;  
    //���е��޸ģ������ڶ��˻������룬�����������һ���ļ���Ȼ���ڷ�����ͨ��һ���ļ�������ȥ��ȡ��ȥ�����Ƿ���ȷ
}
void Client::shutdown(){//�ر��׽��֣�ɾ��DLL
    closesocket(sockClient);            //�ر��׽���
    WSACleanup();                       //ɾ��DLL
}
void Client::IntoOrder(){   //��ָ�����Ͻ�order
    strcat(order,operation);
    strcat(order," ");
    strcat(order,name);
    sprintf(buff,order);        //д��buff
    Send_command(buff);         //����ָ��
    recv(sockClient,rbuff,sizeof(rbuff),0);     //������Ϣ
    cout<<rbuff<<endl;
}

//��������
int  main(){
    Client test;
	int help_point=0;	//ʵ�ֵ�һ�����help��������;
    while(1){
        test.StartSock();       //����winsock����ʼ��
        if(!test.callServer())  //����ʧ��
            continue;
        test.init();            //��ʼ����������
        if(!test.Status()){         //δ��¼
            if(test.log()){
                test.Set_Status(TRUE);
				help_point=1;
                continue;       
            }
            continue;
        }

        //ָ�����벿��
		if(help_point==1){		//��¼ָ���Ѿ���1
			test.help();
			help_point=0;
		}
        cout<<"������Ҫִ�е�ָ��:";
        cin>>test.operation;
        if(!strncmp(test.operation,"get",3)||!strncmp(test.operation,"put",3)||!strncmp(test.operation,"cd",2)){
            cin>>test.name;
        }   
        else if(!strncmp(test.operation,"quit",4)){
            cout<<"��л����ʹ��"<<endl;
            send(test.Sock_return(),test.operation,sizeof(test.operation),0);       //����quitָ�������
            test.shutdown();        //�ر��׽��ֺ����DLL
            break;
        }
        else if(!strncmp(test.operation,"?",1)||!strncmp(test.operation,"help",4)){
            test.help();
        }

        test.IntoOrder();       //��ָ�����Ͻ�Order
        
        int cnt;
        //rbuff��IntoOrder���ص�ָ�
        if(!strncmp(test.rbuff,"get",3)){ //���ع���
			ofstream f1;
            f1.open(test.name,ios::binary);
            if(f1.fail()){
                cout<<"�򿪻��½�"<<test.name<<"�ļ�ʧ��"<<endl;
				cout<<"����������ָ��"<<endl;
                continue;
            }				
            memset(test.rbuff,'\0',sizeof(test.rbuff));   //��ջ�����
            while(cnt=recv(test.Sock_return(),test.rbuff,sizeof(test.rbuff),0)>0){
                f1.write(test.rbuff,sizeof(test.rbuff));     //д���ļ�
            }
            f1.close();
        }
        else if(!strncmp(test.rbuff,"put",3)){ //�ϴ�����
            char filename[40];
            strcpy(filename,test.rbuff+4);
			ifstream f2;
            f2.open(filename,ios::binary);
            if(!f2.fail()){
                if(!test.put(f2)){      //�����ļ�������ʧ��
                    cout<<"�ļ�����ʧ��"<<endl;
					cout<<"����������ָ��"<<endl;
                    continue;
                }
                f2.close();
            }
            else{
                strcpy(test.sbuff,"�޷����ļ�\n");
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