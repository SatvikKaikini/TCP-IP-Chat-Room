#include "client.h"

cred credentials;
int sockfd;
int flag;
int main()
{
    // cred credentials;
    flag = 1;
    printf("=========== HOME MENU ===========\n");
    printf("1. LOGIN\n2. REGISTER\n"); 
    printf("Enter the option:");
    do
    { 
        __fpurge(stdin);
        scanf("%d",&credentials.option);
        if(credentials.option != 1 &&  credentials.option != 2)
        {
            printf(RED"‼️ ..... Invalid Option ..... ‼️\n🔄️ Reenter the option:"RESET);
        }
    } while (credentials.option != 1 && credentials.option != 2);
    
    __fpurge(stdin);
    printf("Enter the USER NAME:");
    scanf("%[^\n]",credentials.user_name);
    __fpurge(stdin);
    printf("Enter the PASSWORD:");
    scanf("%[^\n]",credentials.password);

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr  = inet_addr(SERVER_ID);
    if(connect(sockfd,(struct sockaddr *)&addr,sizeof(addr)) != 0)
    {
        printf(RED"🌐 ...Connection failed... 🌐\n"RESET);
        return -1;
    }
    printf(GREEN"🛜 .....Connected to the server..... 🛜\n"RESET);
    send(sockfd,&credentials,sizeof(credentials),0);
    // printf("Sent\n");
    // recv(sockfd,ack_message,sizeof(ack_message),0);
    // printf("Rec\n");
    // printf("%s\n",ack_message);
    // if(strcmp(ack_message,"SUCCESS") == 0)
    // {
    //     strcpy(ack_message,"success");
    //     send(sockfd,ack_message,strlen(ack_message)+1,0);
    // }
    // else
    // {
    //     strcpy(ack_message,"fail");
    //     send(sockfd,ack_message,strlen(ack_message)+1,0);
    // }
    // if(strcmp(ack_message,"SUCCESS") != 0)
    // {
    //     return -1; 
    // }
    signal(SIGINT,exit_handler);
    pthread_t tid;
    pthread_create(&tid,NULL,receive_thread,&sockfd);
    
    while(1)
    {
        usleep(10000);
        printf("=========== CHAT MENU ===========\n");
        printf("1. Single Chat\n2. Group Chat\n3. Exit\n");
    // printf("Enter the option:");
    int num = 0;
    printf("Enter the option:");
    do
    {
        num = 0;
        fflush(stdout);
        __fpurge(stdin);
        scanf("%d",&num);
        if(num != 1 && num != 2 && num != 3)
        printf(RED"‼️ ..... Invalid Option ..... ‼️\n🔄️ Reenter the option:"RESET);
        
    } while (num != 1 && num != 2 && num != 3);
    // printing_online_users(sockfd,credentials.user_name);
    flag = 2;
    packet pck;
    if(num == 1)
    {

        memset(&pck,0,sizeof(pck));
        pck.type = SINGLE_CHAT;
        strcpy(pck.sender,credentials.user_name);
        printf("Enter the Name:");
        fflush(stdout);
        __fpurge(stdin);
        scanf("%[^\n]",pck.receiver); 
        printf("Enter the message:");
        fflush(stdout);
        __fpurge(stdin);
        scanf("%[^\n]",pck.message);
        send(sockfd,&pck,sizeof(pck),0);   
    }
    else if(num == 2)
    {
        memset(&pck,0,sizeof(pck));
        pck.type = GROUP_CHAT;
        strcpy(pck.sender,credentials.user_name);
        // printf("Enter the message:");
        
            printf("Enter the message:");
            fflush(stdout);
            __fpurge(stdin);
            scanf("%[^\n]",pck.message);
            send(sockfd,&pck,sizeof(pck),0);
        
    }
    else if(num == 3)
    {
        // printf("EXIT CLEN\n");
        memset(&pck,0,sizeof(pck));
        pck.type = EXIT;
        strcpy(pck.sender,credentials.user_name);
        int ret = send(sockfd,&pck,sizeof(pck),0);
        exit(0);
        // printf("%d -> send ret\n",ret);
    }
}
    // printf("Hello\n");
}

void *receive_thread(void* args)
{
    int sockfd = *((int *)args);
    // free(args);
    packet pck;
    while(1)
    {
        memset(&pck,0,sizeof(pck));
        int ret = recv(sockfd,&pck,sizeof(pck),0);
        if(ret <= 0)
        { 
            break;   
        }
        if(pck.type == SINGLE_CHAT  )
        {
            to_upper(pck.sender);
            printf(YELLOW"\nPrivate Message from %s :"RESET,pck.sender);
            printf(BLUE" %s\n"RESET,pck.message);       
            fflush(stdout);
            
        }
        else if( pck.type == GROUP_CHAT )
        {
            to_upper(pck.sender);
            printf(YELLOW"\nGroup message from %s :"RESET,pck.sender);
            printf(BLUE" %s\n"RESET,pck.message);
            fflush(stdout);
            
        }

        else if(pck.type == ONLINE_LIST)
        {
            if(strcmp(pck.message,"null") != 0)
            {
                to_upper(pck.message);
                printf(MAGENTA"\n... %s is online ...\n"RESET,pck.message); 
            }
            fflush(stdout);
        } 
        else if(pck.type == ERROR_MSG)
        {
            if(strcmp(pck.message,"SUCCESS")== 0)
            {
                printf("✅ .....Validation Success..... ✅\n");
            }
            else
            {
            printf(RED"\n🚫  .....%s.....🚫  \n"RESET,pck.message);
            fflush(stdout);
            exit(0);
            }
        }
        else if(pck.type == NO_ONLINE)
        {
            printf(RED"\n✖️    .....%s..... ✖️  \n"RESET,pck.message);
            fflush(stdout);
        }
        else if (pck.type == EXIT)
        {
            to_upper(pck.sender);
            printf(RED"\n... %s is offline ...\n"RESET,pck.sender);
            fflush(stdout);
        }
        else if(pck.type == SER_CRASH)
        {
            printf(RED"\n‼️  .....SERVER CRASHED.....‼️ \n"RED);
            signal(SIGINT,SIG_DFL);
            kill(getpid(),SIGINT);
        }
    } 
}

void exit_handler(int num)
{
    packet pck;
    memset(&pck,0,sizeof(pck));
    pck.type = EXIT;
    strcpy(pck.sender,credentials.user_name);
    send(sockfd,&pck,sizeof(pck),0);
    signal(SIGINT,SIG_DFL);
    usleep(1000);
    kill(getpid(),SIGINT);
}


void to_upper(char *str)
{
    for(int i=0;str[i];i++)
    {
        if(str[i] >= 'a' && str[i] <= 'z')
        {
            str[i] = str[i] - 32;
        }
    }
}