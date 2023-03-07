
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void grep_file(char *keyword, char *input) {
    FILE *file = fopen(input, "r");
    char *buffer = NULL;
    size_t buf_size = 0;
    ssize_t row_size;
    
    if (file == NULL) {
        printf("my-grep: cannot open file\n");
        exit(1);
    }

    row_size = getline(&buffer, &buf_size, file);
    while (row_size >= 0) { // looping as long as eof is found
        char *check;
        check = strstr(buffer, keyword); // reference: https://cplusplus.com/reference/cstring/strstr/

        if (check != NULL) {
            printf("%s", buffer); // if keyword is found from the entered line, the line will be printed
        }

        row_size = getline(&buffer, &buf_size, file);
    }

    free(buffer);
    return;
}

void grep_stdin(char *keyword) {
    char *buffer = NULL;
    size_t buf_size = 0;
    ssize_t input_size;
    char *check;

    while (1) {
        printf("Enter a string, newline quits: ");
        input_size = getline(&buffer, &buf_size, stdin);

        if (input_size <= 1) { // if user enters only newline the loop ends
            break;
        }

        check = strstr(buffer, keyword); // if keyword is found --> buffer is printed

        if (check != NULL) {
            printf("%s", buffer);
        }
    }

    free(buffer);
    return;
}

int main(int argc, char *argv[]) {
    int i = 2;

    if (argc == 1) {
        printf("my-grep: searchterm [file ...]\n");
        exit(1);
    } else if (argc == 2) { // if user sends only a keyword to be found from stdin
        grep_stdin(argv[1]);
    }

    while (i < argc) { // if user sends a keyword and a file where the keyword is to be found
        if (i > 2) {
            printf("\n");
        }
        grep_file(argv[1], argv[i]);
        i++;
    }

    return 0;
}