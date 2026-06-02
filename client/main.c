#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// SIGINT(Ctrl+C) 발생 시 호출될 핸들러
void sigint_handler(int sig) {
    printf("\n[Client] SIGINT(%d) 감지. 서버 연결을 안전하게 종료하고 프로그램을 마칩니다.\n", sig);
    exit(0);
}

int main() {
    // 시그널 핸들러 등록
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    printf("[Client] 클라이언트 프로그램 시작 (종료하려면 Ctrl+C 입력)\n");

    // TODO: TCP 서버 연결 로직 구현
    
    // 메뉴 대기 루프 뼈대
    while(1) {
        printf("\n--- 장치 제어 메뉴 ---\n");
        printf("1. LED 제어\n2. 부저 제어\n3. 조도센서 확인\n4. 7세그먼트 카운트다운\n");
        printf("선택: ");
        
        // 임시 무한루프 방지용 대기
        sleep(5); 
    }

    return 0;
}