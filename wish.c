// Emma Pakarinen 28.2.2023

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

const char basic_error_message[30] = "An error has occurred\n";
const char memory_error_message[40] = "Memory allocation error has occurred\n";
const char execv_error_message[40] = "An error with execv has occurred\n";
const char file_error_message[40] = "An error with file has occurred\n";
const char redir_error_message[40] = "An error with redirection has occurred\n";
const char path_error_message[40] = "An error with path has occurred\n";


typedef struct parallelCommands { 
    char **runcommand, *file;
    int size;
    bool redir;
    struct parallelCommands *next;
} COMMANDS;


void freeArray(char **array, int size) {

    for (int i = 0; i<size; i++) {
        free(array[i]);
    }

    free(array);
}


void freeLinkedList(COMMANDS *first_node) {
    COMMANDS *ptr = first_node;

    while (ptr != NULL) {
        first_node = ptr->next;

        if (ptr->redir) {
            free(ptr->file);
        }

        freeArray(ptr->runcommand, ptr->size); // runcommand is an array so it needs to be freed in freeArray-function
        free(ptr);
        ptr = first_node;
    }

    return;
}


int testRedirection(char *input, ssize_t ip_size) {
    int redir_mark = 0, file_amount = -1; // file_amount = -1 because NULL will be added to the variable at the end
    char *w = NULL, copy[ip_size];
    char *delimiters = " \t\r\n\v\f"; 
    
    strcpy(copy, input);
    w = strtok(copy, delimiters);

    while (w!=NULL) {
        if (strchr(w, '>') != NULL) {
            redir_mark++;
        }
        if (redir_mark > 0) { // if redir mark has been passed --> counting how many output files 
            file_amount++;
        }
        w = strtok(NULL, delimiters);
    }

    if (file_amount > 1) { // file amount should be max 1, if not --> return file_amount and program will throw error in main
        return file_amount;
    }

    return redir_mark; 
}


void runCommandParallel(COMMANDS *first) { 
    COMMANDS *ptr = first;
    int status;
    pid_t pid; // process id

    if (first == NULL) { // test if linked list is empty
        return;
    }

    while (ptr != NULL) {
        pid = fork();
        
        if (pid == 0) { // child process
            if (ptr->redir) { // if redirection is true in current command
                int fd = open(ptr->file, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU); // writing the output of command to file, trunicating or creating the file and giving sufficient rights to the file
                if (fd == -1) {
                    write(STDERR_FILENO, file_error_message, strlen(file_error_message));
                } else {
                    dup2(fd, 1); // duplicating the file descriptor to standard output, reference: https://www.geeksforgeeks.org/dup-dup2-linux-system-call/ 
                    close(fd);
                }
            }
            execv(ptr->runcommand[0], ptr->runcommand);
            write(STDERR_FILENO, execv_error_message, strlen(execv_error_message));
        }
        ptr = ptr->next;
    }

    // parent waiting for children to finish
    while (wait(&status) > 0) { // wait returns -1 when no children are left, reference: https://stackoverflow.com/questions/60210236/parellel-processes-using-fork-with-command-line-parameters-in-c
        continue;
    }

    return;
}


void runCommand(COMMANDS *command) { 
    int status;
    pid_t pid; // process id

    pid = fork();

    if (pid == -1) {
        write(STDERR_FILENO, basic_error_message, strlen(basic_error_message));
        return;
    }

    if (pid == 0) { // child
        if (command->redir) { // if redirection true
            int fd = open(command->file, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU); // writing the output of command to file, trunicating or creating the file
            if (fd == -1) {
                write(STDERR_FILENO, file_error_message, strlen(file_error_message));
            } else {
                dup2(fd, 1); // duplicating the file descriptor to standard output, https://www.geeksforgeeks.org/dup-dup2-linux-system-call/ 
                close(fd);
            }
        }
        execv(command->runcommand[0], command->runcommand);
        write(STDERR_FILENO, execv_error_message, strlen(execv_error_message));
    } else { // parent
        waitpid(pid, &status, 0);
    }

    return;
}


bool testPath(char *path, char *command, char *runpath) {
    bool x = false;

    // building the path
    strcpy(runpath, path);
    strcat(runpath, "/");
    strcat(runpath, command);

    if (access(runpath, X_OK) == 0) { // testing if the path is suitable
        x = true;
    } 

    return x;
}


