#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h> // for __rdtscp, __rdtsc, _mm_clflush
#include <stdlib.h>

#define NUM_SAMPLES 100000

uint64_t measure_access_time(volatile int *addr, int flush_cache) {
    unsigned int aux;
    uint64_t start, end;

    if (flush_cache)
        _mm_clflush((void *)addr); // evict from cache

    _mm_lfence(); // wait for all previous instructions
    start = __rdtscp(&aux); // timestamp before access
    (void)*addr; // memory access
    _mm_lfence(); // wait again
    end = __rdtscp(&aux); // timestamp after access

    return end - start;
}

int main() {
    int *array = malloc(64); // allocate memory
    if (!array) {
        perror("malloc failed");
        return 1;
    }

     FILE *f = fopen("latency_data.csv", "w");
    if (!f) {
        perror("file open failed");
        return 1;
    }

    fprintf(f, "Sample,Cache (cycles),DRAM (cycles)\n");

    uint64_t cache_total = 0, dram_total = 0, cache =0, dram=0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        cache_total += measure_access_time(array, 0);  // Access from cache
        dram_total += measure_access_time(array, 1);   // Access from DRAM
        cache = measure_access_time(array, 0);
        dram = measure_access_time(array, 1);
        fprintf(f, "%d,%lu,%lu\n", i + 1, cache, dram);
    }

    fclose(f);

    printf("Average Cache Access Latency: %lf cycles\n", cache_total / (double)NUM_SAMPLES);
    printf("Average DRAM Access Latency:  %lf cycles\n", dram_total / (double)NUM_SAMPLES);

    free(array);
    printf("Data saved to latency_data.csv\n");
    return 0;
}