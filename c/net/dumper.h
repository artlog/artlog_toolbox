#pragma once

void hexprint(FILE * fp,const char * buffer, int length);
void print_addrinfo(FILE * fp, struct addrinfo * addrinfo);
int display_hostent(FILE * fp, struct hostent * hostent);
int display_address(FILE * fp, char * h_addr, struct hostent * hostent);
