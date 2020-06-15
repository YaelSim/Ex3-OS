// Yael Simhis - 209009604


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <fcntl.h>

size_t getHalf(size_t len) {
    if (len % 2 == 0) {
        return len / 2;
    } else {
        return (len / 2) + 1;
    }
}


size_t countSimilarChars(int file1, int file2, size_t firstLen, size_t secondLen,
                          size_t offset1, size_t offset2) {
    char file1Char[1], file2Char[1];
    size_t fullOffset = 0, count = 0;
    int readChar;

    lseek(file1, offset1, SEEK_SET);
    lseek(file2, offset2, SEEK_SET);

    // loop to count how many chars in the file are identical
    while (fullOffset < firstLen && fullOffset < secondLen) {
        // read one char every time
        readChar = read(file1, file1Char, 1);
        if (readChar != 1) {
            break;
        }
        readChar = read(file2, file2Char, 1);
        if (readChar != 1) {
            break;
        }

        // identical char
        if (*file1Char == *file2Char && *file1Char != '\n' && *file2Char != '\n') {
            count++;
        }

        fullOffset++;
    }
    return count;
}

int compareFiles(char *firstFile, char *secondFile) {
    bool identical = false, similar = false;
    size_t countSimilar, i, j;

    // open the files
    int file1 = open(firstFile, O_RDONLY);
    int file2 = open(secondFile, O_RDONLY);

    // len without "\n"
    size_t firstLen = lseek(file1, 0, SEEK_END) - 1;
    size_t secondLen = lseek(file2, 0, SEEK_END) - 1;

    // get half size
    size_t halfFirstLen = getHalf(firstLen);
    size_t halfSecondLen = getHalf(secondLen);
    size_t getMinHalfLen = firstLen < secondLen ? halfFirstLen : halfSecondLen;

    // check if the file are identical/similar/different
    if (firstLen == secondLen) {
        countSimilar = countSimilarChars(file1, file2, firstLen, secondLen, 0, 0);
        if (countSimilar == firstLen) {
            identical = true;
        }
    }

    if (!identical) {
        for (i = 0; i < firstLen; i++) {
            for (j = 0; j < secondLen; j++) {
                countSimilar = countSimilarChars(file1, file2, firstLen - i, secondLen - j, i, j);
                if (countSimilar >= getMinHalfLen) {
                    similar = true;
                }
            }
        }
    }

    // close the files
    close(file1);
    close(file2);

    if (identical) {
        return 1;
    } else if (similar) {
        return 3;
    } else {
        // the files are different
        return 2;
    }
}

int main(int argc, char *argv[]) {
    // if there is less then 3 arguments then it's valid
    if (argc < 3) {
        exit(-1);
    }
    return compareFiles(argv[1], argv[2]);
}