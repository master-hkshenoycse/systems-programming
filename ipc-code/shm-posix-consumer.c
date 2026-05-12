#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHM_NAME "OS"
#define SIZE 4096
#define SLOT_SIZE 8
#define NUM_SLOTS (SIZE / SLOT_SIZE)
#define ITERATIONS 1000

#define FIFO_P2C "/tmp/prod_to_cons"
#define FIFO_C2P "/tmp/cons_to_prod"

const char FREE_SLOT[SLOT_SIZE] = "freeeee";

int main()
{
    int shm_fd;
    char *ptr;

    /* Open shared memory */
    shm_fd = shm_open(SHM_NAME,
                      O_RDWR,
                      0666);

    if (shm_fd < 0) {
        perror("shm_open");
        exit(1);
    }

    ptr = mmap(0,
               SIZE,
               PROT_READ | PROT_WRITE,
               MAP_SHARED,
               shm_fd,
               0);

    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    int fd_p2c = open(FIFO_P2C, O_RDONLY);
    int fd_c2p = open(FIFO_C2P, O_WRONLY);

    if (fd_p2c < 0 || fd_c2p < 0) {
        perror("fifo open");
        exit(1);
    }

    for (int i = 0; i < ITERATIONS; i++) {

        int slot;

        /* Wait for producer message */
        read(fd_p2c, &slot, sizeof(int));

        char msg[SLOT_SIZE];

        memcpy(msg,
               ptr + (slot * SLOT_SIZE),
               SLOT_SIZE);

        printf("Consumed: %s from slot %d\n",
               msg,
               slot);

        sleep(1);

        /* Free slot */
        memcpy(ptr + (slot * SLOT_SIZE),
               FREE_SLOT,
               SLOT_SIZE);

        /* Notify producer */
        write(fd_c2p, &slot, sizeof(int));
    }

    close(fd_p2c);
    close(fd_c2p);

    munmap(ptr, SIZE);

    shm_unlink(SHM_NAME);

    unlink(FIFO_P2C);
    unlink(FIFO_C2P);

    return 0;
}