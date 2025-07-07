#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_FILE "msg.txt"
#define MAX_MSG_SIZE 2000

double check_accuracy(char* received_msg, int received_msg_size);

#endif