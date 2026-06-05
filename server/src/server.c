#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>        
#include <sys/resource.h>   
#include <syslog.h>         

#define PORT 60000
#define BACKLOG 10
#define BUF_SIZE 100

typedef struct {
    int threshold;
    int client_sock;
} CDS_DATA;

void *client_handler(void *);
void parse_and_execute(char *, int);
void *led_thread(void *);
void *cds_thread(void *);
void *seg_thread(void *);
void *buz_thread(void *);
void become_daemon(const char *);

int main(int argc, char **argv)
{
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int sin_size;   
    pthread_t tid;
    /*[데몬화] 소켓을 열기 전, 프로세스를 백그라운드로 전환하고 시스템 자원을 분리*/
    become_daemon(argv[0]);

    if((server_sock = socket(AF_INET,SOCK_STREAM,0)) == -1){
        syslog(LOG_ERR, "Socket Error");
        exit(1);
    }

    /*TIME_WAIT 상태의 포트를 즉시 재사용하도록 허용 (서버 재시작 시 바인딩 에러 방지)*/
    int opt = 1; // 포트 재사용 옵션 플래그
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&(server_addr.sin_zero),'\0', 8);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
        syslog(LOG_ERR, "Bind Error");
        exit(1);
    }

    if(listen(server_sock,BACKLOG) == -1){
        syslog(LOG_ERR, "Listen Error");
        exit(1);
    }

    syslog(LOG_INFO, "[TCP Server] Client 대기 모드");

        while(1){
            sin_size = sizeof(struct sockaddr_in);
            /*[연결 수락] 클라이언트의 접속을 무한 대기*/
            if((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size)) == -1){
            syslog(LOG_ERR, "accept Error");
            continue;
        }   

        syslog(LOG_INFO, "[Client Connect] 우분투 원격 클라이언트 접속 성공.");

        int *client_sock_ptr = (int *)malloc(sizeof(int));
        *client_sock_ptr = client_sock;

        if (client_sock_ptr == NULL) {
            syslog(LOG_ERR, "malloc Error");
            close(client_sock);
            continue;
        }
        /*[멀티스레딩] 클라이언트가 접속할 때 마다 전담 통신 스레드 생성 (메인 루프 차단 방지)*/
        if(pthread_create(&tid, NULL, client_handler, client_sock_ptr)!= 0){
            syslog(LOG_ERR, "pcreate Error");
            free(client_sock_ptr);
            close(client_sock);
            continue;
        }
        pthread_detach(tid);    // 스레드 자원 자동 회수 

    }
    close(server_sock);
    closelog(); // 데몬 프로세스 탈출 시 로그 종료 세션 닫기
    return 0;
}

void *client_handler(void *arg)
{
    int client_sock = *((int*)arg);
    free(arg);
    char buf[BUF_SIZE];
    int str_len;

    /* [패킷 수신] 클라이언트 소켓으로부터 데이터를 계속 수신 */
    while((str_len = recv(client_sock, buf, sizeof(buf)-1, 0)) > 0) {
        buf[str_len] = '\0';
        
        syslog(LOG_INFO, "[Packet Received] 수신 신호: %s", buf);

        /* 데이터 파싱 */
        parse_and_execute(buf, client_sock);
    }

    syslog(LOG_INFO, "[Disconnect] 우분투 원격 클라이언트 접속 해제.");
    close(client_sock);
    return 0;
}

void parse_and_execute(char *msg, int client_sock)
{
    char cmd[20] = {0,};
    char arg[20] = {0,};
    
    /*[파싱] 패킷 문자열(예: "LED HIGH")을 명령어와 인자로 분리*/
    sscanf(msg, "%s %s", cmd, arg);

    if(strcmp(cmd, "LED") == 0) {
        char *buf = (char *)malloc(sizeof(char) * 10);
        strcpy(buf, arg); 
        pthread_t tid;
        pthread_create(&tid, NULL, led_thread, buf);    // LED 제어 스레드 파생
        pthread_detach(tid);
    }
    else if(strcmp(cmd, "CDS") == 0) {

        CDS_DATA *data = (CDS_DATA *)malloc(sizeof(CDS_DATA));
        data->threshold = atoi(arg);
        data->client_sock = client_sock; // 현재 소켓 저장

        pthread_t tid;
        pthread_create(&tid, NULL, cds_thread, data);   // CDS 제어 스레드 파생
        pthread_detach(tid);
    }
    else if(strcmp(cmd, "SEG") == 0) {
        int *start_num = (int *)malloc(sizeof(int));
        *start_num = atoi(arg); 
        pthread_t tid;
        pthread_create(&tid, NULL, seg_thread, start_num);
        pthread_detach(tid);
    }
    else if(strcmp(cmd, "BUZ") == 0) {
        char *buf = (char *)malloc(sizeof(char) * 10);
        strcpy(buf, arg); 
        pthread_t tid;
        pthread_create(&tid, NULL, buz_thread, buf);    // SEG 제어 스레드 파생
        pthread_detach(tid);
    }
    else if(strcmp(cmd, "EXIT") == 0) {
        syslog(LOG_INFO, "[Server] 클라이언트가 원격 메뉴를 빠져나갔습니다.");
    }
}

