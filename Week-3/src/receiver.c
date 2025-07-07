#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define BIT_DURATION 250000000L
#define NUM_BITS 2048 *3 // Adjust based on expected message size
#define LLC_ARRAY_SIZE (8 * 1024 * 1024)

unsigned long long rdtsc() {
	unsigned long long a, d;
	asm volatile ("mfence");
	asm volatile ("rdtsc" : "=a" (a), "=d" (d));
	a = (d<<32) | a;
	asm volatile ("mfence");
	return a;
}

int main() {
    char* bitstream = (char*) malloc(NUM_BITS);
    char* buffer = (char*) malloc(LLC_ARRAY_SIZE);

    for (int i = 0; i < LLC_ARRAY_SIZE; i += 64)
        buffer[i] = 1; // warm-up

    // === CALIBRATE ===
    int quiet_sum = 0;
    for (int j = 0; j < 10; j++) {
        unsigned long long start = rdtsc();
        int counter = 0;
        while (rdtsc() - start < BIT_DURATION) {
            for (int i = 0; i < LLC_ARRAY_SIZE; i += 64) {
                buffer[i]++;
                counter++;
            }
        }
        quiet_sum += counter;
    }
    int quiet_avg = quiet_sum / 10;
    int threshold = quiet_avg * 0.75;
    printf("Calibrated threshold = %d\n", threshold);

    printf("Receiver starting to collect %d bits...\n", NUM_BITS);

    // === COLLECT BITS ===
    for (int bit_idx = 0; bit_idx < NUM_BITS; bit_idx++) {
        unsigned long long start = rdtsc();
        int counter = 0;
        while (rdtsc() - start < BIT_DURATION) {
            for (int i = 0; i < LLC_ARRAY_SIZE; i += 64) {
                buffer[i]++;
                counter++;
            }
        }

        int bit = (counter < threshold) ? 1 : 0;
        bitstream[bit_idx] = bit;

        // Uncomment for debug
        // printf("Bit %3d: counter = %7d => %d\n", bit_idx, counter, bit);
    }

    // === DETECT PREAMBLE ===
    int start_idx = -1;
    for (int i = 0; i < NUM_BITS - 72; i++) {
    int match = 1;
    for (int j = 0; j < 72; j++) {
        if (bitstream[i + j] != 1) {
            match = 0;
            break;
        }
    }
    if (match) {
        start_idx = i + 72;
        break;
    }
}
    

    if (start_idx == -1) {
        printf("Preamble not found. Exiting.\n");
        free(bitstream);
        free(buffer);
        return 1;
    }

    int received_msg_size = (NUM_BITS - start_idx) / 8;
    char* received_msg = (char*) malloc(received_msg_size);

    for (int i = 0; i < received_msg_size; i++) {
        char byte = 0;
        for (int b = 0; b < 8; b++) {
            byte = (byte << 1) | bitstream[start_idx + i * 8 + b];
        }
        received_msg[i] = byte;
    }

    printf("Received Message:\n");
    for (int i = 0; i < received_msg_size; i++)
        printf("%c", received_msg[i]);
    printf("\n");

    printf(" Accuracy: %.2f%%\n", check_accuracy(received_msg, received_msg_size) * 100);

    free(received_msg);
    free(bitstream);
    free(buffer);
}
