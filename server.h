#ifndef SERVER
#define SERVER

#include <stdio.h>  
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

typedef struct 
{
    char user_name[30];
    char password[30];
    int option;
}cred;

typedef struct data
{
    char user_name[30];
    char password[30];
    struct data *link;
}details;

typedef struct online_list
{
    char user_name[30];
    int sockfd;
    struct online_list *link;
}online_list;

typedef struct 
{
    char sender[30];
    char receiver[30];
    char message[100];
    int type;   
}packet;

#define SERVER_ID "127.0.0.1"
#define PORT 3500

#define ONLINE_LIST 1
#define SINGLE_CHAT 2
#define GROUP_CHAT 3
#define ERROR_MSG 4
#define EXIT 5
#define NO_ONLINE 6
#define SER_CRASH 7

#define RED         "\033[1;3;31m"
#define GREEN       "\033[1;32m"
#define YELLOW      "\033[0;3;33m"
#define BLUE        "\033[1;3;34m"
#define MAGENTA     "\033[1;3;35m"
#define CYAN        "\033[1;3;36m"
#define RESET       "\033[0m"

void *client_fun(void *arg);
void read_data_to_list();
void write_data_to_file();
int search_user_password(char * password, char *name);
int search_user_name(char *name);
void insert_online_user_to_list(char *name,int fd);
void printing_online(int datafd ,char *name);
void print_online_users_to_all_clients(char *name);
int search_online(char *name,char *sender);
void insert_at_first(char *user_name,char* password);
void print_offline_users_to_all_clients(char *name);
void delete_from_online_list(char *name);
void exit_handle(int signum);
void print_server_crash_to_all_clients(void);
#endif
