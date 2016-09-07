#ifndef SERVER
#define SERVER

void perform_http(int,char*);

void parse_req(char*,char*,char*,char*);

int scan_dir(char*,char*);

void read_file(char*,char*,char*);

void cleanExit(int);

#endif