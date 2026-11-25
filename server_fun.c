#include "server.h"

#define SUCCESS 1
#define FAILURE 0

extern details * head;
extern online_list * o_head;
pthread_mutex_t online_mutex = PTHREAD_MUTEX_INITIALIZER;
void *client_fun(void *arg)
{
    int datafd = *((int *)arg) ;
    free(arg);
    // printf("data fd = %d\n",datafd);
    cred credentials;
    // char ack_message[50];
    packet pck;
    memset(&pck,0,sizeof(pck));
    recv(datafd,&credentials,sizeof(credentials),0);
    // printf("USER NAME = %s\nPassword = %s\nOption = %d\n",credentials.user_name,credentials.password,credentials.option);
    if(credentials.option == 1) // LOGIN
    {
        // printf("LOGIN\n");
        if(search_user_name(credentials.user_name) == SUCCESS)
        {
            if(search_online(credentials.user_name,credentials.password) == -1)
            {
            if(search_user_password(credentials.password,credentials.user_name) == SUCCESS)
            {
                printf("%s Connected to the server...\n",credentials.user_name);
                strcpy(pck.message,"SUCCESS");
                pck.type = ERROR_MSG;
                send(datafd,&pck,sizeof(pck),0);
                print_online_users_to_all_clients(credentials.user_name);
                insert_online_user_to_list(credentials.user_name,datafd);
                printing_online(datafd,credentials.user_name);
            }
            else
            {
                // printf("VALIDATION PASSWORD FAILED\n");
                strcpy(pck.message,"INVALID PASSWORD");
                pck.type = ERROR_MSG;
                send(datafd,&pck,sizeof(pck),0);
            }
        }
        else
        {
            strcpy(pck.message,"USER ALREADY IN LOGIN");
                pck.type = ERROR_MSG;
                send(datafd,&pck,sizeof(pck),0);

        }
        } 
        else
        {
            // printf("VALIDATION USERNAME FAILED\n");
            strcpy(pck.message,"INVALID USER");
            pck.type = ERROR_MSG;
            send(datafd,&pck,sizeof(pck),0); 
        }
    }
    else if(credentials.option == 2)
    {
        if(search_user_name(credentials.user_name) == FAILURE)
        {
            // printf("REGISTER\n");
            insert_at_first(credentials.user_name,credentials.password);
            write_data_to_file();
            // printf("REGISTRATOIN SUCCESS\n");
            strcpy(pck.message,"SUCCESS");
            // insert_online_user_to_list(credentials.user_name,datafd);
            pck.type = ERROR_MSG;
            send(datafd,&pck,sizeof(pck),0); 
            print_online_users_to_all_clients(credentials.user_name);
            insert_online_user_to_list(credentials.user_name,datafd);
            printing_online(datafd,credentials.user_name);
        }
        else
        {
            // printf("USER exixts already\n");
            // printf("VALIDATION USERNAME FAILED\n");
            strcpy(pck.message,"USER ALREADY EXISTS");
            pck.type = ERROR_MSG;
            send(datafd,&pck,sizeof(pck),0); 
        }
    }
    // printing_online(datafd);
    while(1)
    {
        packet pck;
        memset(&pck,0,sizeof(pck));
        int ret = recv(datafd,&pck,sizeof(pck),0);
        if(ret <= 0)
        {
            close(datafd);
            pthread_exit(NULL);
        }
        // printf("RECV ret = %d\n",ret);
        if(pck.type == SINGLE_CHAT)
        {
            int fd = search_online(pck.receiver,pck.sender);
            if(fd != -1)
            {
                send(fd,&pck,sizeof(pck),0);
            }
            else
            {
                memset(&pck,0,sizeof(pck));
                strcpy(pck.message,"PERSON NOT IN ONLINE / YOU CANNOT SEND MESSAGE TO YOU");
                pck.type = NO_ONLINE;
                send(datafd,&pck,sizeof(pck),0);
            }
        }
        else if(pck.type == GROUP_CHAT)
        {
            pthread_mutex_lock(&online_mutex);
            online_list *temp = o_head;
            while(temp != NULL)
            {
                if(strcmp(temp -> user_name,pck.sender) == 0)
                {
                    temp = temp -> link;
                    continue;
                }
                send(temp ->sockfd,&pck,sizeof(pck),0);
                temp = temp -> link;
            }
            pthread_mutex_unlock(&online_mutex);
        }
        else if(pck.type == EXIT)
        {
            delete_from_online_list(pck.sender);
            print_offline_users_to_all_clients(pck.sender);
            close(datafd);
            pthread_exit(NULL);  
        }
    }
    close(datafd);
    // message data;
    // memset(&data,0,sizeof(data));
    // recv(sockfd,&data,sizeof(data),0);
    // if(data.option == 1)
    // {
        
    // }
    // else if(num == 2)
    // {
    //     printf("Group\n");
    // }
    // else if(num == 3)
    // {
    //     printf("Exit\n");
    // }
    // close(datafd);
}

void exit_handle(int signum)
{
    print_server_crash_to_all_clients();
    signal(SIGINT,SIG_DFL);
    usleep(1000);
    kill(getpid(),SIGINT);
}


void read_data_to_list()
{
    details * new = malloc(sizeof(details));
    strcpy(new->user_name,"list_end");
    strcpy(new->password,"list_end");
    new->link = NULL;
    head = new;
    FILE *fptr = fopen("details.txt","r");
    if(fptr == NULL)
    {
        printf("FILE OPENING FAILED\n");
        exit(-1);
    }
    char name[30];
    char password[30];
    while(fscanf(fptr,"%[^;];%[^\n]\n",name,password) != EOF)
    {
        // printf("NAME= %s\nPASS = %s\n",name,password);
        insert_at_first(name,password);
    }
    
    details *temp = head;
    while(temp != NULL)
    {
        // printf("%s -- %s\n", temp->user_name,temp->password);
        temp = temp->link;
    }
}

