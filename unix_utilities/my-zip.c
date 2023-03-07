#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void comp_file_stdout(char *input) {
    FILE *file = fopen(input, "r");
    char *buffer = NULL;
    size_t buf_size = 0;
    ssize_t row_size;
    int c = 1;

    if (file == NULL) {
        printf("my-zip: cannot open file\n");
        exit(1);
    }

    row_size = getline(&buffer, &buf_size, file);

    while (row_size >= 0) {
        int len = strlen(buffer);
        int i = 0;
        while (i < len-1) { // i < buffer (row in a file) -1
            int j = i+1; // j is always one bigger than i
            c = 1;
            while (buffer[j] == buffer[i] && j < len) { // as long as the current and the next one are same
                j++;
                c++;
            }
            
            fwrite(&c, sizeof(c), 1, stdout); // writing to terminal
            fwrite(&buffer[i], sizeof(buffer[i]), 1, stdout);
            i = j-1;
            
            if (buffer[j]=='\n') { // taking newline into account, if it is found, then it is printed to stdout
                int n = 1;
                fwrite(&n, sizeof(n), 1, stdout);
                fwrite(&buffer[j], sizeof(buffer[j]), 1, stdout);
            }
            i++;
        }
        c = 0;
        row_size = getline(&buffer, &buf_size, file);
    }
    free(buffer);
    fclose(file);
    return;
}


char *merge_files(char *f, int n) { // writing the contents of files (if entered > 1) to the same merged.txt file
    FILE *file;
    FILE *m_file;
    char c;

    if (n == 0) { // check to see if merged.txt is empty or not
        file = fopen(f, "r");
        m_file = fopen("merged.txt", "w");
    } else {
        file = fopen(f, "r");
        m_file = fopen("merged.txt", "a"); 
    }

    if (file == NULL || m_file == NULL) {
        printf("my-zip: cannot open file\n");
        exit(1);
    }

    while ((c = fgetc(file)) != EOF) {
        fputc(c, m_file); // writing to the file character by character
    }

    fclose(file);
    fclose(m_file);
    return("merged.txt");
}



int main(int argc, char *argv[]) {
    char *files;
    int i = 1;
    int c = 0; // check for merge_files function
    if (argc == 1) {
        printf("my-zip: file1 [file2 ...]\n");
        exit(1);
    } else if (argc > 2) { // if more than 1 file is entered
        while (i < argc) {
            files = merge_files(argv[i], c);
            i++;
            c = 1; // check = 1 when merged.txt has content in merge_files function
        }      
        comp_file_stdout(files); // compressing the files
    } else {
        comp_file_stdout(argv[1]); // compressing file
    } 

    return 0;
}