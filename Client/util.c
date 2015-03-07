
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

