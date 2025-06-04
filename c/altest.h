#ifndef __ALTEST_H__
#define __ALTEST_H__

#include <stdio.h>
#include <sys/resource.h>
#include <time.h>

int checktest(FILE * out, char* test, int result, time_t * start);

void show_memory_usage();

#endif // #ifndef __ALTEST_H__
