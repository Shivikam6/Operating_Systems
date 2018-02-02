
#ifndef UTILS_H
#define UTILS_H

#define TRUE 1
#define FALSE 0

#define LOG 0
#define NOTIFY 1
#define ERROR 2

extern int ERRORflag;

void print(int display, char const* str);


char* cat(char* s1,char* s2);
/* reverse:  reverse string s in place */

char* itoa(int n);

char *pad(char *value);


#endif