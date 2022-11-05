#ifndef V1_COPY_H
#define V1_COPY_H


#include <stddef.h>


void copy(int from_fd, int to_fd, size_t count);

char* processHeader(char *rawData);

#endif //V1_COPY_H
