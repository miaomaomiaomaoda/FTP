#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "ws2_32.lib")
#define BUF_SIZE 1024
int main()
{
   //��ʼ��dll
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2, 2), &wsaData);

   //�����׽���
   SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

   //����sockaddr_in�ṹ�����,���ڴ洢ip����,��ַ,�˿�
   sockaddr_in sockAddr;
   memset(&sockAddr, 0, sizeof(sockAddr));
   sockAddr.sin_family = AF_INET;
   sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   sockAddr.sin_port = htons(1234);

   //��������
   connect(sock, (SOCKADDR *)&sockAddr, sizeof(SOCKADDR));

   char filename[100] = {0};				//�ļ���
   printf("�������ļ���: ");
   gets(filename);
   FILE *fp = fopen(filename, "rb"); //�Զ����Ʒ�ʽ�򿪣��������ļ�
   char buffer[BUF_SIZE] = {0}; //������
   int nCount;
   while ((nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0)
   {
      send(sock, buffer, nCount, 0); //��fp����buffer,��������
   }

   //�ļ�������Ϻ�ֱ�ӹر��׽���
   fclose(fp);
   closesocket(sock);

   //��ֹDLL
   WSACleanup();
   system("pause");
   return 0;
}