/*[동적 로딩] 동적 라이브러리 로드*/
void *led_thread(void *arg)
{
    void* handle;
    void (*ptr)(char*);
    char* error;

    handle = dlopen("./libdevice.so", RTLD_LAZY | RTLD_NODELETE);
    if (!handle) {
        syslog(LOG_ERR, "dlopen 실패: %s", dlerror());
        free(arg); 
        pthread_exit(NULL);
    }

    ptr = dlsym(handle, "led_function");

    error = dlerror();
    if (error != NULL) {
        syslog(LOG_ERR, "dlopen 실패: %s", dlerror());
        free(arg);
        dlclose(handle);
        pthread_exit(NULL);
    }

    ptr((char *)arg);

    free(arg); 
    dlclose(handle);
    pthread_exit(NULL); 
}

void *cds_thread(void *arg)
{
    CDS_DATA *data = (CDS_DATA *)arg;
    int threshold = data->threshold;
    int sock = data->client_sock;
    free(data);
    
    void* handle;
    void (*ptr)(int, int);
    char* error;

    handle = dlopen("./libdevice.so", RTLD_LAZY | RTLD_NODELETE);
    if (!handle) {
        syslog(LOG_ERR, "dlopen 실패: %s", dlerror());
        pthread_exit(NULL);
    }

    ptr = dlsym(handle, "cds_function");
    error = dlerror();
    if (error != NULL) {
        syslog(LOG_ERR, "dlopen 실패: %s", dlerror());
        dlclose(handle);
        pthread_exit(NULL);
    }

    ptr(threshold, sock);
    dlclose(handle);
    pthread_exit(NULL);
}

void *seg_thread(void *arg)
{
    void* handle;
    void (*ptr)(int);
    char* error;

    handle = dlopen("./libdevice.so", RTLD_LAZY | RTLD_NODELETE);
    if (!handle) {
        syslog(LOG_ERR, "dlopen 실패: %s", dlerror());
        free(arg);
        pthread_exit(NULL);
    }

    ptr = dlsym(handle, "seg_function");
    error = dlerror();
    if (error != NULL) {
        syslog(LOG_ERR, "dlopen 실패: %s", dlerror());
        free(arg);
        dlclose(handle);
        pthread_exit(NULL);
    }

    ptr(*((int*)arg));

    free(arg); 
    dlclose(handle);
    pthread_exit(NULL);
}

void *buz_thread(void *arg)
{
    void* handle;
    void (*ptr)(char*);
    char* error;

    handle = dlopen("./libdevice.so", RTLD_LAZY | RTLD_NODELETE);
    if (!handle) {
        syslog(LOG_ERR, "dlopen 실패: %s", dlerror());
        free(arg);
        pthread_exit(NULL);
    }

    ptr = dlsym(handle, "buz_function");
    error = dlerror();
    if (error != NULL) {
        syslog(LOG_ERR, "dlopen 실패: %s", dlerror());
        free(arg);
        dlclose(handle);
        pthread_exit(NULL);
    }

    ptr((char*)arg);

    free(arg); 
    dlclose(handle);
    pthread_exit(NULL);
}

void become_daemon(const char *cmd) 
{
    struct sigaction sa;
    struct rlimit rl;
    int fd0, fd1, fd2, i;
    pid_t pid;

    /* 파일 생성을 위한 마스크를 0으로 설정(권한 해제) */
    umask(0);

    /* 사용할 수 있는 최대의 파일 디스크립터 수 얻기 */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        perror("getlimit()");
    }

    /* fork()로 자식 프로세스 생성 후 부모 종료 (백그라운드 진입) */
    if ((pid = fork()) < 0) {
        perror("fork() error");
        exit(1);
    } else if (pid != 0) { 
        exit(0); // 부모는 터미널을 반환하고 즉시 퇴장
    }

    /* 터미널 제어권을 해제하기 위해 완전히 새로운 세션 세우기 */
    setsid();

    /* 세션 리더 이탈 시 발생할 수 있는 SIGHUP 신호를 무시 */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) { 
        perror("sigaction() : Can't ignore SIGHUP");
    }

    /* 상속받은 모든 파일 디스크립터 정리 (자원 누수 방지) */
    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++) {
        close(i);
    }

    /* 표준 I/O(0, 1, 2) 장치를 커널 쓰레기통 파일인 /dev/null 리다이렉트 연결 */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    /* 시스템 커널 로그(/var/log/syslog) 등록 */
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
        exit(1);
    }

    syslog(LOG_INFO, "라즈베리파이 백그라운드 원격 제어 서버 데몬화 성공");
}