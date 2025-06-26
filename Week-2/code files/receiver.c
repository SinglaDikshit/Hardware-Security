#include <stdio.h>
#include "utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define THRESHOLD 2000
#define BIT_DELAY 2000  // microseconds
#define START_PATTERN 0b10101011

unsigned long long rdtsc() {
    unsigned long long a, d;
    asm volatile ("mfence");
    asm volatile ("rdtsc" : "=a" (a), "=d" (d));
    a = (d<<32) | a;
    asm volatile ("mfence");
    return a;
}

void maccess(void* p) {
    asm volatile ("movq (%0), %%rax\n"
        :
        : "c" (p)
        : "rax");
}

void flush(void* p) {
    asm volatile ("clflush 0(%0)\n"
        :
        : "c" (p)
        : "rax");
}

int read_bit(void* addr) {
    unsigned long long start = rdtsc();
    maccess(addr);
    unsigned long long end = rdtsc();
    flush(addr);
    unsigned long long delta = end - start;
    printf("Access latency: %llu cycles\n", delta);  // Debug output
    return (delta < THRESHOLD) ? 1 : 0;
}

int main() {
    printf("Receiver started. Waiting for sender...\n");

    // Shared memory setup
    int fd = shm_open("/covert_channel", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }
    ftruncate(fd, 4096);
    char* shared_mem = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    void* target = &shared_mem[128];
    printf("Listening on shared memory address...\n");

    // Wait for start pattern
    printf("Waiting for start pattern...\n");
    int found = 0;
    while (!found) {
        int pattern = 0;
        for (int i = 0; i < 8; i++) {
            int bit = read_bit(target);
            pattern = (pattern << 1) | bit;
            usleep(BIT_DELAY);
        }

        int xor = pattern ^ START_PATTERN;
        int bit_diff = __builtin_popcount(xor);
        printf("Pattern received: 0x%02X | Diff bits: %d\n", pattern, bit_diff);

        if (bit_diff <= 1) {  // fuzzy match: allow 1-bit error
            found = 1;
            printf("Start pattern detected (fuzzy match).\n");
        }
    }

    // Receive message
    char received_msg[MAX_MSG_SIZE] = {0};
    int msg_index = 0;

    while (msg_index < MAX_MSG_SIZE) {
        char ch = 0;
        for (int i = 0; i < 8; i++) {
            ch = (ch << 1) | read_bit(target);
            usleep(BIT_DELAY);
        }
        received_msg[msg_index++] = ch;
        if (ch == '\0') break;
    }

    int received_msg_size = msg_index;
    printf("Received: %s\n", received_msg);
    printf("Accuracy (%%): %f\n", check_accuracy(received_msg, received_msg_size) * 100);

    shm_unlink("/covert_channel");  // clean up
    return 0;
}
