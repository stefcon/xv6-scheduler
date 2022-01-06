#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, char *argv[])
{
    int alfa, preemptive;
    if (argc < 2){
        printf("Argument error\n");
        exit(-1);
    }

    if (strcmp(argv[1], "sjf") == 0) {
        if (argc != 4){
            printf("Argument error\n");
            exit(-1);
        }
        alfa = atoi(argv[2]);
        preemptive = atoi(argv[3]);
        exit(setsjf(alfa, preemptive));
    }
    else if (strcmp(argv[1], "cfs") == 0) {
        exit(setcfs());
    }
    printf("Argument error\n");
    exit(-1);
}


