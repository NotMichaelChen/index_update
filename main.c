#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>

int main(int argc, char **argv) {
    char k = -1;
    char *oldfile, *newfile;
    bool isold = false, isnew = false;
    
    int opt;
    while((opt = getopt(argc, argv, "k:o:n:")) != -1) {
        switch(opt) {
        case 'k':
            k = *optarg;
            break;
        case 'o':
            oldfile = optarg;
            isold = true;
            break;
        case 'n':
            newfile = optarg;
            isnew = true;
            break;
        }
    }
    
    int blocks = 0;
    if(isdigit(k)) {
        blocks = k - '0';
    }
    else {
        printf("Error: Invalid number of blocks specified\n");
        exit(EXIT_FAILURE);
    }
    
    if(!isold) {
        printf("Error: Old file not specified\n");
        exit(EXIT_FAILURE);
    }
    
    if(!isnew) {
        printf("Error: New file not specified\n");
        exit(EXIT_FAILURE);
    }
    
    printf("%d\n", blocks);
    printf("%s\n", oldfile);
    printf("%s\n", newfile);
    return 0;
}