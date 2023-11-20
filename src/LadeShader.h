#pragma once

#include <cstdio>
#include <cstdlib>

char *readTextFileIntoString(const char *filename) {
    printf("Reading shader %s\n", filename);
    char *buf;
    long length;
    FILE *f{fopen(filename, "rb")};

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buf = new char[length + 1]; //End of String needs space
        fread(buf, 1, length, f);
        fclose(f);
        buf[length] = 0;
    } else {
        printf("File not found!\n");
        // Print the error message from the OS
        perror(filename);
        return nullptr;
    }

    return buf;
}
