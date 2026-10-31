/*
 * problema1.c
 * Problema 1 - Caso 1 do Teorema Mestre
 * Multiplicacao recursiva de matrizes (divisao e conquista).
 *
 * Recorrencia:  T(n) = 8T(n/2) + Theta(n^2)
 *   a = 8, b = 2, f(n) = n^2, n^(log_b a) = n^3
 *   Caso 1 (epsilon = 1)  =>  T(n) = Theta(n^3)
 *
 * Contagem de operacoes:
 *   - caso base (n = 1): 1 multiplicacao
 *   - combinacao: n^2 somas
 *   => T(1) = 1, T(n) = 8T(n/2) + n^2  =>  T(n) = 2n^3 - n^2
 *
 * Uso:    ./problema1 [n_max] [arquivo_csv]
 *   n_max: maior tamanho (potencia de 2). Padrao: 256
 *   Executa n = 1, 2, 4, ..., n_max e grava em arquivo CSV (modo append).
 *
 * Compilar: gcc -O2 -Wall -o problema1 problema1.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef long long ll;

#define REPETICOES 3            /* repeticoes de cada n para medir o tempo */
#define PROBLEMA   "P1"

static ll operacoes = 0;        /* contador global de operacoes */

/* Retorna o tempo atual em milissegundos (relogio monotonico). */
static double agora_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

/*
 * Multiplica A * B e grava em C (sem acumular: C e sobrescrita).
 * As matrizes sao vetores 1D em ordem de linha; lda/ldb/ldc sao os
 * "strides" (largura da matriz original), o que permite usar submatrizes
 * sem copia, apenas com aritmetica de ponteiros.
 * n deve ser potencia de 2.
 */
static void mult_rec(const int *A, int lda,
                     const int *B, int ldb,
                     int *C, int ldc, int n)
{
    if (n == 1) {                       /* caso base: T(1) = 1 */
        C[0] = A[0] * B[0];
        operacoes++;
        return;
    }

    int h = n / 2;
    int *T = malloc((size_t)n * n * sizeof(int));   /* temporaria, stride n */
    if (!T) { fprintf(stderr, "Erro: malloc falhou\n"); exit(1); }

    /* Quadrantes de A, B, C e T */
    const int *A11 = A,               *A12 = A + h;
    const int *A21 = A + h * lda,     *A22 = A + h * lda + h;
    const int *B11 = B,               *B12 = B + h;
    const int *B21 = B + h * ldb,     *B22 = B + h * ldb + h;
    int *C11 = C,                     *C12 = C + h;
    int *C21 = C + h * ldc,           *C22 = C + h * ldc + h;
    int *T11 = T,                     *T12 = T + h;
    int *T21 = T + h * n,             *T22 = T + h * n + h;

    /* 8 chamadas recursivas de tamanho n/2  ->  8T(n/2) */
    mult_rec(A11, lda, B11, ldb, C11, ldc, h);   /* C11 = A11*B11 (+ A12*B21) */
    mult_rec(A12, lda, B21, ldb, T11, n,   h);
    mult_rec(A11, lda, B12, ldb, C12, ldc, h);   /* C12 = A11*B12 (+ A12*B22) */
    mult_rec(A12, lda, B22, ldb, T12, n,   h);
    mult_rec(A21, lda, B11, ldb, C21, ldc, h);   /* C21 = A21*B11 (+ A22*B21) */
    mult_rec(A22, lda, B21, ldb, T21, n,   h);
    mult_rec(A21, lda, B12, ldb, C22, ldc, h);   /* C22 = A21*B12 (+ A22*B22) */
    mult_rec(A22, lda, B22, ldb, T22, n,   h);

    /* Combinacao: C += T  ->  f(n) = n^2 somas */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i * ldc + j] += T[i * n + j];
    operacoes += (ll)n * n;

    free(T);
}

/* Multiplicacao iterativa classica, usada apenas para validar o resultado
 * (nao entra na contagem de operacoes nem no tempo medido). */
static void mult_ref(const int *A, const int *B, int *C, int n)
{
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            int s = 0;
            for (int k = 0; k < n; k++) s += A[i * n + k] * B[k * n + j];
            C[i * n + j] = s;
        }
}

int main(int argc, char *argv[])
{
    int n_max = (argc > 1) ? atoi(argv[1]) : 256;
    const char *arq = (argc > 2) ? argv[2] : "resultados.csv";

    if (n_max < 1 || (n_max & (n_max - 1)) != 0) {
        fprintf(stderr, "n_max deve ser potencia de 2 (recebido: %d)\n", n_max);
        return 1;
    }

    FILE *f = fopen(arq, "a");
    if (!f) { perror("fopen"); return 1; }
    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0)                                   /* arquivo novo */
        fprintf(f, "problema,n,operacoes,tempo_ms\n");

    srand(42);                                           /* reprodutivel */

    printf("%6s %16s %16s %12s\n", "n", "operacoes", "2n^3-n^2", "tempo(ms)");

    for (int n = 1; n <= n_max; n *= 2) {
        size_t tam = (size_t)n * n;
        int *A  = malloc(tam * sizeof(int));
        int *B  = malloc(tam * sizeof(int));
        int *C  = malloc(tam * sizeof(int));
        int *Cr = malloc(tam * sizeof(int));
        if (!A || !B || !C || !Cr) { fprintf(stderr, "malloc falhou\n"); return 1; }

        for (size_t i = 0; i < tam; i++) { A[i] = rand() % 10; B[i] = rand() % 10; }

        double melhor = -1.0;
        ll ops = 0;
        for (int r = 0; r < REPETICOES; r++) {
            operacoes = 0;
            double t0 = agora_ms();
            mult_rec(A, n, B, n, C, n, n);
            double dt = agora_ms() - t0;
            if (melhor < 0 || dt < melhor) melhor = dt;  /* menor tempo = menos ruido */
            ops = operacoes;
        }

        /* Validacoes */
        mult_ref(A, B, Cr, n);
        for (size_t i = 0; i < tam; i++)
            if (C[i] != Cr[i]) { fprintf(stderr, "ERRO: resultado incorreto (n=%d)\n", n); return 1; }

        ll esperado = 2LL * n * n * n - (ll)n * n;
        if (ops != esperado)
            fprintf(stderr, "AVISO: contagem %lld != formula %lld (n=%d)\n", ops, esperado, n);

        printf("%6d %16lld %16lld %12.3f\n", n, ops, esperado, melhor);
        fprintf(f, "%s,%d,%lld,%.6f\n", PROBLEMA, n, ops, melhor);

        free(A); free(B); free(C); free(Cr);
    }

    fclose(f);
    printf("Resultados gravados em %s\n", arq);
    return 0;
}