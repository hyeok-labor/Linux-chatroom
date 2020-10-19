#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <string.h>

#define MAXLINE 512
#define MAX_SOCK 64
char *escapechar = "exit";
int maxfdp1;
int num_chat = 0; // 채팅 참가자 수
int client_s[MAX_SOCK];

/* 채팅 탈퇴 처리 */
void removeClient(int i) // i번째 매개변수
{
    close(client_s[i]); /* 해당 소켓 닫기 */
    if (i != num_chat - 1)
        client_s[i] = client_s[num_chat - 1]; //  i번째 참가자와 맨 마지말 참가장의 위치 switch
    num_chat--;                               // 참가자 수 감소
    printf("1 participant out . now = %d\n", num_chat);
}

/* client_s[]내의 최대 소켓번호 얻기 (k는 초기치)*/
int getmax(int k)
{
    int max = k;
    int r;
    for (r = 0; r < num_chat; r++) // 채팅 참가자 수만큼 소켓 배열 탐색
        if (client_s[r] > max)
            max = client_s[r];
    return max;
}
int main(int argc, char *argv[])
{
    char rline[MAXLINE], my_msg[MAXLINE];       // buffer 생성
    char *start = "Connetctd to chat server\n"; // 최초 연결시
    int i, j, n;
    int s, client_fd, clilen;
    fd_set read_fds;
    struct sockaddr_in client_addr, server_addr; // 소켓주소 구조체 선언
    if (argc != 2)                               // 포트번호를 입력 안했을 경우
    {
        printf("사용법 : %s <port>\n", argv[0]);
        exit(0);
    }
    /* 초기소켓 생성 */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Server: Can't open stream socket.");
        exit(0);
    }
    /* server_addr 구조체의 내용 세팅 */
    bzero((char *)&server_addr, sizeof(server_addr)); //초기화(0으로 채운다)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Server: Can't bind local address.\n");
        exit(0);
    }

    /*클라이언트로부터 연결요청을 기다림 */
    listen(s, 5);    // listen 접속 대기 (큐 크기 5)
    maxfdp1 = s + 1; // 최대 소켓번호 +1

    while (1)
    {
        FD_ZERO(&read_fds);            // fd_set  초기화(0으로 세팅)
        FD_SET(s, &read_fds);          // 소켓에 해당하는 file discripter를 1로 세팅*/
        for (i = 0; i < num_chat; i++) // ""
            FD_SET(client_s[i], &read_fds);
        maxfdp1 = getmax(s) + 1; // maxfdp1 재 계산 */

        if (select(maxfdp1, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0) // select setting
        {
            printf("select error =< 0 \n");
            exit(0);
        }

        if (FD_ISSET(s, &read_fds)) // read_fds 중 s에 해당하는 비트가 세팅되있다면
        {                           // == 연결요청이 있다면
            clilen = sizeof(client_addr);
            client_fd = accept(s, (struct sockaddr *)&client_addr, &clilen); //  accept()
            if (client_fd == -1)
            {
                printf("accept error \n");
                exit(0);
            }
            /* 채팅 클라이언트 목록에 추가 */
            client_s[num_chat] = client_fd;
            num_chat++;                               // 채팅 참가자수 1 증가
            send(client_fd, start, strlen(start), 0); // "Connetctd to chat_server" 를 접속한 클라이언트에 보냄
            printf("No.%d partipatant added,\n", num_chat);
        }

        /*클라이언트가 보낸 메시지를 모든 클라이언트에게 방송     ==> 미사용, 클라이언트에서 메시지 받는 기능 제거함. */
        for (i = 0; i < num_chat; i++) // 모든 클라이언트 검색
        {
            memset(rline, '\0', MAXLINE);         // buffer 초기화
            if (FD_ISSET(client_s[i], &read_fds)) // read 해당 소켓에서 read 할 것이 있는지 확인
            {
                if ((n = recv(client_s[i], rline, MAXLINE, 0)) <= 0)
                {
                    removeClient(i);
                    continue;
                }
                /* 종료문자 처리 */
                if (strstr(rline, escapechar) != NULL) // "exit" ==> 종료
                {
                    removeClient(i);
                    continue;
                }
                // 모든 채팅 참가자에게 메시지 방송
                rline[n] = '\0';
                for (j = 0; j < num_chat; j++)
                    send(client_s[j], rline, n, 0);
                printf("%s\n", rline);
            }
        }
    }
    return 0;
}