bool findCorrectPath(int path_count, char *runpath, char **command, char **path) {
    bool path_found = false;

    path_found = testPath(path[0], command[0], runpath); // testing the first path
    if (path_found) { // if path_found = true --> correct path has been found and return back to main
        return path_found;
    }

    if (path_count > 1) { // if user has intialized multiple paths 
        for (int i = 1; i<path_count; i++) { // i = 1 because the first path has already been tested above

            if (!runpath) { // if malloc failed 
                write(STDERR_FILENO, memory_error_message, strlen(memory_error_message));
                exit(1);
            }

            path_found = testPath(path[i], command[0], runpath);
            if (path_found) { // if path_found = true --> correct path has been found and loop can be ended
                return path_found;
            }

            runpath = (char *) realloc(runpath, (strlen(path[i]) + strlen(command[0]) + 2)); // +2 for '/0' and '/' from testPath function
        }
    }

    free(runpath);
    return path_found;
}


void parseInput(char *input, char **command, ssize_t ip_size, int commandcount, bool redir, char **outputfile) {
    char *word;
    char input_copy[ip_size];
    
    strcpy(input_copy, input);
    char delimiters[] = " \t\r\n\v\f>"; // reference: https://stackoverflow.com/questions/26597977/split-string-with-multiple-delimiters-using-strtok-in-c 
    word = strtok(input_copy, delimiters); 

    for (int i = 0; i < commandcount; i++) {
        command[i] = strdup(word); 
        word = strtok(NULL, delimiters);
    }

    if (redir) { /* if redirection is true then saving the filename to outputfile from word -variable (since the outputfile was retracted from command_count previously in
    main if redirection was true) */
        *outputfile = malloc(strlen(word) + 1);
        strcpy(*outputfile, word);
    } else {
        *outputfile = NULL;
    }

    return;
} 


int countParameters(char *input, ssize_t ip_size) {
    int i = 0; 
    char *w = NULL, copy[ip_size];
    char *delimiters = " \t\r\n\v\f>"; 
    
    strcpy(copy, input);
    w = strtok(copy, delimiters);

    while (w!=NULL) {
        i++;
        w = strtok(NULL, delimiters);
    }

    return i; 
}


int countParameters_Parallel(char *input, ssize_t ip_size) {
    int i = 0; 
    char *w = NULL, copy[ip_size];
    char *delimiters = "&"; // separating the parallel commands
    
    strcpy(copy, input);
    w = strtok(copy, delimiters);
    while (w!=NULL) {
        i++;
        w = strtok(NULL, delimiters);
    }
    return i; 
}


void parseInput_Parallel(char *input, char **command, ssize_t ip_size) {
    int i = 0;
    char *word;
    char input_copy[ip_size];
    strcpy(input_copy, input);
    char delimiters[] = "&"; 

    word = strtok(input_copy, delimiters); 
    while (word != NULL) {
        command[i] = strdup(word); // saving the commands to be run in parallel
        word = strtok(NULL, delimiters);
        i++;
    }

    return;
} 



