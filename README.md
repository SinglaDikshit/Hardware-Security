#  Hardware Security

This repository contains notes, slides, and code assignments from the Hardware Security short course, with a focus on cache-based side-channel attacks, specifically Flush+Reload and Occupancy-based attacks.

---

## Table of Contents

1. [Course Overview](#course-overview)
2. [Slides & Notes](#slides--notes)
3. [Assignment 1: Flush+Reload Attack](#assignment-1-flushreload-attack)
   - [Background](#background)
   - [Implementation](#implementation)
   - [Usage](#usage)
   - [Results](#results)
4. [Assignment 2: Occupancy-Based Attack](#assignment-2-occupancy-based-attack)
   - [Background](#background-1)
   - [Implementation](#implementation-1)
   - [Usage](#usage-1)
   - [Results](#results-1)
5. [Mitigations & Countermeasures](#mitigations--countermeasures)
6. [References](#references)

---

## Course Overview

A short course on hardware security, covering:

- Instruction Set Architectures (ISA) and processor pipelines
- Superscalar, in-order and out-of-order execution
- Memory hierarchy and cache organization (direct-mapped, fully-associative, set-associative)
- Caching concepts: hits, misses, replacement policies, write strategies
- Virtual memory and address translation
- Special instructions for cache control (RDTSC, CLFLUSH, fences)
- Side-channel attack vectors and mitigations

Slides and detailed notes are available in the `slides/` directory.

---

## Slides & Notes

- `slides/Session_1_InOrder.pdf`: Basics of in-order superscalar pipelines and hazards
- `slides/Session_2_OutOfOrder.pdf`: Out-of-order superscalar design, reservation stations, reorder buffer
- `slides/Session_3_Caches.pdf`: Cache hierarchy, organizations, and replacement/write strategies
- `slides/Session_4_Attacks.pdf`: Flush+Reload and Occupancy-based attack mechanisms

Refer to the PDF files for full details and diagrams.

---

## Assignment 1: Flush+Reload Attack

### Background

Flush+Reload is a cache side-channel technique that leverages shared memory pages:

1. The attacker flushes a target memory line from the cache using `clflush`.
2. The attacker waits for the victim to access the same line.
3. The attacker reloads the line and measures access latency via `RDTSC`.
4. A low latency indicates the victim accessed the line (cache hit), a high latency indicates otherwise.

### Implementation

- **Language**: C
- **Files**:
  - `flush_reload/sender.c`: Attacker pseudocode and measurement loop
  - `flush_reload/receiver.c`: Victim dummy access pattern

### Usage

Refer to Readme and make file in code files

## Results

On test runs, the attacker correctly identified victim accesses with an accuracy of 47%.

## Assignment 2: Occupancy-Based Attack

 ### Background

Occupancy-based attacks infer activity by measuring cache occupancy:

Sender: Occupies the LLC by sweeping over a large buffer to send a 1, remains idle to send a 0.

Receiver: Measures how many memory accesses it can perform in a fixed time window. Lower throughput indicates higher occupancy (1), higher throughput indicates lower occupancy (0).

### Implementation

Language: C

Files:

occupancy_attack/sender.c: Continuously sweeps memory on 1 or idles on 0.

occupancy_attack/receiver.c: Measures access loops per time window using RDTSC.

### Usage

Refer to Readme and Make file in code files.

### Results

Receiver throughput correlates with sender activity, achieving >90% accuracy over 1000 bits transmitted.

## Mitigations & Countermeasures

Covered in the course:

Cache randomization (CEASER, SCATTER CACHE)

Partitioning techniques (way and set partitioning)

Fully associative randomized caches (MIRAGE)

Data-store decoupling (MAYA cache)

See slides/Session_5_Mitigations.pdf for details on each strategy.



### References

Anirudh Kr. Jangid & Praveen Prajapat, In-Order Superscalar Lecture

Praveen Prajapat & Anubhav Bhatla, Flush+Reload & Occupancy-Based Attacks 

CEASER, SCATTER CACHE, MIRAGE, MAYA publications

