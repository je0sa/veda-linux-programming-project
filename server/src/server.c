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

int main(int argc, char **argv)
{
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int sin_size;   
    pthread_t tid;

    if((server_sock = socket(AF_INET,SOCK_STREAM,0)) == -1){
        perror("socket");
        exit(1);
    }

    int opt = 1; // 포트 재사용 옵션 플래그(서버 재시작 시 바인딩 에러 방지)
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&(server_addr.sin_zero),'\0', 8);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
        perror("bind");
        exit(1);
    }

    if(listen(server_sock,BACKLOG) == -1){
        perror("listen");
        exit(1);
    }

    printf("[TCP Server] 라즈베리파이 원격 장치 제어 서버 가동 완료 (Port: %d)...\n", PORT);

        while(1){
            sin_size = sizeof(struct sockaddr_in);
        if((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size)) == -1){
            perror("accept");
            continue;
        }

        printf("[Server Connect]: got connection from (IP: %s)\n",inet_ntoa(client_addr.sin_addr));

        // 멀티 클라이언트 대응 및 메인 프로세스 대기 차단을 위해 소켓 관리 스레드 생성
        int *client_sock_ptr = (int *)malloc(sizeof(int));
        *client_sock_ptr = client_sock;

        if (client_sock_ptr == NULL) {
            perror("malloc");
            close(client_sock);
            continue;
        }

        if(pthread_create(&tid, NULL, client_handler, client_sock_ptr)!= 0){
            perror("pcreate");
            free(client_sock_ptr);
            close(client_sock);
            continue;
        }
        pthread_detach(tid);

    }
    close(server_sock);
    return 0;
}

void *client_handler(void *arg)
{
    int client_sock = *((int*)arg);
    free(arg);
    char buf[BUF_SIZE];
    int str_len;

    // 클라이언트 소켓으로부터 데이터를 계속 수신
    while((str_len = recv(client_sock, buf, sizeof(buf)-1, 0)) > 0) {
        buf[str_len] = '\0';
        
        printf("[Received Packet] 클라이언트 수신 데이터: %s\n", buf);

        // 데이터 파싱
        parse_and_execute(buf, client_sock);
    }

    printf("[Disconnect] 우분투 클라이언트 접속 종료.\n");
    close(client_sock);
    return 0;
}

void parse_and_execute(char *msg, int client_sock)
{
    char cmd[20] = {0,};
    char arg[20] = {0,};
    
    // 클라이언트가 보낸 "LED HIGH", "CDS 150" 등 문자열을 공백 기준으로 파싱함
    sscanf(msg, "%s %s", cmd, arg);

    if(strcmp(cmd, "LED") == 0) {
        char *buf = (char *)malloc(sizeof(char) * 10);
        strcpy(buf, arg); 
        pthread_t tid;
        pthread_create(&tid, NULL, led_thread, buf);
        pthread_detach(tid);
    }
    else if(strcmp(cmd, "CDS") == 0) {

        CDS_DATA *data = (CDS_DATA *)malloc(sizeof(CDS_DATA));
        data->threshold = atoi(arg);
        data->client_sock = client_sock; // 현재 소켓 저장

        pthread_t tid;
        pthread_create(&tid, NULL, cds_thread, data);
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
        pthread_create(&tid, NULL, buz_thread, buf);
        pthread_detach(tid);
    }
    else if(strcmp(cmd, "EXIT") == 0) {
        printf("[Server] 클라이언트가 원격 메뉴를 빠져나갔습니다.\n");
    }
}

void *led_thread(void *arg)
{
    void* handle;
    void (*ptr)(char*);
    char* error;

    handle = dlopen("./libdevice.so", RTLD_LAZY | RTLD_NODELETE);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        free(arg); 
        pthread_exit(NULL);
    }

    ptr = dlsym(handle, "led_function");

    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
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
        fprintf(stderr, "%s\n", dlerror());
        pthread_exit(NULL);
    }

    ptr = dlsym(handle, "cds_function");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
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
        fprintf(stderr, "%s\n", dlerror());
        free(arg);
        pthread_exit(NULL);
    }

    ptr = dlsym(handle, "seg_function");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
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
        fprintf(stderr, "%s\n", dlerror());
        free(arg);
        pthread_exit(NULL);
    }

    ptr = dlsym(handle, "buz_function");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        free(arg);
        dlclose(handle);
        pthread_exit(NULL);
    }

    ptr((char*)arg);

    free(arg); 
    dlclose(handle);
    pthread_exit(NULL);
}