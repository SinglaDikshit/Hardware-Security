#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define SHARED_MEM_SIZE 4096
#define BIT_DELAY 2000
#define START_PATTERN 0b10101011

unsigned long long rdtsc() {
	unsigned long long a, d;
	asm volatile ("mfence");
	asm volatile ("rdtsc" : "=a" (a), "=d" (d));
	a = (d<<32) | a;
	asm volatile ("mfence");
	return a;
}

void maccess(void* p)
{
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

int main(){

    // ********** DO NOT MODIFY THIS SECTION **********
    FILE *fp = fopen(MSG_FILE, "r");
    if(fp == NULL){
        printf("Error opening file\n");
        return 1;
    }

    char msg[MAX_MSG_SIZE];
    int msg_size = 0;
    char c;
    while((c = fgetc(fp)) != EOF){
        msg[msg_size++] = c;
    }
    fclose(fp);

    clock_t start = clock();
    // **********************************************
    // ********** YOUR CODE STARTS HERE **********


    int fd = shm_open("/covert_channel", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, 4096);

    char* shared_mem = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

    void* target = &shared_mem[128]; // arbitrary offset

    // Send a known start sequence, e.g., 10101011
    //int start_sequence[8] = {1, 0, 1, 0, 1, 0, 1, 1};
    //for (int i = 0; i < 8; i++) {
     //   if (start_sequence[i]) maccess(target); // send 1
      //  usleep(BIT_DELAY);
        //flush(target);
    //    }

    char strt = START_PATTERN;
for (int i = 7; i >= 0; i--) {
    int bit = (strt >> i) & 1;
    if (bit) maccess(target);
    usleep(BIT_DELAY);
    flush(target);
}
    printf("Start pattern sent!\n");


    // Transmit the message
    for (int i = 0; i < msg_size; i++) {
        char ch = msg[i];
        for (int bit = 7; bit >= 0; bit--) {
            int bit_val = (ch >> bit) & 1;
            if (bit_val == 1) {
                maccess(target);  // access to bring into cache
            }
            usleep(BIT_DELAY);
            flush(target);  // always flush after each bit
        }
    }



    // ********** YOUR CODE ENDS HERE **********
    // ********** DO NOT MODIFY THIS SECTION **********
    clock_t end = clock();
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Message sent successfully\n");
    printf("Time taken to send the message: %f\n", time_taken);
    printf("Message size: %d\n", msg_size);
    printf("Bits per second: %f\n", msg_size * 8 / time_taken);
    // **********************************************
}
