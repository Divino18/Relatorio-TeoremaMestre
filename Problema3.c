/*
 * problema3.c
 * Problema 3 - Caso 3 do Teorema Mestre
 * Contagem de inversoes por divisao e conquista com combinacao quadratica.
 *
 * Recorrencia:  T(n) = 2T(n/2) + Theta(n^2)
 *   a = 2, b = 2, f(n) = n^2/4 = Theta(n^2), n^(log_b a) = n
 *   Caso 3 (epsilon = 1, regularidade com c = 1/2)  =>  T(n) = Theta(n^2)
 *
 * Contagem de operacoes:
 *   - 1 operacao por comparacao a[i] > a[j] entre um elemento da metade
 *     esquerda e um da metade direita (combinacao: (n/2)*(n/2) = n^2/4).
 *   - caso base (n = 1): nenhuma operacao.
 *   => T(1) = 0, T(n) = 2T(n/2) + n^2/4  =>  T(n) = n(n-1)/2
 *   A contagem NAO depende dos dados (so do tamanho n).
 *   O vetor NAO e modificado pelo algoritmo.
 *
 * Uso:    ./problema3 [n_max] [arquivo_csv]
 *   n_max: maior tamanho (potencia de 2). Padrao: 16384 (2^14)
 *   Executa n = 1, 2, 4, ..., n_max e grava em arquivo CSV (modo append).
 *
 * Compilar: gcc -O2 -Wall -o problema3 problema3.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef long long ll;

#define REPETICOES 5            /* repeticoes de cada n para medir o tempo */
#define PROBLEMA   "P3"

static ll operacoes = 0;        /* contador global de operacoes */

/* Retorna o tempo atual em milissegundos (relogio monotonico). */
static double agora_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

/*
 * Conta as inversoes de a[lo..hi) por divisao e conquista.
 * (hi - lo) deve ser potencia de 2.
 * Combinacao quadratica: compara todo elemento da esquerda com todo
 * elemento da direita  ->  f(n) = (n/2)*(n/2) = n^2/4.
 */
static ll conta_inversoes(const int *a, int lo, int hi)
{
    int n = hi - lo;
    if (n <= 1) return 0;                       /* caso base: T(1) = 0 */

    int mid = lo + n / 2;

    /* 2 chamadas recursivas de tamanho n/2  ->  2T(n/2) */
    ll inv = conta_inversoes(a, lo, mid) + conta_inversoes(a, mid, hi);

    /* Combinacao: inversoes cruzadas, f(n) = n^2/4 comparacoes */
    ll cmp = 0;
    for (int i = lo; i < mid; i++)
        for (int j = mid; j < hi; j++) {
            cmp++;
            if (a[i] > a[j]) inv++;
        }
    operacoes += cmp;

    return inv;
}

/*
 * Referencia independente (nao entra na contagem nem no tempo):
 * conta inversoes por merge sort em O(n log n). Ordena a copia 'a'.
 */
static ll inversoes_ref(int *a, int *tmp, int lo, int hi)
{
    if (hi - lo <= 1) return 0;
    int mid = lo + (hi - lo) / 2;
    ll inv = inversoes_ref(a, tmp, lo, mid) + inversoes_ref(a, tmp, mid, hi);

    int i = lo, j = mid, p = lo;
    while (i < mid && j < hi) {
        if (a[i] <= a[j]) tmp[p++] = a[i++];
        else { tmp[p++] = a[j++]; inv += mid - i; }
    }
    while (i < mid) tmp[p++] = a[i++];
    while (j < hi)  tmp[p++] = a[j++];
    memcpy(a + lo, tmp + lo, (size_t)(hi - lo) * sizeof(int));
    return inv;
}

int main(int argc, char *argv[])
{
    ll n_max = (argc > 1) ? atoll(argv[1]) : 16384;
    const char *arq = (argc > 2) ? argv[2] : "resultados.csv";

    if (n_max < 1 || (n_max & (n_max - 1)) != 0) {
        fprintf(stderr, "n_max deve ser potencia de 2 (recebido: %lld)\n", n_max);
        return 1;
    }

    FILE *f = fopen(arq, "a");
    if (!f) { perror("fopen"); return 1; }
    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0)                                   /* arquivo novo */
        fprintf(f, "problema,n,operacoes,tempo_ms\n");

    srand(42);                                           /* reprodutivel */

    printf("%9s %16s %16s %12s\n", "n", "operacoes", "n(n-1)/2", "tempo(ms)");

    for (ll n = 1; n <= n_max; n *= 2) {
        int *a   = malloc((size_t)n * sizeof(int));
        int *cp  = malloc((size_t)n * sizeof(int));
        int *tmp = malloc((size_t)n * sizeof(int));
        if (!a || !cp || !tmp) { fprintf(stderr, "malloc falhou\n"); return 1; }

        for (ll i = 0; i < n; i++) a[i] = rand() % 1000000;

        double melhor = -1.0;
        ll ops = 0, inv = 0;
        for (int r = 0; r < REPETICOES; r++) {
            operacoes = 0;
            double t0 = agora_ms();
            inv = conta_inversoes(a, 0, (int)n);
            double dt = agora_ms() - t0;
            if (melhor < 0 || dt < melhor) melhor = dt;  /* menor tempo = menos ruido */
            ops = operacoes;
        }

        /* Validacoes */
        memcpy(cp, a, (size_t)n * sizeof(int));
        ll inv_ref = inversoes_ref(cp, tmp, 0, (int)n);
        if (inv != inv_ref) {
            fprintf(stderr, "ERRO: inversoes %lld != referencia %lld (n=%lld)\n",
                    inv, inv_ref, n);
            return 1;
        }

        ll esperado = n * (n - 1) / 2;
        if (ops != esperado)
            fprintf(stderr, "AVISO: contagem %lld != formula %lld (n=%lld)\n",
                    ops, esperado, n);

        printf("%9lld %16lld %16lld %12.3f\n", n, ops, esperado, melhor);
        fprintf(f, "%s,%lld,%lld,%.6f\n", PROBLEMA, n, ops, melhor);

        free(a); free(cp); free(tmp);
    }

    fclose(f);
    printf("Resultados gravados em %s\n", arq);
    return 0;
}