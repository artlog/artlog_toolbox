#pragma once

void hexprint(FILE * fp,const char * buffer, int length);
void print_addrinfo(FILE * fp, struct addrinfo * addrinfo);
int display_hostent(struct hostent * hostent);
int display_address(char * h_addr, struct hostent * hostent);
