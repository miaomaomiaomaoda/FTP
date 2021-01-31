#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "ws2_32.lib")
#define BUF_SIZE 1024
int main()
{
   //初始化dll
   WSADATA wsaData;
   WSAStartup(MAKEWORD(2, 2), &wsaData);

   //创建套接字
   SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

   //创建sockaddr_in结构体变量,用于存储ip类型,地址,端口
   sockaddr_in sockAddr;
   memset(&sockAddr, 0, sizeof(sockAddr));
   sockAddr.sin_family = AF_INET;
   sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   sockAddr.sin_port = htons(1234);

   //建立连接
   connect(sock, (SOCKADDR *)&sockAddr, sizeof(SOCKADDR));

   char filename[100] = {0};				//文件名
   printf("请输入文件名: ");
   gets(filename);
   FILE *fp = fopen(filename, "rb"); //以二进制方式打开（创建）文件
   char buffer[BUF_SIZE] = {0}; //缓冲区
   int nCount;
   while ((nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0)
   {
      send(sock, buffer, nCount, 0); //将fp读入buffer,发送数据
   }

   //文件接收完毕后直接关闭套接字
   fclose(fp);
   closesocket(sock);

   //终止DLL
   WSACleanup();
   system("pause");
   return 0;
}