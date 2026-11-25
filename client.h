#ifndef CLIENT
#define CLIENT

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

void printing_online_users(int sockfd, char *name);
void *receive_thread(void* args);
void exit_handler(int num);
void to_upper(char *str);
#endif