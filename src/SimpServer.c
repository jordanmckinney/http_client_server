/*------------------------------
* SimpServer.c
* Description: HTTP server program
* CSC 361
* Author: Jordan McKinney
-------------------------------*/

#define MAX_STR_LEN 256         // max string length    
#define DEFAULT_PORT_ID 8080    // default port number
#define STR_LEN 32              // typical string length
#define MAX_QUEUED_CLI 3        // max queued clients
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <sys/utsname.h>
#include "../include/server.h"
#include "../include/util.h"

 /* -------------- Main() routine ----------------*
 * four main tasks are excuted:
 * generate socket and get socket id,
 * max number of connection is 3,
 * accept request from client and generate new socket,
 * use the socket id to connect sopecified server,
 * communicate with client and close new socket after done
 *-----------------------------------------------*/
int main(int argc, char *argv[])
{
    int sock_fd, newsock_fd, port_num;
    struct sockaddr_in serv_addr, cli_addr;
    char folder[STR_LEN];
    socklen_t cli_len;

    // get port number and resource directory from server admin
    if (argc < 3) {
   		fprintf(stderr,"Specify port number and directory name\n");
   		exit(1);
 	}
    port_num = atoi(argv[1]);
 	strcpy(folder,argv[2]);
    folder[sizeof(argv[2])] = '\0';

    // create listen socket
    if((sock_fd=socket(AF_INET,SOCK_STREAM,0))<0){
    	fprintf(stderr,"Error opening socket\n");
        cleanExit(sock_fd);
    }
	
    // set server address fields
	bzero( (char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_num);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // bind to socket
   	if (bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    	fprintf(stderr,"Error binding socket\n");
        cleanExit(sock_fd);
    }

    // start serve loop: listen for connections, accept client connection
    // read request, serve response, close socket; repeat
    while(1){

        // listen for clients 
    	listen(sock_fd,MAX_QUEUED_CLI);

        // accept client connection
        cli_len = sizeof(cli_addr);
        if((newsock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &cli_len))<0){
        	fprintf(stderr,"Error on accept\n");
            cleanExit(newsock_fd);
        }

        // read client request, serve response, then close socket
        perform_http(newsock_fd,folder);
        close(newsock_fd);
    }

    cleanExit(sock_fd);
    return 0;
}

/*-----------------------------------------------*
 * accepts a request from "sockid" and sends a response to "sockid"
 *-----------------------------------------------*/
void perform_http(int sockid, char* folder)
{
    char sendline[MAX_STR_LEN], recvline[MAX_STR_LEN];
    char method[STR_LEN], resource[STR_LEN], version[STR_LEN];
    char sysname[STR_LEN], timestr[STR_LEN];
    char fullpath[STR_LEN+2]="./";
    int filefound=0;

    // clear sendline and recvline then read contents from client
    bzero(recvline,sizeof(recvline));
    bzero(sendline,sizeof(sendline));
    read(sockid,recvline,sizeof(recvline));

    // parse client request
    parse_req(recvline,method,resource,version);

    // get system info, save to sysname
    struct utsname unameData;
    uname(&unameData);
    sprintf(sysname,"%s", unameData.sysname);
    
    // get current time, save to timestr
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(timestr,"Date: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, 
        tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec); 

    // prepend folder with './'
    strcat(fullpath,folder);

    // verify request method and version are supported, else 501 error
    if(strcmp(method,"GET")!=0 || strcmp(version,"HTTP/1.0")!=0){
        strcat(sendline, "HTTP/1.0 501 Not Implemented\r\n");  
    }
    // verify requested file exists, else 404 error
    else if((filefound=scan_dir(fullpath,resource))!=1){
        strcat(sendline, "HTTP/1.0 404 Not Found\r\n");
    }else
        strcat(sendline, "HTTP/1.0 200 OK\r\n");
    
    // concatenate timestamp and system name to response, end header
    strcat(sendline, timestr);
    strcat(sendline, "Server: ");
    strcat(sendline, sysname);
    strcat(sendline,"\r\n\r\n");
    
    // if file found, concatenate contents to response
    if(filefound==1)
        read_file(fullpath,resource,sendline);

    // send response to client
    writen(sockid,sendline,sizeof(sendline));    
}

/*-----------------------------------------------*
 * parse client request into HTTP method, requested
 * resource, and HTTP version
 *-----------------------------------------------*/
void parse_req(char *recvline, char *method, char *resource, char *version)
{
    char *meth_end, *res_strt, *res_end, *ver_strt, *ver_end;
    int meth_sz, res_sz, ver_sz;
    
    // find method start and end in recvline, copy to method
    meth_end = strchr(recvline,' ');
    meth_sz = meth_end-recvline;
    strncpy(method,recvline,meth_sz);
    method[meth_sz] = '\0';

    // find resource start and end in recvline, copy to resource
    res_strt = strchr(recvline,'/')+1;
    res_end = strchr(res_strt,' ');
    res_sz = res_end-res_strt;
    strncpy(resource,res_strt,res_sz);
    resource[res_sz] = '\0';

    // find version start and end in recvline, copy to version
    ver_strt = res_end+1;
    ver_end = strstr(ver_strt,"\r\n");
    ver_sz = ver_end-ver_strt;
    strncpy(version,ver_strt,ver_sz);
    version[ver_sz] = '\0';
}

/*-----------------------------------------------*
 * checks directory fullpath for resource
 *-----------------------------------------------*/
int scan_dir(char *fullpath, char *resource)
{
    struct dirent **namelist;
    int n;

    // scandir returns number of items in path, -1 if path not valid
    n = scandir(fullpath, &namelist, NULL, alphasort);
    if (n < 0)
        return 0;
    else{
        while (n--) {
            // check whether resource name matches any items in path
            if((strcmp(resource,namelist[n]->d_name))==0)
                return 1;
        }
        free(namelist);
        return 0;
    }
}

/*-----------------------------------------------*
 * opens file specified by resource and reads contents
 * into sendline
 *-----------------------------------------------*/
void read_file(char *fullpath, char *resource, char *sendline)
{
    // make copy of fullpath for modification
    char newpath[sizeof(fullpath)+2]; // try +1
    strcpy(newpath,fullpath);

    strcat(fullpath,"/");
    strcat(fullpath,resource);

    // open file specified by fullpath
    FILE *f = fopen(fullpath, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  

    // read contents into string
    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    // append file contents into sendline
    strcat(sendline,string);
}

/*-----------------------------------------------*
 * cleans up opened sockets when killed by a signal
 *-----------------------------------------------*/
void cleanExit(int sockid)
{
    close(sockid);
    exit(0);
}