int main(int argc, char *argv[]) {
    char *buffer = NULL, *runpath = NULL, **command = NULL, **commandparams = NULL;
    size_t buf_size = 0, command_count = 0, path_count = 1, redir_check = 0, command_count_parallel = 0;
    ssize_t input_size = 0;
    FILE *batchfile = NULL;
    char **path = calloc(path_count+1, sizeof(char *));
    path[0] = strdup("/bin"); // initial path of wish, path_count = 1
    bool found_path, redir, parallel, batch = false; // boolean variables: if a path needs to be tested, command to be redirected, commands to be run parallel, batch file invoked


    if (argc > 2) {
        write(STDERR_FILENO, "usage: ./wish [batchfile]\n", strlen("usage: ./wish [batchfile]\n"));
        exit(1);
    } 
    if (argc == 2) { // if batch -file invoked with program call to wish.c
        batchfile = fopen(argv[1], "r");
        if (!batchfile) {
            write(STDERR_FILENO, file_error_message, strlen(file_error_message));
            exit(1);
        }
        batch = true;
    }


    while(1) {
        // initializing variables
        COMMANDS *pFirst = NULL;
        parallel = false;
        redir = false;
        found_path = false;

        if (batch) {
            input_size = getline(&buffer, &buf_size, batchfile); // reading lines from batch file if batch = true
            if (input_size < 0) {
                break;
            }
        } else {
            printf("wish> ");
            input_size = getline(&buffer, &buf_size, stdin); // reading from user input
        }

        if (input_size == 1) { // ignoring newline 
            continue;
        }

        buffer[strcspn(buffer, "\n")] = 0; // removing newline from buffer

        // checking for parallel marks in input
        if (strchr(buffer, '&') != NULL) {
            parallel = true;
        }

        // checking for redirection marks in input
        if (strchr(buffer, '>') != NULL) {

            if (!parallel) { // if parallel = false then the user input should have only one '>' and only one outputfile
                redir_check = testRedirection(buffer, input_size); // test_redirection should only return 1 in order for the program to continue

                if (redir_check > 1) { // if more than 1 of ">" is found from user input or too many output files (when parallel is false) it leads to error
                    write(STDERR_FILENO, redir_error_message, strlen(redir_error_message));
                    continue;
                }
            }

            redir = true;
        }


        if (parallel) {
            command_count = countParameters_Parallel(buffer, input_size); // count for how many commands are to be run in parallel mode
        } else {
            command_count = countParameters(buffer, input_size);
            if (redir) {
                if (command_count < 2) { // to check for sufficient amount of parameters are given when redirecting command. Command_count should be min. 2 (i.e., {"ls", "output.txt"})
                    write(STDERR_FILENO, redir_error_message, strlen(redir_error_message));
                    continue;
                }
                command_count -= 1; // retracting the output -file from command_count variable
            }
        }

        if (command_count == 0) { // if buffer was only whitespace
            continue;
        }

       
        command = calloc(command_count+1, sizeof(char *)); // reference: https://stackoverflow.com/questions/22416160/changing-an-array-of-strings-inside-a-function-in-c 
        // added +1 to command_count for NULL
        if (!command) {
            write(STDERR_FILENO, memory_error_message, strlen(memory_error_message));
            exit(1);
        }


        if (parallel) {
            parseInput_Parallel(buffer, command, input_size); // saving parallel commands to command -variable

            if (redir) { // if redirection is true then checking if the command have correct amount of redirection marks
                for (int i = 0; i < command_count; i++) {

                    redir_check = testRedirection(command[i], strlen(command[i]));

                    if (redir_check > 1) { // if more than 1 of ">" is found per parallel command --> error
                        write(STDERR_FILENO, redir_error_message, strlen(redir_error_message));
                        freeArray(command, command_count);
                        redir = false;
                        break;
                    }
                }

                if (!redir) { // if redir is false after loop --> back to the beginning of the wish-loop
                    continue;
                }
            }
        } else { 
            if ((pFirst = (COMMANDS*)malloc(sizeof(COMMANDS))) == NULL) {
                write(STDERR_FILENO, memory_error_message, strlen(memory_error_message));
                exit(1);
            }
            parseInput(buffer, command, input_size, command_count, redir, &pFirst->file); // saving command with parameters to command -variable
        }
        

        // Checking for built-in commands:
        if (strcmp(command[0], "exit") == 0) {

            if (command_count != 1) { // if exit has arguments --> throw error
                write(STDERR_FILENO, basic_error_message, strlen(basic_error_message));
                continue;
            } else {
                freeArray(command, command_count); 
                free(pFirst);
                break;
            }
        } else if (strcmp(command[0], "path") == 0) { 

            freeArray(path, path_count); // freeing previously allocated memory for path -list 

            path_count = command_count-1; // if user sends "wish> path /usr/bin" then we only want /usr/bin so the path_count is one less than command_count
            path = calloc(path_count+1, sizeof(char *)); // +1 for NULL

            if (!path) {
                write(STDERR_FILENO, memory_error_message, strlen(memory_error_message));
                exit(1);
            }

            if (path_count == 0) { // if user gave only path and no arguments
                path[0] = strdup("");
            } else {
                for (int i = 0; i<path_count; i++) { 
                    path[i] = strdup(command[i+1]);
                }
            }
    
            freeArray(command, command_count); // 
            free(pFirst);

            continue;
        } else if (strcmp(command[0], "cd") == 0) {
            if (command_count != 2) { // cd accepts only one argument (i.e. wish> cd directory) which equals 2 arguments
                write(STDERR_FILENO, "usage: cd directory\n", strlen("usage: cd directory\n"));
                continue;
            } 
            if (chdir(command[1]) != 0) {
                write(STDERR_FILENO, basic_error_message, strlen(basic_error_message));
                continue;
            }
            continue;
        }
        
        
        if (path_count == 0) { // if path_count = 0, then there are no paths added and we can go back to the start of the loop instead of testing for suitable paths for commands
            write(STDERR_FILENO, path_error_message, strlen(path_error_message));
            continue;
        } 
    

        if (parallel) { // dealing with parallel commands and parsing the input in this section
            
            for (int i = 0; i<command_count; i++) { // iterating through commands to be run parallel
                COMMANDS *pNew, *ptr;

                if ((pNew = (COMMANDS*)malloc(sizeof(COMMANDS))) == NULL) {
                    write(STDERR_FILENO, memory_error_message, strlen(memory_error_message));
                    exit(1);
                }

                pNew->redir = false; // initializing redirection as false for the command
                command_count_parallel = countParameters(command[i], strlen(command[i]) + 1); // checking argument count for each command to be run parallel

                if (strchr(command[i], '>') != NULL) { // checking if command has redirection mark
                    pNew->redir = true; 
                    command_count_parallel -= 1; // retracting the output -file from command_count variable
                }

                commandparams = calloc(command_count_parallel, sizeof(char *)); // saving the command and its parameters to commandparams -variable
                parseInput(command[i], commandparams, (strlen(command[i]) + 1), command_count_parallel, pNew->redir, &pNew->file);
                

                runpath = (char *) malloc((strlen(path[0])+ strlen(commandparams[0]) + 2)); // +2 for '/0' and '/' from testPath-function
                found_path = findCorrectPath(path_count, runpath, commandparams, path);
                
                if (!found_path) { // if found_path = false after the function --> no suitable paths found so freeing variables/arrays and breaking out of the for-loop
                    freeArray(commandparams, command_count_parallel);
                    if (i > 0) {
                        freeLinkedList(pFirst);
                    }
                    else {
                        free(pNew);
                    }
                    break;
                } else { // creating linked list for the parallel commands

                    pNew->runcommand = calloc(command_count_parallel+2, sizeof(char *)); 
                    if (!pNew->runcommand) {  // if calloc failed
                        write(STDERR_FILENO, memory_error_message, strlen(memory_error_message));
                        exit(1);
                    }

                    pNew->size = command_count_parallel; 
                    pNew->next = NULL;

                    pNew->runcommand[0] = strdup(runpath); // first parameter for execv is the file path where the command can be run

                    for (int i = 1; i < command_count_parallel; i++) {
                        pNew->runcommand[i] = strdup(commandparams[i]); // saving rest arguments to runcommand
                    }

                    pNew->runcommand[command_count_parallel + 1] = NULL;
                   
                    // creating the linked list
                    if (pFirst == NULL) {
                        pFirst = pNew;
                    } else {
                        ptr = pFirst;
                        while (ptr->next != NULL) {
                            ptr = ptr->next;
                        }
                        ptr->next = pNew;
                    }
                    
                }
                // freeing variables before next iteration in for -loop
                free(runpath);
                freeArray(commandparams, command_count_parallel);
            }

        } else { // if user has given one command to be run
            runpath = (char *) malloc((strlen(path[0])+ strlen(command[0]) + 2)); // +2 for '/0' and '/' from testPath function
            found_path = findCorrectPath(path_count, runpath, command, path);
            
            if (found_path) {
                if (redir) {
                    pFirst->redir = true;
                } else {
                    pFirst->redir = false;
                }

                pFirst->runcommand = calloc(command_count+2, sizeof(char *)); 

                if (!pFirst->runcommand) {  // if calloc failed
                    write(STDERR_FILENO, memory_error_message, strlen(memory_error_message));
                    exit(1);
                }

                pFirst->size = command_count;
                pFirst->runcommand[0] = strdup(runpath); // first parameter for execv is the file path where the command can be run

                for (int i = 1; i < command_count; i++) {
                    pFirst->runcommand[i] = strdup(command[i]);
                }

                pFirst->next = NULL;

            } else {
                free(pFirst);
            }
        }

        if (!found_path) { // if found_path = false after the function --> no suitable paths found and back to beginning of main loop
            write(STDERR_FILENO, path_error_message, strlen(path_error_message)); 
            freeArray(command, command_count);
            continue; 
        }


        if (parallel) {
            runCommandParallel(pFirst);
        } else {
            runCommand(pFirst);
            free(runpath);
        }

        // freeing rest variables/arrays that had memory allocated
        freeLinkedList(pFirst);
        freeArray(command, command_count);
    }


    freeArray(path, path_count+1);
    free(buffer);

    return 0;
}