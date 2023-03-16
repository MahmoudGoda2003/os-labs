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

struct ElementData{
    int i,j;
    int *row;
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
                free(l);
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

void *ElementMultiplication(void *ThreadData){
    struct ElementData *data = ThreadData;
    data->c->data[data->i][data->j] =0;
    for (int i = 0; i < data->b->row; ++i) {
        data->c->data[data->i][data->j] += data->row[i]*data->b->data[i][data->j];
    }
    free(data);
    pthread_exit(0);
}

void ElementMatrixMultiplication(struct data *Data){
    int ThreadsNumber = Data->a->row*Data->b->col;
    pthread_t Threads[ThreadsNumber];
    for (int i = 0; i < Data->c->row; ++i) {
        for (int j = 0; j < Data->c->col; ++j) {
            struct ElementData *ThreadData = malloc(sizeof(struct ElementData));
            ThreadData->row = Data->a->data[i];
            ThreadData->c = Data->c;
            ThreadData->b = Data->b;
            ThreadData->i = i;
            ThreadData->j = j;
            pthread_create(&Threads[i*Data->b->col+j], NULL, ElementMultiplication, ThreadData);
        }
    }
    for (int i = 0; i < ThreadsNumber; ++i) {
        pthread_join(Threads[i], NULL);
    }
}

void Free(struct matrix *Data){
    for (int i = 0; i < Data->row; ++i) {
        free(Data->data[i]);
    }
    free(Data->data);
}

int main(int argc, char *argv[]) {
    struct timeval stop, start;
    struct data *Data = malloc(sizeof(struct data));
    struct matrix a,b,c;
    char *A="a",*B="b",*C="c";
    if (argc == 4){
        A = argv[1];
        B = argv[2];
        C = argv[3];
    }
    if (ReadFile(A, &a) || ReadFile(B, &b)) {
        free(Data);
        return 0;
    }

    Data->a = &a;
    Data->b = &b;
    CreateMatrix(&c, a.row, b.col);
    Data->c = &c;

    if (a.col != b.row) {
        printf("Sizes aren't suitable\n");
        Free(Data->c);
        Free(Data->a);
        Free(Data->b);
        free(Data);
        return 0;
    }

    char name[100];
    strcpy(name,C);
    for (int i = 0; i < 3; ++i) {
        gettimeofday(&start, NULL);
        if(i==0){
            MatrixMultiplication(Data);
            strcat(name,"0");
        }else if(i==1){
            RowMatrixMultiplication(Data);
            strcat(name,"1");
        }else{
            ElementMatrixMultiplication(Data);
            strcat(name,"2");
        }
        gettimeofday(&stop, NULL);
        printf("%d.Seconds taken %lu\n", i,stop.tv_sec - start.tv_sec);
        printf("%d.Microseconds taken: %lu\n", i,stop.tv_usec - start.tv_usec);
        WriteFile(name, Data->c);
    }
    Free(Data->c);
    Free(Data->a);
    Free(Data->b);
    free(Data);
    return 0;
}

