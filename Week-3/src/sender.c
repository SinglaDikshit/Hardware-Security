#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include "utils.h"

#define BIT_DURATION 250000000L
#define LLC_ARRAY_SIZE (8 * 1024 * 1024)

unsigned long long rdtsc() {
	unsigned long long a, d;
	asm volatile ("mfence");
	asm volatile ("rdtsc" : "=a" (a), "=d" (d));
	a = (d<<32) | a;
	asm volatile ("mfence");
	return a;
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
    char *buffer = (char*) malloc(LLC_ARRAY_SIZE);
    for (int i = 0; i < LLC_ARRAY_SIZE; i += 64)
        buffer[i] = 1; // warm-up

    sleep(1); // allow receiver to get ready

    // Send preamble: 0xFF, 0xFF, 0xFF = 24 continuous 1s
    for (int p = 0; p < 3; p++) {
        for (int b = 0; b < 8; b++) {
            for (int r = 0; r < 3; r++) {
            unsigned long long start = rdtsc();
            while (rdtsc() - start < BIT_DURATION) {
                for (int i = 0; i < LLC_ARRAY_SIZE; i += 64)
                    buffer[i]++;
            }
            }
        }
    }

    // Send actual message
    for (int i = 0; i < msg_size; i++) {
        char c = msg[i];
        for (int b = 0; b < 8; b++) {
            int bit = (c >> (7 - b)) & 1;
            for (int r = 0; r < 3; r++) {
            unsigned long long start = rdtsc();
            if (bit == 1) {
                while (rdtsc() - start < BIT_DURATION)
                    for (int i = 0; i < LLC_ARRAY_SIZE; i += 64)
                        buffer[i]++;
            } else {
                while (rdtsc() - start < BIT_DURATION)
                    for (int i = 0; i < 64; i++)
                        asm volatile ("nop");
            }
        }
        }
    }

    free(buffer);
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
