#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void read_serie(char * arquivo, double *vetor, int tamanho){
    FILE *fp = fopen(arquivo, "r");
    if(fp==NULL){
        fprintf(stderr, "não foi possivel abrir o arquivo %s\n", arquivo);
        exit(1);
    }
    for(int i=0; i<tamanho; ++i){
        fscanf(fp, "%lf", &vetor[i]);
    }
}

void max_min_avg(double *vetor, int tamanho, double *max, double *min, double *media){
    double soma = 0;
    *max = *min = vetor[0];
    for(int i=0; i< tamanho; i++){
        soma += vetor[i];
        if (vetor[i] > *max)
            *max = vetor[i];
        if (vetor[i] < *min)
            *min = vetor[i];
    }
    *media = soma / tamanho;
}
void max_min_sum(double *vetor, int tamanho, double *max, double *min, double *sum){
    double soma = 0;
    *max = *min = vetor[0];
    for(int i=0; i< tamanho; i++){
        soma += vetor[i];
        if (vetor[i] > *max)
            *max = vetor[i];
        if (vetor[i] < *min)
            *min = vetor[i];
    }
    *sum = soma;
}

int main(int argc, char **argv){
    if (argc != 4){
        fprintf(stderr, "necessário 3 argumentos: %s <arquivo time series> <tamanho time series> <tamanho janela>\n", argv[0]);
        return 1;
    }

    MPI_Init(NULL, NULL);

    // Obter número de processos
    int num_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Obter ranking dos processos
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int tam_serie = atoi(argv[2]);
    int tam_janela = atoi(argv[3]);
    double *serie = (double *) malloc(sizeof(double)*tam_serie);

    // Para série total
    if (rank == 0) {
        read_serie(argv[1], serie, tam_serie);
        printf("tamanho da serie: %d, tamanho da janela: %d\n",tam_serie, tam_janela);
    }
    MPI_Bcast(serie, tam_serie, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    double max, min, media, auxmax, auxmin, soma;

    // Declara série menor para paralelização
    int tam_auxserie = tam_serie / num_procs;
    double *auxserie = (double *) malloc(sizeof(double)*(tam_auxserie));

    // Scatter
    MPI_Scatter(serie, tam_auxserie, MPI_DOUBLE, auxserie, tam_auxserie, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Obtém máximo, mínimo e soma de cada subsérie
    max_min_sum(auxserie, tam_auxserie, &auxmax, &auxmin, &soma);

    // Gather e cálculo de média
    MPI_Reduce(&auxmax, &max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&auxmin, &min, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&soma, &media, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        media = media / tam_serie;
        printf("serie total - max: %lf, min: %lf, media: %lf\n", max, min, media);
    }


    // Para janelas

    // Vetores para armazenar resultados
    int num_janelas = tam_serie - tam_janela;
    double *medias = (double *) malloc(sizeof(double)*(num_janelas + 1));
    double *maximos = (double *) malloc(sizeof(double)*(num_janelas + 1));
    double *minimos = (double *) malloc(sizeof(double)*(num_janelas + 1));

    // for(int i = 0; i <= tam_serie - tam_janela; i++){
    //     max_min_avg(&serie[i],tam_janela, &max, &min, &media);
    //     printf("janela %d - max: %lf, min: %lf, media: %lf\n", i, max, min, media);
    // }

    MPI_Finalize();
    return 0;
}
