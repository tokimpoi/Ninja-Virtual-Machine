#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "operations.h"
#include "execute.h"
#include "prog.h"
#include "njvm.h"
#include "debug.h"

const int VERSION = 3;

unsigned int *program_memory;
int stack[MAX];
int sp = 0;
int pc = 0;
int pcSize = 0;
int fSize = 0;
int fp;
int *sda;
int sdaSize;
int runBool = 0;
int debugBool = 0;
int quit = 0;
FILE * filepointer;

void print() {
    pc = 0;
    unsigned int ir; 
    while(1) {
        ir = program_memory[pc];
        pc = pc + 1;
        listProg(ir, pc - 1);
        if(ir >> 24 == HALT) break;
    }
}

void run() {
    pc = 0;
    unsigned int ir; 
    while(1) {
        ir = program_memory[pc];
        pc = pc + 1;
        if(debugBool == 1) debugRun(ir);
        else { execute(ir); }
        if(ir >> 24 == HALT || quit == 1) break;
    } 
}

void readBin(char * filename) {//FILE *filepointer
    
    if((filepointer = fopen(filename, "r")) == NULL) {
        printf("Error: cannot open code file '%s'\n", filename);
        exit(25);
    }

    int read_len = 0;
    char c[4];
    read_len = fread(c, 1, 4, filepointer);
    if(c[0] != 'N' || c[1] != 'J' || c[2] != 'B' || c[3] != 'F') {
        exit(25);
    }

    read_len = fread(c, 1, 4, filepointer);
    if(c[0] != VERSION) {
        exit(25);
    }

    unsigned int n;
    read_len = fread(&n, 1, 4, filepointer);
    pcSize = SIGN_EXTEND(IMMEDIATE(n));
    program_memory = malloc(pcSize*sizeof(unsigned int));
    if(program_memory == NULL) {
        exit(-1);
    }

    read_len = fread(&n, 1, 4, filepointer);
    sdaSize = SIGN_EXTEND(IMMEDIATE(n));
    sda = malloc(sdaSize*sizeof(int));
    if(sda == NULL) {
        exit(-1);
    }

    pc = 0;
    unsigned int ir;
    while((read_len = fread(&ir, sizeof(unsigned int), 1, filepointer)) != 0) {
        program_memory[pc] = ir;
        pc++;
    }
}

int main(int argc, char** argv) {
    if (argc > 1) {
        int i;
        for(i = 1; i < argc; i++) {
            //If argv starts with --
            if(argv[i][0] == '-' && argv[i][1] == '-') {
                if (strcmp(argv[i],"--help") == 0) {
                    printf("Usage: ./njvm [options] <code file>\n");
                    printf("Options:\n");
                    printf("  --debug          start virtual machine in debug mode\n");
                    printf("  --version        show version and exit\n");
                    printf("  --help           show this help and exit\n");
                    return 0;
                } else if (strcmp(argv[i],"--version") == 0) {
                    printf("Ninja Virtual Machine version %d (compiled %s, %s)\n", VERSION,__DATE__,__TIME__);
                    return 0;
                } else if(strcmp("--debug", argv[i]) == 0) {
                    debugBool = 1;
                } else {
                    printf("unknown command line argument '%s', try '%s --help'\n",argv[i],argv[0]);
                    return 1;
                }
            } 
            else {
                readBin(argv[i]);
            }
        }
    }
    
    if(filepointer == NULL) {
                printf("Error: no code file specified\n");
                return 1;
            }

    printf("Ninja Virtual Machine started\n");
    run();
    printf("Ninja Virtual Machine stopped\n");
        
    free(program_memory);
    free(sda);

    return 0;
}