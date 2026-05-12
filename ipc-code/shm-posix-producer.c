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

    /* Create shared memory */
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    ptr = mmap(0, SIZE,
               PROT_READ | PROT_WRITE,
               MAP_SHARED,
               shm_fd,
               0);

    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    /* Initialize all slots as free */
    for (int i = 0; i < NUM_SLOTS; i++) {
        memcpy(ptr + (i * SLOT_SIZE), FREE_SLOT, SLOT_SIZE);
    }

    /* Create FIFOs */
    mkfifo(FIFO_P2C, 0666);
    mkfifo(FIFO_C2P, 0666);

    int fd_p2c = open(FIFO_P2C, O_WRONLY);
    int fd_c2p = open(FIFO_C2P, O_RDONLY);

    if (fd_p2c < 0 || fd_c2p < 0) {
        perror("fifo open");
        exit(1);
    }

    int next_slot = 0;

    for (int i = 0; i < ITERATIONS; i++) {

        /* Wait until current slot becomes free */
        while (memcmp(ptr + (next_slot * SLOT_SIZE),
                      FREE_SLOT,
                      SLOT_SIZE) != 0) {

            int freed_slot;

            /* Wait for consumer notification */
            read(fd_c2p, &freed_slot, sizeof(int));
        }

        char msg[SLOT_SIZE];

        snprintf(msg, SLOT_SIZE, "MSG%04d", i);

        memcpy(ptr + (next_slot * SLOT_SIZE),
               msg,
               SLOT_SIZE);

        printf("Produced: %s at slot %d\n",
               msg,
               next_slot);

        /* Notify consumer */
        write(fd_p2c, &next_slot, sizeof(int));

        next_slot = (next_slot + 1) % NUM_SLOTS;
    }

    close(fd_p2c);
    close(fd_c2p);
    munmap(ptr, SIZE);

    return 0;
}