void insert_at_first(char *user_name,char* password)
{
    details *temp;
    details * new = malloc(sizeof(details));
    if(new == NULL)
    {
        printf("Memory not allocated\n");
        return;
    }
    strcpy(new->user_name,user_name);
    strcpy(new->password,password);
    new->link = head;
    head = new; 
}

void write_data_to_file()
{
    FILE* fptr = fopen("details.txt", "w");
    details *temp = head;
    while(temp != NULL)
    {
        if(strcmp(temp->user_name,"list_end") != 0)
        fprintf(fptr,"%s;%s\n",temp->user_name,temp->password);
        temp = temp->link;
    }
    fclose(fptr);
}

int search_user_name(char *name)
{
    details *temp = head;
    if(head == NULL)
    {
        return FAILURE;
    }
    while(temp != NULL)
    {
        if(strcasecmp(temp->user_name,name) == 0)
        {
            return SUCCESS;
        }
        temp = temp->link;
    }
    return FAILURE;
}

int search_user_password(char * password, char *name)
{
    details *temp = head;
    if(head == NULL)
    {
        return FAILURE;
    }
    while(temp != NULL)
    {
        if((strcasecmp(temp->password,password) == 0) && (strcasecmp(temp->user_name,name) == 0))
        {
            return SUCCESS;
        }
        temp = temp->link;
    }
    return FAILURE;
}


void insert_online_user_to_list(char *name,int fd)
{
    pthread_mutex_lock(&online_mutex);
    online_list *new = malloc(sizeof(online_list));
    strcpy(new -> user_name,name);
    new -> sockfd = fd;
    new->link = NULL;
    if(o_head == NULL)
    {
        o_head = new;
    }
    else
    {
        new->link = o_head;
        o_head = new;
    }
    pthread_mutex_unlock(&online_mutex);
}

void printing_online(int datafd,char *name)
{
    packet pck;
    pthread_mutex_lock(&online_mutex);
    online_list *temp = o_head;
    while(temp != NULL)
    {
        if(strcmp(temp->user_name,name) == 0)
        {
            temp = temp-> link;
            continue;
        }
        memset(&pck,0,sizeof(pck));
        pck.type = ONLINE_LIST;
        strcpy(pck.message,temp->user_name);
        send(datafd,&pck,sizeof(pck),0);
        temp= temp->link;
    }
    pthread_mutex_unlock(&online_mutex);
    memset(&pck,0,sizeof(pck));
    strcpy(pck.message,"null");
    send(datafd,&pck,sizeof(pck),0);
}

int search_online(char *name,char *sender)
{
    pthread_mutex_lock(&online_mutex);
    online_list *temp = o_head;
    while(temp != NULL)
    {
        
        if((strcmp(temp -> user_name,name) == 0) && (strcmp(temp -> user_name,sender) != 0) )
        {
            pthread_mutex_unlock(&online_mutex);
            return temp -> sockfd;
        }
        temp = temp -> link;
    }
    pthread_mutex_unlock(&online_mutex);
    return -1;
}

void print_online_users_to_all_clients(char *name)
{
    pthread_mutex_lock(&online_mutex);
    online_list *temp = o_head;
    packet pck;
    while(temp != NULL)
    {
        memset(&pck,0,sizeof(pck));
        pck.type = ONLINE_LIST;
        strcpy(pck.message,name);
        send(temp->sockfd,&pck,sizeof(pck),0);
        temp = temp->link;
    }
    pthread_mutex_unlock(&online_mutex);
}

void delete_from_online_list(char *name)
{
    // printf("DELETE\n");
    pthread_mutex_lock(&online_mutex);
    online_list *temp = o_head;
    online_list *prev = NULL;
    
    while(temp != NULL)
    {
        if(strcmp(temp -> user_name,name) == 0)
        {
            if(prev == NULL)
            {
                o_head = temp -> link;
            }
            else
            {
                prev -> link = temp-> link;   
            }
            printf("%s is Disconnected with server...\n",name);
            free(temp);
            break;
        }
        else
        {
            prev = temp;
            temp = temp -> link;   
        }

    }
    pthread_mutex_unlock(&online_mutex);
    // printf("DELETE\n");
}

void print_offline_users_to_all_clients(char *name)
{
    // printf("PRINT OFFLINE FUNCTION\n");
    pthread_mutex_lock(&online_mutex);
    online_list *temp = o_head;
    packet pck;
    while(temp != NULL)
    {
        memset(&pck,0,sizeof(pck));
        pck.type = EXIT;
        strcpy(pck.sender,name);
        send(temp->sockfd,&pck,sizeof(pck),0);
        temp = temp->link;
    }
    pthread_mutex_unlock(&online_mutex);
}

void print_server_crash_to_all_clients(void)
{
    // printf("PRINT OFFLINE FUNCTION\n");
    pthread_mutex_lock(&online_mutex);
    online_list *temp = o_head;
    packet pck;
    while(temp != NULL)
    {
        memset(&pck,0,sizeof(pck));
        pck.type = SER_CRASH;
        // strcpy(pck.message,);
        send(temp->sockfd,&pck,sizeof(pck),0);
        temp = temp->link;
    }
    pthread_mutex_unlock(&online_mutex);
}

