
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct character { // struct for printing 
    int n; // the number of same sequential characters
    char c; // character
};

void uncomp_file_stdout(char *input) {
    FILE *file = fopen(input, "rb");
    struct character character;

    if (file == NULL) {
        printf("my-unzip: cannot open file\n");
        exit(1);
    }
    
    while (fread(&character.n, sizeof(character.n), 1, file) > 0) { // uncompressing the file
        fread(&character.c, sizeof(character.c), 1, file); 
        for (int i = 0; i < character.n; i++) { // printing the same sequential characters
            printf("%c", character.c);
        }
    }

    fclose(file);
    return;
}


int main(int argc, char *argv[]) {

    int i = 1;
    if (argc == 1) {
        printf("my-zip: file1 [file2 ...]\n");
        exit(1);
    } else {
        while (i < argc) { // looping through all files
            uncomp_file_stdout(argv[i]);
            i++;
        }
    } 

    return 0;
}
