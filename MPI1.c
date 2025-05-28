#define _CRT_SECURE_NO_WARNINGS
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 100
#define MAX_LINE_LEN 1024
#define MAX_WORDS 10000

typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordFreq;

void normalizeWord(char* word) {
    int j = 0;
    char temp[MAX_WORD_LEN];
    for (int i = 0; word[i]; ++i) {
        if (isalpha((unsigned char)word[i])) {
            temp[j++] = tolower(word[i]);
        }
    }
    temp[j] = '\0';
    strcpy(word, temp);
}

void addWord(WordFreq* list, int* count, const char* word) {
    for (int i = 0; i < *count; ++i) {
        if (strcmp(list[i].word, word) == 0) {
            list[i].count++;
            return;
        }
    }
    strcpy(list[*count].word, word);
    list[*count].count = 1;
    (*count)++;
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double startTime = MPI_Wtime();  

    const char* filename = "C:\\Users\\stanc\\source\\repos\\Proiect APD\\Proiect APD\\text.txt";


    FILE* file = fopen(filename, "r");

    if (!file) {
        if (rank == 0) fprintf(stderr, "Eroare la deschiderea fisierului!\n");
        MPI_Finalize();
        return 1;
    }

    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    long chunkSize = fileSize / size;
    long start = rank * chunkSize;
    long end = (rank == size - 1) ? fileSize : start + chunkSize;

    fseek(file, start, SEEK_SET);

    if (rank != 0) {
        char skip[MAX_LINE_LEN];
        fgets(skip, MAX_LINE_LEN, file); 
    }

    WordFreq* localWords = malloc(MAX_WORDS * sizeof(WordFreq));
    int localCount = 0;

    char line[MAX_LINE_LEN];
    while (ftell(file) < end && fgets(line, MAX_LINE_LEN, file)) {
        char* token = strtok(line, " \t\r\n");
        while (token) {
            normalizeWord(token);
            if (strlen(token) > 0)
                addWord(localWords, &localCount, token);
            token = strtok(NULL, " \t\r\n");
        }
    }

    fclose(file);

    
    if (rank == 0) {
        WordFreq* globalWords = malloc(MAX_WORDS * size * sizeof(WordFreq));
        int globalCount = 0;

        
        for (int i = 0; i < localCount; ++i) {
            addWord(globalWords, &globalCount, localWords[i].word);
            globalWords[globalCount - 1].count = localWords[i].count;
        }

        for (int i = 1; i < size; ++i) {
            int recvCount;
            MPI_Recv(&recvCount, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            WordFreq* recvWords = malloc(recvCount * sizeof(WordFreq));
            MPI_Recv(recvWords, recvCount * sizeof(WordFreq), MPI_BYTE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int j = 0; j < recvCount; ++j) {
                int found = 0;
                for (int k = 0; k < globalCount; ++k) {
                    if (strcmp(globalWords[k].word, recvWords[j].word) == 0) {
                        globalWords[k].count += recvWords[j].count;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    strcpy(globalWords[globalCount].word, recvWords[j].word);
                    globalWords[globalCount].count = recvWords[j].count;
                    globalCount++;
                }
            }

            free(recvWords);
        }

        
        FILE* out = fopen("word_freq_mpi.txt", "w");
        for (int i = 0; i < globalCount; ++i) {
            fprintf(out, "%s: %d\n", globalWords[i].word, globalWords[i].count);
        }

        
        double endTime = MPI_Wtime();  
        fprintf(out, "\nTimp total de executie: %.6f secunde\n", endTime - startTime);

        fclose(out);
        printf("Rezultatele au fost salvate in word_freq_mpi.txt\n");

        free(globalWords);
    }
    else {
        // Procesele non-zero trimit
        MPI_Send(&localCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(localWords, localCount * sizeof(WordFreq), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }

    free(localWords);
    MPI_Finalize();
    return 0;
}
