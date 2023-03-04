#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct content {
        char *string;
        struct content *pPrev;
        struct content *pNext;
} CONTENT;


CONTENT *readfile(char *input, CONTENT *pFirst) {
        CONTENT *pNew, *ptr;
        
        char *buffer = NULL; // reference for getline: https://riptutorial.com/c/example/8274/get-lines-from-a-file-using-getline-- 
        size_t bfsize = 0; 
        ssize_t rivi_size; 

        FILE *file = fopen(input, "r");

        if (file == NULL) {
                fprintf(stderr,"reverse: cannot open file '%s'\n", input);
                exit(1);
        }
        

        // saving memory for linked list
        int koko = sizeof(CONTENT);

        // first line form the file
        rivi_size = getline(&buffer, &bfsize, file); 
        
        while (rivi_size >= 0) {
                
                pNew = (CONTENT*)malloc(koko);
                if (pNew == NULL) {
                        fprintf(stderr,"Malloc failed.\n");
                        exit(1);
                }

                pNew->string = malloc(strlen(buffer) + 1);
                if (pNew->string == NULL) {
			fprintf(stderr, "Malloc failed.\n");
			exit(1);
		}

                strcpy(pNew->string, buffer);
                pNew->pNext=NULL;

                if (pFirst == NULL) { 
                        pFirst = pNew;
                        pFirst->pPrev = NULL; // first node's previous and next are pointing to NULL which helps printing backwards
                } else {
                        ptr = pFirst;
                        while(ptr->pNext != NULL) {
                                ptr = ptr->pNext;
                        }
                        ptr->pNext = pNew;
                        ptr->pNext->pPrev = ptr; // the previous of the next node points to the current (second to last) node

                }
                rivi_size = getline(&buffer, &bfsize, file); 
        }

        free(buffer);    
        fclose(file);
        return pFirst;
}


CONTENT *print_backw(CONTENT *pF) {
        CONTENT *ptr = pF;

        if (pF == NULL) {
                exit(0);
        }

        while (1) {
                if (ptr->pNext == NULL) { // finding the last node and printing it
                        fprintf(stdout, "%s", ptr->string);
                        ptr = ptr->pPrev; // ptr is the previous node
                        free(ptr->pNext->string); // freeing the node we don't need anymore
                        free(ptr->pNext);
                        ptr->pNext = NULL;
                        ptr = pF; // ptr back to the beginning
                }
                if (ptr->pNext == NULL && ptr->pPrev == NULL) { // if next and previous nodes are NULL then this is the first node to be printed which will be the last to be printed
                        fprintf(stdout, "%s", ptr->string);
                        free(ptr->string);
                        free(ptr);
                        break;
                }
                ptr = ptr->pNext;
        }

        return pF;
}


CONTENT *write_backw(char *output, CONTENT *pF) {
        FILE *file;
        CONTENT *ptr = pF;

        file = fopen(output, "w");

        if (file == NULL) {
                fprintf(stderr,"reverse: cannot open file '%s'\n", output);
                exit(1);
        }
        
        if (pF == NULL) {
                exit(0);
        }

        // writing the contents from other file backwards to the second file
        while (1) {
                if (ptr->pNext == NULL) {
                        fprintf(file, "%s", ptr->string);
                        ptr = ptr->pPrev;
                        free(ptr->pNext->string); // freeing the nodes we don't need anymore
                        free(ptr->pNext);
                        ptr->pNext = NULL;
                        ptr = pF;
                }
                if (ptr->pNext == NULL && ptr->pPrev == NULL) {
                        fprintf(file, "%s", ptr->string);
                        free(ptr->string);
                        free(ptr);
                        break;
                }
                ptr = ptr->pNext;
                
        }

        fclose(file);
        return pF;
}



CONTENT *read_input(CONTENT *pFirst) {
        CONTENT *ptr, *pNew;
        char *buffer = NULL;
        size_t bf_size = 0;
        size_t input;

        int koko = sizeof(CONTENT);
        
        while(1) {
                fprintf(stdout, "Write lines to be reversed, enter exits: ");
                input = getline(&buffer, &bf_size, stdin);

                if (input == 1) { // if stdin was a newline --> break
                        break;
                }

                pNew = (CONTENT*)malloc(koko);
                if (pNew == NULL) {
                        fprintf(stderr,"Malloc failed.\n");
                        exit(1);
                }

                pNew->string = malloc(strlen(buffer) + 1);
                if (pNew->string == NULL) {
			fprintf(stderr, "Malloc failed.\n");
			exit(1);
		}

                // saving the input to linked list
                strcpy(pNew->string, buffer);
                pNew->pNext=NULL;

                if (pFirst == NULL) { 
                        pFirst = pNew;
                        pFirst->pPrev = NULL;
                } else {
                        ptr = pFirst;
                        while(ptr->pNext != NULL) {
                                ptr = ptr->pNext;
                        }
                        ptr->pNext = pNew;
                        ptr->pNext->pPrev = ptr;
                }

        }

        free(buffer); 

        return pFirst;
}


int main(int args, char *argv[]) { 
        CONTENT *pFirst = NULL;
        char file[256];
        char file2[256];
        int check = 0;
        int check2 = 0;

        if (args == 2) { // if user gives only one file to be printed backwards
                pFirst = readfile(argv[1], pFirst);
                pFirst = print_backw(pFirst);
        } else if (args == 3) { // user gives two files: the contents from first file is saved backwards to the second file

                // if user writes i.e., dir/file ja dir2/file, check if the files are the same
                char *keno = strrchr(argv[1], '/'); // finds the last '/' in the argument

                if (keno != NULL) { // if the first argument is a file
                        strcpy(file, keno+1);                        
                        check++;
                } 
                
                char *keno2 = strrchr(argv[2], '/');
                if (keno2 != NULL) { // if the secong argument is also a path
                        strcpy(file2, keno2+1); 
                        check2++;
                        if (check != 0) { // if the first argument was a path --> check if file and file2 are the same
                                if (strcmp(file, file2) == 0) {
                                        fprintf(stderr, "reverse: input and output file must differ\n");
                                        exit(1); 
                                }
                        } else { // if only the second argument was a path
                                if (strcmp(argv[1], file2) == 0) {
                                        fprintf(stderr, "reverse: input and output file must differ\n");
                                        exit(1); 
                                }
                        }
                } else if (check != 0 && check2 == 0) { // if only the first argument was a path
                        if (strcmp(file, argv[2]) == 0) {
                                fprintf(stderr, "reverse: input and output file must differ\n");
                                exit(1); 
                        }
                }

                if (strcmp(argv[1], argv[2]) == 0) { // if neither were paths --> checking if the arguments differ
                        fprintf(stderr, "reverse: input and output file must differ\n");
                        exit(1);
                }

                pFirst = readfile(argv[1], pFirst); // reading the first file
                pFirst = write_backw(argv[2], pFirst); // printing the first file to the second file backwards

        } else if (args == 1) {

                pFirst = read_input(pFirst); // reading from standard input
                pFirst = print_backw(pFirst); // printing the input backwards

        } else {
                fprintf(stderr, "usage: reverse <input> <output>\n");
                exit(1);
        }

        return 0;
}