/*
 * problema2.c
 * Problema 2 - Caso 2 do Teorema Mestre
 * Merge sort ternario (divisao em 3 partes).
 *
 * Recorrencia:  T(n) = 3T(n/3) + Theta(n)
 *   a = 3, b = 3, f(n) = n, n^(log_b a) = n
 *   Caso 2  =>  T(n) = Theta(n log n)
 *
 * Contagem de operacoes:
 *   - 1 operacao por elemento selecionado na intercalacao de 3 vias
 *     (a intercalacao de um trecho de tamanho n seleciona n elementos).
 *   - caso base (n = 1): nenhuma operacao.
 *   => T(1) = 0, T(n) = 3T(n/3) + n  =>  T(n) = n * log_3(n)
 *   A contagem NAO depende dos dados (so do tamanho n).
 *   O memcpy de volta para o vetor original nao e contado no modelo.
 *
 * Uso:    ./problema2 [n_max] [arquivo_csv]
 *   n_max: maior tamanho (potencia de 3). Padrao: 531441 (3^12)
 *   Executa n = 1, 3, 9, ..., n_max e grava em arquivo CSV (modo append).
 *
 * Compilar: gcc -O2 -Wall -o problema2 problema2.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef long long ll;

#define REPETICOES 5            /* repeticoes de cada n para medir o tempo */
#define PROBLEMA   "P2"

static ll operacoes = 0;        /* contador global de operacoes */

/* Retorna o tempo atual em milissegundos (relogio monotonico). */
static double agora_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

/*
 * Intercala tres trechos ordenados de a[]:
 *   [lo, m1), [m1, m2), [m2, hi)
 * O resultado e montado em tmp[lo..hi) e copiado de volta para a[].
 * Cada elemento selecionado conta 1 operacao  ->  f(n) = n.
 */
static void merge3(int *a, int *tmp, int lo, int m1, int m2, int hi)
{
    int i = lo, j = m1, k = m2, p = lo;

    while (p < hi) {
        int w = -1, v = 0;                      /* w: trecho vencedor; v: valor */
        if (i < m1)                 { w = 0; v = a[i]; }
        if (j < m2 && (w < 0 || a[j] < v)) { w = 1; v = a[j]; }
        if (k < hi && (w < 0 || a[k] < v)) { w = 2; v = a[k]; }

        tmp[p++] = v;
        operacoes++;                            /* 1 elemento selecionado */

        if (w == 0) i++;
        else if (w == 1) j++;
        else k++;
    }
    memcpy(a + lo, tmp + lo, (size_t)(hi - lo) * sizeof(int));
}

/*
 * Ordena a[lo..hi) por merge sort ternario.
 * (hi - lo) deve ser potencia de 3.
 */
static void msort3(int *a, int *tmp, int lo, int hi)
{
    int n = hi - lo;
    if (n <= 1) return;                         /* caso base: T(1) = 0 */

    int t  = n / 3;
    int m1 = lo + t;
    int m2 = lo + 2 * t;

    /* 3 chamadas recursivas de tamanho n/3  ->  3T(n/3) */
    msort3(a, tmp, lo, m1);
    msort3(a, tmp, m1, m2);
    msort3(a, tmp, m2, hi);

    /* Combinacao: f(n) = n */
    merge3(a, tmp, lo, m1, m2, hi);
}

/* Verifica se o vetor esta ordenado de forma nao decrescente. */
static int esta_ordenado(const int *a, int n)
{
    for (int i = 1; i < n; i++)
        if (a[i - 1] > a[i]) return 0;
    return 1;
}

/* Retorna 1 se m e potencia de 3 (m >= 1). */
static int potencia_de_3(ll m)
{
    if (m < 1) return 0;
    while (m % 3 == 0) m /= 3;
    return m == 1;
}

int main(int argc, char *argv[])
{
    ll n_max = (argc > 1) ? atoll(argv[1]) : 531441;
    const char *arq = (argc > 2) ? argv[2] : "resultados.csv";

    if (!potencia_de_3(n_max)) {
        fprintf(stderr, "n_max deve ser potencia de 3 (recebido: %lld)\n", n_max);
        return 1;
    }

    FILE *f = fopen(arq, "a");
    if (!f) { perror("fopen"); return 1; }
    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0)                                   /* arquivo novo */
        fprintf(f, "problema,n,operacoes,tempo_ms\n");

    srand(42);                                           /* reprodutivel */

    printf("%9s %14s %14s %12s\n", "n", "operacoes", "n*log3(n)", "tempo(ms)");

    for (ll n = 1; n <= n_max; n *= 3) {
        int *orig = malloc((size_t)n * sizeof(int));
        int *work = malloc((size_t)n * sizeof(int));
        int *tmp  = malloc((size_t)n * sizeof(int));
        if (!orig || !work || !tmp) { fprintf(stderr, "malloc falhou\n"); return 1; }

        ll soma_orig = 0;
        for (ll i = 0; i < n; i++) { orig[i] = rand(); soma_orig += orig[i]; }

        double melhor = -1.0;
        ll ops = 0;
        for (int r = 0; r < REPETICOES; r++) {
            memcpy(work, orig, (size_t)n * sizeof(int)); /* fora da medicao */
            operacoes = 0;
            double t0 = agora_ms();
            msort3(work, tmp, 0, (int)n);
            double dt = agora_ms() - t0;
            if (melhor < 0 || dt < melhor) melhor = dt;  /* menor tempo = menos ruido */
            ops = operacoes;
        }

        /* Validacoes */
        ll soma_final = 0;
        for (ll i = 0; i < n; i++) soma_final += work[i];
        if (!esta_ordenado(work, (int)n) || soma_final != soma_orig) {
            fprintf(stderr, "ERRO: vetor incorreto (n=%lld)\n", n);
            return 1;
        }

        int k = 0;
        for (ll p = 1; p < n; p *= 3) k++;               /* k = log3(n) */
        ll esperado = (ll)k * n;
        if (ops != esperado)
            fprintf(stderr, "AVISO: contagem %lld != formula %lld (n=%lld)\n",
                    ops, esperado, n);

        printf("%9lld %14lld %14lld %12.3f\n", n, ops, esperado, melhor);
        fprintf(f, "%s,%lld,%lld,%.6f\n", PROBLEMA, n, ops, melhor);

        free(orig); free(work); free(tmp);
    }

    fclose(f);
    printf("Resultados gravados em %s\n", arq);
    return 0;
}