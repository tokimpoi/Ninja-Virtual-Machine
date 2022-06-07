#ifndef NJVM_H
#define NJVM_H

#define MAX 10000

#include <stdbool.h>

typedef struct {
unsigned int size; /* byte count of payload data */
unsigned char data [1]; /* payload data , size as needed */
} *ObjRef;

typedef struct {
bool isObjRef ; /* slot used for object reference ? */
union {
ObjRef objRef; /* used if isObjRef=TRUE */
int number; /* used if isObjRef=FALSE */
} u;
} StackSlot ;

extern unsigned int *programm_speicher;
extern StackSlot stack[MAX];
extern int sp;
extern int pc;
extern int pcSize;
extern int fSize;
extern int fp;
extern ObjRef *sda;
extern int sdaSize;
extern int runBool;
extern int quit;
extern ObjRef returnRegister;

void run();
void print();

#endif
