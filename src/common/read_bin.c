#include "vulkan-tutorial.h"

#include <stdio.h>
#include <stdlib.h>

char *read_bin(const char *path, int *p_size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return NULL;
    char *buf = (char *)malloc(sizeof(char) * MAX_SHADER_BIN_SIZE);
    int cnt = 0;
    while (!feof(file)) {
        char c;
        if (fread(&c, sizeof(char), 1, file) > 0) {
            buf[cnt] = c;
            cnt += 1;
        }
    }
    fclose(file);
    *p_size = cnt;
    return buf;
}
