#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "cachelab.h"

typedef __uint64_t uint64_t;
typedef struct {
    bool valid;
    uint64_t tag;
    uint64_t counter;
} line;
typedef line *set;
typedef set *cache;

cache getCache(int s, uint64_t E);
void releaseCache(int s, cache cac);
void cacheProcess(int s, uint64_t E,int b, FILE *file, bool verbose);
void spaceLimitExit();
void wrongParamExit();
void printSummary(int hit, int miss, int evi);

int main(int argc, char *const argv[])
{
    const char *helpMessage = "Usage: ./csim [-hv] -s <num> -E <num> -b <num>" \
    " -t <file>\n";
    const char *optString = "hvs:E:b:t:";

    uint64_t E = 0;
    int s = 0, b = 0;
    bool verbose = false;
    char param;
    FILE *file = NULL;
    extern char *optarg;

    while ((param = getopt(argc, argv, optString)) != -1) {
        switch (param) {
            case 'h': 
                printf("%s", helpMessage);
                exit(0);
            case 'v':
                verbose = true;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                file = fopen(optarg, "r");
                if (file == NULL)
                    wrongParamExit();
                break;
            default:
                wrongParamExit();
        }
    }
    
    if (s == 0 || E == 0 || b == 0 || file == NULL)
        wrongParamExit();
    
    cacheProcess(s, E, b, file, verbose);
   
    fclose(file);
    return 0;
}

cache getCache(int s, uint64_t E) {
    uint64_t S = 1 << s;
    cache cac = (set *) calloc(S, sizeof(set));
    if (cac == NULL)
        spaceLimitExit();

    for (int i = 0; i < S; i++) {
        cac[i] = calloc(E, sizeof(line));
        if (cac[i] == NULL)
            spaceLimitExit();
    }

    return cac;
}

void releaseCache(int s, cache cac) {
    uint64_t S = 1 << s; 
    for (int i = 0; i < S; i++)
        free(cac[i]);
    free(cac);
}

void cacheProcess(int s, uint64_t E,int b, FILE *file, bool verbose) {
    cache cac = getCache(s, E);
    uint64_t address = 0, setbit, tagbit, index = 0;
    char operation;
    int hit = 0, miss = 0, evi = 0, e = 64 - s - b, size = 0;
    bool hitFlag = false, blankFlag = false;

    while (fscanf(file, " %c %lx,%d", &operation, &address, &size) == 3) {
        if (operation == 'I')
            continue;
        
        if (verbose)
            printf("%c %lx,%d ", operation, address, size);

        setbit = address << e >> (e + b);
        tagbit = address >> (s + b);
        
        set st = cac[setbit];
        
        hitFlag = false;
        blankFlag = false;
        index = 0;

        for (int i = 0; i < E; i++) {
            if (!st[i].valid) {
                if (hitFlag) break;
                blankFlag = true;
                st[i].valid = true;
                st[i].tag = tagbit;
                break;
            }
            if (st[i].valid && st[i].tag == tagbit) {
                hitFlag = true;
                st[i].counter = 0;
                continue;
            }
            st[i].counter++;
        }

        if (operation == 'M')
            hit++;

        if (hitFlag) {
            if (verbose) {
               if (operation == 'M') printf("hit hit\n");
               else printf("hit\n");
            }
            hit++;
            continue;
        }

        miss++;

        if (verbose) {
            printf("miss");
            if (!blankFlag) printf(" eviction");
            if (operation == 'M') printf(" hit");
            printf("\n");
        }

        if (!blankFlag) {
            evi++;
            for (int i = 1; i < E; i++) {
                if (st[i].counter > st[index].counter)
                    index = i;
            }
            st[index].tag = tagbit;
            st[index].counter = 0;
        }
    }

    releaseCache(s, cac);
    printSummary(hit, miss, evi);
}

void spaceLimitExit() {
    printf("Not enough memory\n");
    exit(1);
}

void wrongParamExit() {
    printf("Wrong parameters!\n");
    exit(1);
}
