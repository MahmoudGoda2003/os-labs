#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

struct matrix{
    int row, col;
    int **data;
};

struct data{
    struct matrix *a;
    struct matrix *b;
    struct matrix *c;
};
struct RowData{
    int *row;
    int index;
    struct matrix *b;
    struct matrix *c;
};

void CreateMatrix(struct matrix *m, int row, int col) {
    m->row = row;
    m->col = col;
    m->data = malloc(row * sizeof(int *));
    for (int i = 0; i < row; i++) {
        m->data[i] = malloc(col * sizeof(int));
    }
}

int ReadFile(char *file, struct matrix *m) {
    FILE *File;
    char name[100];

    memcpy(name, file, sizeof(name));
    strcat(name, ".txt");
    File = fopen(name, "r");
    if (File == NULL) {
        printf("Failed to open file %s\n", name);
        return 1;
    }

    int row, col;
    char *l = NULL;
    size_t len = 0;
    getline(&l, &len, File);
    sscanf(l, "row=%d col=%d", &row, &col);
    CreateMatrix(m, row, col);

    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            if (fscanf(File, "%d", &m->data[i][j]) != 1) {
                printf("Failed to read file %s\n", name);
                free(m->data[0]);
                free(m->data);
                return 1;
            }
        }
    }

    free(l);
    fclose(File);
    return 0;
}

void WriteFile(char *file,struct matrix *m){
    struct matrix *x = m;
    FILE *File;
    char name[100];
    memcpy(name,file,sizeof(name));
    strcat(name,".txt");
    File =fopen(name,"w");

    fprintf(File, "row=%d col=%d\n", m->row, m->col);
    for (int i = 0; i < m->row; ++i) {
        for (int j = 0; j < m->col; ++j) {
            fprintf(File, "%d\t", m->data[i][j]);
        }
        fprintf(File, "\n");
    }
    fclose(File);
}

void MatrixMultiplication(struct data *Data) {
    for (int i = 0; i < Data->a->row; i++) {
        for (int j = 0; j < Data->b->col; j++) {
            Data->c->data[i][j] = 0;

            for (int k = 0; k < Data->a->col ;k++) {
                Data->c->data[i][j] += Data->a->data[i][k] * Data->b->data[k][j];
            }
        }
    }
}
void *RowMultiplication(void *ThreadData){
    struct RowData *data = ThreadData;
    for (int i = 0; i < data->b->col; ++i) {
        data->c->data[data->index][i] =0;
        for (int j = 0; j < data->b->row; ++j) {
            data->c->data[data->index][i]+= data->row[j]*data->b->data[j][i];
        }
    }
    free(data);
    pthread_exit(0);
}

void RowMatrixMultiplication(struct data *Data) {
    int ThreadsNumber =Data->a->row;
    pthread_t Threads[ThreadsNumber];
    for (int i = 0; i < ThreadsNumber; ++i) {
        struct RowData *ThreadData = malloc(sizeof(struct RowData));
        ThreadData->row = Data->a->data[i];
        ThreadData->b = Data->b;
        ThreadData->c = Data->c;
        ThreadData->index = i;
        pthread_create(&Threads[i], NULL, RowMultiplication, ThreadData);
    }
    for (int i = 0; i < ThreadsNumber; ++i) {
        pthread_join(Threads[i], NULL);
    }
}

int main() {
    struct data *Data = malloc(sizeof(struct data));
    struct matrix a;
    struct matrix b;
    struct matrix c;

    if (ReadFile("a", &a) || ReadFile("b", &b)) {
        printf("Didn't read files");
        return 0;
    }

    Data->a = &a;
    Data->b = &b;
    CreateMatrix(&c, a.row, b.col);
    Data->c = &c;

    if (a.col != b.row) {
        printf("Sizes aren't suitable");
        return 0;
    }
    for (int i = 0; i < 2; ++i) {
        if(i==0){
            MatrixMultiplication(Data);
        }else if(i==1){
            RowMatrixMultiplication(Data);
        }
        WriteFile(sprintf("", "c%d",i), Data->c);
    }
    free(Data->c->data);
    free(Data);
    return 0;
}
