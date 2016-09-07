#ifndef CLIENT
#define CLIENT

void parse_URI(char*,char*,int*,char*);

void perform_http(int,char*,char*);

int open_connection(char*,int);

#endif