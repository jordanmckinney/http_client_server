/*------------------------------
* SimpClient.c
* Description: HTTP client program
* CSC 361
* Author: Jordan McKinney
-------------------------------*/

#define MAX_STR_LEN 256         // max string length
#define MAX_RES_LEN 5000        // max response length
#define DEFAULT_PORT_ID 80      // default port number
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include "../include/client.h"
#include "../include/util.h"

/* -------------- Main() routine ----------------*
 * three main tasks are excuted:
 * accept the input URI and parse it into fragments for further operation,
 * open socket connection with specified sockid ID,
 * use the socket id to connect sopecified server
 *-----------------------------------------------*/
int main()
{
    char uri[MAX_STR_LEN];
    char hostname[MAX_STR_LEN];
    char identifier[MAX_STR_LEN];
    int sock_id, port;

    printf("Open URI: ");
    scanf("%s", uri);

    // parse inputted URI
    parse_URI(uri, hostname, &port, identifier);
    
    // open connection
    sock_id = open_connection(hostname, port);
    
    // perform HTTP request, receive response
    perform_http(sock_id, identifier, hostname);
    return 0; 
}

/*-----------------------------------------------*
 * parse URI into hostname, resource and identifier
 *-----------------------------------------------*/
void parse_URI(char *uri, char *hostname, int *port, char *identifier){
    char *prot_end;
    char proto[8];
    char *host_strt, *host_end;
    char *port_strt;
    char *id_strt;
    
    // find protocol end and host start in uri
    prot_end=strstr(uri,"://");
    host_strt=prot_end+3;
    strncpy(proto,uri,prot_end-uri);

    // check for supported protocol
    if(strcmp(proto,"http")!=0){
        fprintf(stderr, "Error, only http supported\n");
        exit(0);
    }

    // find id start, and set host end
    id_strt=strchr(uri+(host_strt-uri),'/');
    host_end=id_strt;
    
    // set port to default value
    *port=DEFAULT_PORT_ID;
    
    // if port was specified override default, adjust host end
    // handles ...:PORT/... and ...[:PORT]/... formats
    if(strchr(uri+(host_strt-uri),':')){
        host_end=strchr(uri+(host_strt-uri),':');
        port_strt=host_end+1;
        if(*(host_end-1)=='[')
            host_end--;
        *port=atoi(port_strt);
    }

    // copy hostname and id
    strncpy(hostname,host_strt,host_end-host_strt);
    hostname[host_end-host_strt]='\0';
    strcpy(identifier,id_strt);
}

/*-----------------------------------------------*
 * connects to a remote server on a specified port
 *-----------------------------------------------*/
int open_connection(char *hostname, int port)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int sock_fd;

    // create socket
    if((sock_fd=socket(AF_INET, SOCK_STREAM, 0))<0){
        fprintf(stderr, "Error opening socket\n");
        exit(0);
    }
    
    // find IP from hostname
    server=gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"Hostname look-up failed\n");
        exit(0);
    }

    // set server address fields
    bzero( (char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(port);

    // open connection
    if(connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0){
        fprintf(stderr,"Error connecting\n");
        exit(0);
    }
    return sock_fd;
}

/*-----------------------------------------------*
 * connects to a HTTP server using hostname and port,
 * and get resource specified by identifier
 *-----------------------------------------------*/
void perform_http(int sock_id, char *identifier, char *hostname)
{
    char sendline[MAX_STR_LEN];
    char recvline[MAX_RES_LEN];
    char header[MAX_RES_LEN];
    char *head_end;

    // clear sendline then read in GET request
    bzero(sendline,sizeof(sendline));
    sprintf(sendline, "GET %s HTTP/1.0\r\n\r\n",identifier);
    printf("\n---Request begin---\n");

    // send request
    writen(sock_id,sendline,sizeof(sendline));

    // print request details
    printf("GET %s HTTP/1.0\n",identifier);
    printf("Host: %s\n\n", hostname);
    printf("---Request end---\n");
    printf("HTTP request sent, awaiting response...\n\n");
    
    // clear recvline then copy response to recvline
    bzero(recvline, sizeof(recvline));
    readn(sock_id,recvline,sizeof(recvline));

    // find and print header
    head_end = strstr(recvline,"\r\n\r\n");
    printf("---Response header---\n");
    strncpy(header,recvline,head_end-recvline);
    header[head_end-recvline]='\0';
    printf("%s\n\n", header);

    // print body
    printf("--- Response body ---\n");
    printf("%s\n",head_end+4);

    close(sock_id);
}