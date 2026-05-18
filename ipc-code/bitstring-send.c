#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>

#define LENGTH 8

char recvdString[9];

bool canSend = false;

volatile sig_atomic_t recvIndex = 0;

/* Signal handler for receiving bit 1 */
void sigHandle1(int sig)
{
    recvdString[recvIndex++] = '1';
}

/* Signal handler for receiving bit 0 */
void sigHandle0(int sig)
{
    recvdString[recvIndex++] = '0';
}

/* Synchronization handler */
void synchronizeParent(int sig)
{
    canSend = true;
}

int main()
{
    signal(SIGUSR1, synchronizeParent);

    int cpid = fork();

    if (cpid < 0) {
        perror("fork");
        exit(1);
    }

    /* CHILD PROCESS */
    if (cpid == 0) {

        /* Register handlers */

        signal(SIGUSR1, sigHandle1);

        signal(SIGUSR2, sigHandle0);

        /* Notify parent child is ready */
        kill(getppid(), SIGUSR1);

        /* Wait until all 8 bits received */
        while (recvIndex < LENGTH) {
            pause();
        }

        recvdString[8] = '\0';

        printf("[Child] Received bitstring is\t%s\n",
               recvdString);

        exit(0);
    }

    /* PARENT PROCESS */
    else {

        printf("Please input a %d-bit bitstring:\t",
               LENGTH);

        char tmp[256];

        fgets(tmp, LENGTH + 1, stdin);

        if (strlen(tmp) != LENGTH) {

            printf("Error : Input string not of length %d\n",
                   LENGTH);

            kill(cpid, SIGKILL);

            wait(NULL);

            exit(1);
        }

        for (int i = 0; i < LENGTH; i++) {

            if (!(tmp[i] == '1' || tmp[i] == '0')) {

                printf("Error : Input string not a bitstring at index %d char %c\n",
                       i,
                       tmp[i]);

                kill(cpid, SIGKILL);

                wait(NULL);

                exit(1);
            }
        }

        printf("[Parent] Input bitstring is \t%s\n",
               tmp);

        /* Wait until child ready */
        while (!canSend) {
            ;
        }

        /* Send bits one by one */

        for (int i = 0; i < LENGTH; i++) {

            if (tmp[i] == '1') {

                kill(cpid, SIGUSR1);
            }
            else {

                kill(cpid, SIGUSR2);
            }

            sleep(1);
        }

        wait(NULL);
    }

    return 0;
}