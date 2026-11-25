#include "server.h"

details * head = NULL;

online_list * o_head = NULL;

int main()
{
    read_data_to_list();
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr  = inet_addr(SERVER_ID);
    if(bind(sockfd,(struct sockaddr *)&addr,sizeof(addr)) == -1)
    {
        perror("bind");
        return -1;
    }

    listen(sockfd,100);
    printf("SERVER IS WAITING.......\n");
    signal(SIGINT,exit_handle);
    while(1)
    {
        struct sockaddr_in c_addr;
        socklen_t len = sizeof(c_addr);
        int *datafd = malloc(sizeof(int));
        *datafd = accept(sockfd,(struct sockaddr *)&c_addr,&len);
        pthread_t tid;
        pthread_create(&tid,NULL,client_fun,datafd); 
        pthread_detach(tid);  
    }
    
}
