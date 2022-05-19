#include <stdio.h>
#include <string.h>

// multiprocessing includes
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>

// Time included for testing execution time
#include <time.h>

#include "header_files/threader.h"

#include "header_files/objects.h"
// #include "header_files/semaphores.h"

int LOOP_CON = 1;

void signal_handler(int sig);

int main(int argc, char *argv[]) {

    int sig_val = 1;

    struct sigaction sig = { 0 };
    sig.sa_handler = &signal_handler;
    sig.sa_flags = SA_RESTART;
    sig_val = sigaction(SIGUSR1, &sig, NULL);

    // for (int i = 0; i < argc; i++) {
    //     printf("Arguments list %d = %s\n", i, argv[i]);
    // }
    if (strcmp(argv[0], "process_1") == 0) {
        printf("Process_1 identified.Printing process argv[0] : %s\n", argv[0]);
    }

    while (!sig_val) {
        sleep(1);
        if (LOOP_CON) {
            printf("Signal received from the child process!\n");
            LOOP_CON = 0;
        }
    }
    return 0;
}

void signal_handler(int sig) {
    LOOP_CON = 1;
}
