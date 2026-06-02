#include <stdio.h>
#include <stdlib.h>
#include <dlfmap.h> // 동적 로딩을 위한 헤더 (컴파일 시 -ldl 필요)
#include "../include/device_control.h"

int main() {
    printf("[Server] TCP 장치 제어 서버 시작...\n");
    
    // TODO: 2일차에 완벽한 Daemon 프로세스 전환 로직 구현 (fork, setsid 등)
    // TODO: TCP 소켓 바인딩 및 Listen 로직 구현
    
    printf("[Server] 멀티 프로세스/스레드 기반 클라이언트 대기 뼈대 준비 완료\n");
    return 0;
}