#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
// #pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll

#define BUF_SIZE 1024

int main(){
    //先检查文件是否存在
    // char filename[] = "D:\\RECAIUS_CH_0307.mp4";  //文件名
    char filename[100] = {0};  //文件名
    printf("Input filename to send (ie. D:\\\\send.avi):");
    scanf("%s", filename);
    printf("filename = %s\n", filename);
    FILE *fp = fopen(filename, "rb");  //以二进制方式打开文件
    if(fp == NULL){
        printf("Cannot open file, press any key to exit!\n");
        system("pause");
        exit(0);
    }

    // WSADATA wsaData;
    // WSAStartup( MAKEWORD(2, 2), &wsaData);
    int servSock = socket(PF_INET, SOCK_STREAM, 0);

    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = PF_INET;
    // sockAddr.sin_addr.s_addr = inet_addr("192.168.1.106");
    sockAddr.sin_addr.s_addr = inet_addr("172.17.220.113"); //连接阿里云client用公网ip,服务器用ifconfig ip
    printf("s_addr = %d\n", sockAddr.sin_addr.s_addr);
    sockAddr.sin_port = htons(80);
    // bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    bind(servSock, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
    listen(servSock, 20);

    struct sockaddr_in clntAddr;
    // int nSize = sizeof(clntAddr);
    socklen_t clnt_addr_size = sizeof(clntAddr);
    int clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clnt_addr_size);

    //循环发送数据，直到文件结尾
    char buffer[BUF_SIZE] = {0};  //缓冲区
    int nCount;
    while( (nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0 ){
        write(clntSock, buffer, nCount);
    }

    shutdown(clntSock, SHUT_WR);  //文件读取完毕，断开输出流，向客户端发送FIN包
    recv(clntSock, buffer, BUF_SIZE, 0);  //阻塞，等待客户端接收完毕

    fclose(fp);
    close(clntSock);
    close(servSock);
    // WSACleanup();

    system("pause");
    return 0;
}
