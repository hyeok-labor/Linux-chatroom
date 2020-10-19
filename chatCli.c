// echo client ( 입력받은 내용을 서버에 전송 )

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1024

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr = {
        0,
    };
    struct sockaddr_in client_addr = {
        0,
    };
    socklen_t client_size = sizeof(client_addr);

    char sendBuf[MAX_SIZE];
    char recvBuf[MAX_SIZE];

    // create socket
    sock = socket(PF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    /*if (argc != 2) // 포트 및 ip 미입력시
    {
        printf("사용법 : %s <127.0.0.1> <port>\n", argv[0]);
        exit(0);
    }*/

    // connect()
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    read(sock, recvBuf, MAX_SIZE); // 연결 여부 메시지 확인용
    printf("%s\n", recvBuf);

    while (1)
    {
        fputs("input >> ", stdout);      // put message
        fgets(sendBuf, MAX_SIZE, stdin); // get message

        write(sock, sendBuf, MAX_SIZE);
        if (!strncmp("q", sendBuf, strlen("q")))
        {
            break;
        }
    }

    close(sock);

    return 0;
}