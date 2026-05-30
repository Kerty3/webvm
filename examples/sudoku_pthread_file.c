#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9
#define THREADS 27

int sudoku[SIZE][SIZE];
int valid[THREADS] = {0};

typedef struct {
    int row;
    int col;
    int index;
} parameters;

int read_sudoku_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        perror("Cannot open file");
        return 0;
    }

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (fscanf(fp, "%d", &sudoku[i][j]) != 1) {
                printf("File format error: need 81 integers.\n");
                fclose(fp);
                return 0;
            }
        }
    }

    fclose(fp);
    return 1;
}

void *check_row(void *arg) {
    parameters *p = (parameters *)arg;
    int seen[10] = {0};

    for (int col = 0; col < SIZE; col++) {
        int num = sudoku[p->row][col];

        if (num < 1 || num > 9 || seen[num]) {
            pthread_exit(NULL);
        }

        seen[num] = 1;
    }

    valid[p->index] = 1;
    pthread_exit(NULL);
}

void *check_column(void *arg) {
    parameters *p = (parameters *)arg;
    int seen[10] = {0};

    for (int row = 0; row < SIZE; row++) {
        int num = sudoku[row][p->col];

        if (num < 1 || num > 9 || seen[num]) {
            pthread_exit(NULL);
        }

        seen[num] = 1;
    }

    valid[p->index] = 1;
    pthread_exit(NULL);
}

void *check_subgrid(void *arg) {
    parameters *p = (parameters *)arg;
    int seen[10] = {0};

    for (int i = p->row; i < p->row + 3; i++) {
        for (int j = p->col; j < p->col + 3; j++) {
            int num = sudoku[i][j];

            if (num < 1 || num > 9 || seen[num]) {
                pthread_exit(NULL);
            }

            seen[num] = 1;
        }
    }

    valid[p->index] = 1;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t threads[THREADS];
    parameters *data[THREADS];
    int thread_index = 0;

    if (argc != 2) {
        printf("Usage: %s sudoku.txt\n", argv[0]);
        return 1;
    }

    if (!read_sudoku_from_file(argv[1])) {
        return 1;
    }

    for (int i = 0; i < SIZE; i++) {
        data[thread_index] = malloc(sizeof(parameters));
        data[thread_index]->row = i;
        data[thread_index]->col = 0;
        data[thread_index]->index = thread_index;

        pthread_create(&threads[thread_index], NULL,
                       check_row, data[thread_index]);
        thread_index++;
    }

    for (int i = 0; i < SIZE; i++) {
        data[thread_index] = malloc(sizeof(parameters));
        data[thread_index]->row = 0;
        data[thread_index]->col = i;
        data[thread_index]->index = thread_index;

        pthread_create(&threads[thread_index], NULL,
                       check_column, data[thread_index]);
        thread_index++;
    }

    for (int row = 0; row < SIZE; row += 3) {
        for (int col = 0; col < SIZE; col += 3) {
            data[thread_index] = malloc(sizeof(parameters));
            data[thread_index]->row = row;
            data[thread_index]->col = col;
            data[thread_index]->index = thread_index;

            pthread_create(&threads[thread_index], NULL,
                           check_subgrid, data[thread_index]);
            thread_index++;
        }
    }

    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    int is_valid = 1;

    for (int i = 0; i < THREADS; i++) {
        if (valid[i] == 0) {
            is_valid = 0;
            break;
        }
    }

    if (is_valid) {
        printf("Sudoku solution is valid.\n");
    } else {
        printf("Sudoku solution is invalid.\n");
    }

    for (int i = 0; i < THREADS; i++) {
        free(data[i]);
    }

    return 0;
}
