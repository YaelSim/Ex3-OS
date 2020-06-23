// Yael Simhis - 209009604 LATE-SUBMISSION

#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define NO_C_FILE "NO_C_FILE,0\n"
#define COMPILATION_ERROR "COMPILATION_ERROR,10\n"
#define TIMEOUT "TIMEOUT,20\n"
#define WRONG "WRONG,50\n"
#define SIMILAR "SIMILAR,75\n"
#define EXCELLENT "EXCELLENT,100\n"

// check if the file is directory
int isDirectory(char *folderPath) {
    struct stat buffer;
    if (stat(folderPath, &buffer) != 0)
        return 0;
    return S_ISDIR(buffer.st_mode);
}

// check if the file is a .c file
int isCFile(char *filePath) {
    char *CFile = strrchr(filePath, '.');
    if(CFile == NULL){
        return 0;
    }
    if(strcmp(CFile,".c") == 0){
        return 1;
    }
    return 0;
}

// compile the c file, if it returned 0 then it didn't succeed
int compileCFile(char *path, char *CFile) {
    char *CompileCommand[] = {"gcc", CFile, NULL};
    int returned, status;
    pid_t pid = fork();

    //child process
    if (pid == 0) {
        returned = chdir(path);
        if (returned == -1) {
            perror("Error in chdir\n");
            exit(-1);
        }
        close(0);
        close(1);
        close(2);
        returned = execvp("gcc", CompileCommand);
        if (returned == -1) {
            perror("Error in execvp\n");
            exit(-1);
        }
    }

    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

//run the .c file
int runCFile(char *path, char *inputFile, char *outputFile) {
    pid_t pid;
    int openFile, outFile, returned, status;
    char *runCommand[] = {"a.out", NULL};
    time_t start = time(NULL);
    pid = fork();

    //child process
    if (pid == 0) {
        // open file
        openFile = open(inputFile, O_RDONLY);
        if (openFile == -1) {
            perror("Error in opening file\n");
            exit(-1);
        }

        // open file
        outFile = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        if (outFile == -1) {
            perror("Error in opening file\n");
            exit(-1);
        }

        returned = chdir(path);
        if (returned == -1) {
            perror("Error in chdir\n");
            exit(-1);
        }

        dup2(openFile, 0);
        dup2(outFile, 1);
        close(openFile);
        close(outFile);
        close(3);

        // run the command
        returned = execvp("./a.out", runCommand);
        if (returned == -1) {
            perror("Error in execvp\n");
            exit(-1);
        }
    }

    waitpid(pid, &status, 0);
    time_t end = time(NULL);
    // if more then 3 seconds them its timout
    if (end - start > 3) {
        return 1;
    }
    return 0;
}

// compare the file using comp.out from ex31
int compare(char *firstFile, char *secondFile) {
    int returned, status;
    pid_t pid;
    char *compareFiles[] = {"comp.out", firstFile, secondFile, NULL};
    pid = fork();

    //child process
    if (pid == 0) {
        close(0);
        close(1);
        close(2);

        returned = execvp("./comp.out", compareFiles);
        if (returned == -1) {
            perror("Error in execvp\n");
            exit(-1);
        }
    }

    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

int main(int argc, char *argv[]) {
    char *firstLine, *secondLine, *thirdLine;
    char writeToCSV[500] = "";
    time_t start, end;
    if (argc < 2) {
        exit(-1);
    }

    // open the files
    int openFile = open(argv[1], O_RDONLY);
    if (openFile < 0) {
        perror("Error in opening file\n");
        return -1;
    }

    // get the len of the file
    size_t fileLen = lseek(openFile, 0, SEEK_END);
    if (fileLen < 0) {
        perror("Error in getting number of length of text\n");
        return -1;
    }

    char buffer[fileLen];
    //initialize array
    memset(buffer, '\0', fileLen*sizeof(char));

    if (lseek(openFile, 0, SEEK_SET) < 0) {
        perror("Error\n");
        return -1;
    }

    // split the given file to 3 lines
    int readFile = read(openFile, buffer, fileLen);
    if (readFile < 0) {
        perror("Error in reading the file\n");
        return -1;
    }
    // get the 3 lines
    firstLine = strtok(buffer, "\n");
    secondLine = strtok(NULL, "\n");
    thirdLine = strtok(NULL, "\n");

    // close the file, no needed anymore
    close(openFile);

    // open the directory, if could not open directory then need to exit
    DIR *openDir = opendir(firstLine);
    if (openDir == NULL) {
        perror("Not a valid directory\n");
        return -1;
    }

    // looping through the directory and enter the sub- directors
    struct dirent *dir;
    struct dirent *inner;
    char folderPath[PATH_MAX];
    char *innerName;
    int flag, breakFlag;
    char *outputPath = "out.txt";
    while ((dir = readdir(openDir)) != NULL) {
        flag = 0;
        // if not a directory then continue
        if (dir->d_type != DT_DIR) {
            continue;
        }

        // get the path to the sub-director
        sprintf(folderPath,"%s/%s", firstLine, dir->d_name);
        //if we get bad path then continue
        if ((strcmp(dir->d_name, ".") == 0) || (strcmp(dir->d_name, "..") == 0)) {
            continue;
        }

        // if the path is a directory
        if(isDirectory(folderPath)) {
            // open the folder
            DIR *innerDir = opendir(folderPath);
            if (innerDir == NULL) {
                perror("Not a valid directory\n");
                return -1;
            }

            // loop inside the folder to find .c files
            while ((inner = readdir(innerDir)) != NULL) {
                breakFlag = 0;
                innerName = inner->d_name;
                //if we get bad path then continue
                if ((strcmp(innerName, ".") == 0) || (strcmp(innerName, "..") == 0)) {
                    continue;
                }

                // check if its .c file
                if (isCFile(innerName)) {
                    flag = 1;
                    // check if the file compiles
                    if (compileCFile(folderPath, innerName) != 0) {
                        // write to results.csv
                        strcat(writeToCSV, dir->d_name);
                        strcat(writeToCSV,",");
                        strcat(writeToCSV, COMPILATION_ERROR);
                        breakFlag = 1;
                    }

                    // if not compiled then break
                    if (breakFlag == 1) {
                        breakFlag = 0;
                        break;
                    }

                    // if compiles then run it
                    if (runCFile(folderPath, secondLine, "out.txt") == 1) {
                        // write to results.csv
                        strcat(writeToCSV, dir->d_name);
                        strcat(writeToCSV,",");
                        strcat(writeToCSV, TIMEOUT);
                        breakFlag = 1;
                    }

                    // if timeout in running then break
                    if (breakFlag == 1) {
                        break;
                    }

                    // use the ex31.c to tell if the output is identical/similar/different
                    int result = compare(thirdLine, outputPath);

                    // print the command by the given result
                    if (result == 1) {
                        //identical - write to results.csv
                        strcat(writeToCSV, dir->d_name);
                        strcat(writeToCSV,",");
                        strcat(writeToCSV, EXCELLENT);
                    } else if (result == 3) {
                        //similar - write to results.csv
                        strcat(writeToCSV, dir->d_name);
                        strcat(writeToCSV,",");
                        strcat(writeToCSV, SIMILAR);
                    } else if (result == 2) {
                        // different - write to results.csv
                        strcat(writeToCSV, dir->d_name);
                        strcat(writeToCSV,",");
                        strcat(writeToCSV, WRONG);
                    } else {
                        perror("Invalid result\n");
                        exit(-1);
                    }
                }
            }

            //if there is no .c file in the folder
            if(flag == 0) {
                // write to results.csv
                strcat(writeToCSV, dir->d_name);
                strcat(writeToCSV,",");
                strcat(writeToCSV, NO_C_FILE);
            }

            // close the sub directory
            closedir(innerDir);
        }
    }

    // close the directory
    closedir(openDir);

    // write to results.csv
    size_t len = strlen(writeToCSV);
    ssize_t writeToFile;
    // create the write file
    int writeCsv = creat("results.csv", 0644);
    if (writeCsv < 0) {
        perror("Error in creating file\n");
        return -1;
    }

    // write to file
    writeToFile = write(writeCsv, writeToCSV, len);
    if (writeToFile < 0) {
        perror("Error in writing to file");
        exit(1);
    }

    close(writeCsv);

    // remove files
    remove("output");
    remove("out.txt");

    return 0;
}