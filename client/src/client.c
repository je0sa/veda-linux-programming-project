#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAXSIZE 100
#define PORT 60000

void run(int);

int main(int argc, char *argv[])
{
    int sockfd;
    socklen_t addr_len;
    struct hostent *he;
    struct sockaddr_in server_addr;

    sigset_t sigset;
    sigfillset(&sigset); // 모든 시그널을 블록
    sigdelset(&sigset, SIGINT); 
    sigprocmask(SIG_SETMASK, &sigset, NULL);

    if (argc != 2)
    {
        fprintf(stderr, "usage : client hostname \n");
        exit(1);
    }
    if ((he = gethostbyname(argv[1])) == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    printf("[ %s ]\n", (char *)inet_ntoa(server_addr.sin_addr));
    memset(&(server_addr.sin_zero), '\0', 8);
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        exit(1);
    }

    printf("[Connect Success] 라즈베리파이 원격 제어 서버 연결\n");
    run(sockfd);

    return 0;
}

void run(int sd)
{
    char buf[MAXSIZE];
    char packet[MAXSIZE]; 
    int numbytes;

    fd_set fdset, fdset1;

    FD_ZERO(&fdset);
    FD_SET(0, &fdset);
    FD_SET(sd, &fdset);
    fdset1 = fdset;
    
    while (1)
    {
        fdset = fdset1; // select 호출 후, 매번 백업본 복사
        printf("\ninput[1:LED][2:CDS][3:SEGMENT][4:BUZZER][5:EXIT(종료)]: ");
        fflush(stdout); // 키보드 입력 버퍼를 비움
        select(sd + 1, &fdset, NULL, NULL, NULL); // 타임아웃 설정 NULL: 이벤트 발생하기 전까지 Block한다는 의미

        if (FD_ISSET(0, &fdset))
        {
            int selectNum = 0;

            if (scanf("%d", &selectNum) != 1) {
                while(getchar() != '\n'); // 버퍼 비우기
                continue;
            }

            memset(packet, 0, sizeof(packet));  // 패킷버퍼 초기화

            if (selectNum == 1) // LED
            {
                while (1)
                {
                    char led_buf[20] = {0,};
                    printf("명령 입력 ([HIGH][MID][LOW][OFF][EXIT]): ");
                    scanf("%s", led_buf);

                    if (strcmp(led_buf, "EXIT") == 0)
                    {
                        printf("[LED] 메인 메뉴로 돌아갑니다.\n");
                        break;
                    }

                    // ex: "LED HIGH" 형태로 문자열 조립
                    sprintf(packet, "LED %s", led_buf);
                    
                    if(send(sd, packet , strlen(packet) ,0) == -1) {
                        perror("send");
                        exit(1);
                    }
                }
            }
            else if (selectNum == 2) // 조도센서
            {
                int threshold = 0;

                printf("input threshold: ");
                scanf("%d", &threshold);

                // ex: "CDS 150" 형태로 문자열 조립
                sprintf(packet, "CDS %d", threshold);

                if(send(sd, packet, strlen(packet), 0) == -1) {
                    perror("send 실패");
                    exit(1);
                }
            }
            else if (selectNum == 3) // 세그먼트
            {
                int temp_num;
                printf("시작 숫자 입력 (0~9): ");
                scanf("%d", &temp_num);

                if (temp_num < 0 || temp_num > 9)
                {
                    printf("0~9 사이의 숫자를 입력하세요.\n");
                    continue;
                }

                // ex: "SEG 5" 형태로 문자열 조립
                sprintf(packet, "SEG %d", temp_num);

                if(send(sd, packet, strlen(packet), 0) == -1) {
                    perror("send 실패");
                    exit(1);
                }
            }
            else if (selectNum == 4) // 부저
            {
                char buz_buf[20] = {0,};
                printf("명령 입력 ([ON][OFF]): ");
                scanf("%s", buz_buf);

                // ex: "BUZ ON" 형태로 문자열 조립
                sprintf(packet, "BUZ %s", buz_buf);

                if(send(sd, packet, strlen(packet), 0) == -1) {
                    perror("send 실패");
                    exit(1);
                }
            }
            else if (selectNum == 5)
            {
                printf("[Main] 테스트 프로그램 종료 및 연결 끊음\n");
   
                sprintf(packet, "EXIT");
                send(sd, packet, strlen(packet), 0);

                close(sd);
                exit(0);
            }

        }
        else if (FD_ISSET(sd, &fdset))
        {
            memset(buf, 0, sizeof(buf));

            if ((numbytes = recv(sd, buf, MAXSIZE - 1, 0)) == -1)
            {
                perror("recv");
                exit(1);
            }
            else if (numbytes == 0)
            {
                printf("\n서버 연결 종료\n");
                close(sd);
                exit(1);
            }
            buf[numbytes] = '\0';
            printf("[서버에서 전송한 메세지]: %s", buf);
        }
        else
        {
            continue;
        }
    }
}