
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_LEN 1024

void printfile(char *input) {
    FILE *file = fopen(input, "r"); // opening the file
    char buffer[MAX_LINE_LEN];

    if (file == NULL) { // if the file can't be opened
        printf("my-cat: cannot open file\n");
        exit(1);
    }

    while (fgets(buffer, MAX_LINE_LEN, file) != NULL) {
        printf("%s", buffer); // printing out the contents of the file line by line
    }
    fclose(file);
    return;
}

int main(int argc, char *argv[]) { 
    int i = 1;

    if (argc == 1) { // if the user did not enter file(s)
        exit(0);
    }

    while (i < argc) {
        printfile(argv[i]);
        i++;
    }

    return(0);
}

