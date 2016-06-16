#pragma once
#include <stdio.h>
#include "Common.h"
void * emptyFile = "";

char * loadTextFile(const char * fname, int * size) {
    DBG_ASSERT(size != nullptr, "null size ptr");
    FILE *f = fopen(fname, "r");
    DBG_ASSERT(f != nullptr, "File %s couldn't be opened", fname);
    fseek(f, 0L, SEEK_END);
    *size = ftell(f);
    rewind(f);
    char *buf = new char[*size + 1];
    size_t res = fread(buf, sizeof(char), *size, f);
    DBG_ASSERT( res != 0, " Expected non-zero result for fread");
    buf[res] = '\0';

    fclose(f);

    return buf;
}



int getLine(char **line, FILE *fp) {
    DBG_ASSERT(fp != nullptr, "invalid file pointer for getline");
    DBG_ASSERT(line != nullptr, "null line");
    size_t sz = 512;
    char *buf = new char[sz];
    int i = 0;
    char c;
    
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            buf[i] = '\0';
            break;
        }
        buf[i++] = c;

        if (i == sz) {
            char *tmp = new char[sz << 1];
            for (int j = 0; j < sz; j++) {
                tmp[j] = buf[j];
            }
            delete[] buf;
            buf = tmp;
        }
    }

    *line = buf;
    return i;
}