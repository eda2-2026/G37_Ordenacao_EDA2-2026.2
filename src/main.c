/* main.c - le o CSV, ordena com o bucketSort e imprime JSON no stdout.
 *
 * Uso: ./livraria data/livros.csv
 *
 * Erros vao para o stderr e o programa sai com codigo != 0, para que o
 * server.py consiga separar o JSON (stdout) da mensagem de erro (stderr). */

#include <stdio.h>

#include "livro.h"
#include "bucket.h"

#define MAX_LIVROS 1000

/* O JSON e o CSS da pagina usam o tipo em minuscula ("romance" vira a
 * classe .romance da lombada). tipo_para_texto() devolve capitalizado,
 * por isso esta versao propria aqui. */
static const char *tipo_para_json(Tipo t)
{
    switch (t) {
        case MISTERIO: return "misterio";
        case AVENTURA: return "aventura";
        case ROMANCE:  return "romance";
        default:       return "romance";
    }
}

/* Imprime a string entre aspas, escapando o que o JSON nao aceita cru:
 * a aspa dupla, a barra invertida e os caracteres de controle.
 * Os bytes do UTF-8 (>= 0x80) saem inalterados. */
static void json_string(const char *s)
{
    putchar('"');
    for (; *s != '\0'; s++) {
        unsigned char c = (unsigned char) *s;
        if (c == '"' || c == '\\') {
            printf("\\%c", c);
        } else if (c < 0x20) {
            printf("\\u%04x", c);
        } else {
            putchar(c);
        }
    }
    putchar('"');
}

/* Um livro: {"id":1,"nome":"...","descricao":"...","tipo":"aventura"} */
static void json_livro(const Livro *l)
{
    printf("{\"id\":%d,\"nome\":", l->id);
    json_string(l->nome);
    printf(",\"descricao\":");
    json_string(l->descricao);
    printf(",\"tipo\":");
    json_string(tipo_para_json(l->tipo));
    putchar('}');
}

int main(int argc, char *argv[])
{
    static Livro  chegada[MAX_LIVROS];    /* os livros na ordem do arquivo */
    static Livro *ordenados[MAX_LIVROS];  /* ponteiros, que o bucketSort reordena */
    int n, i, b;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <caminho-do-csv>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s data/livros.csv\n", argv[0]);
        return 1;
    }

    /* Etapa 1: leitura (a mesma de sempre, em livro.c). */
    n = ler_csv(argv[1], chegada, MAX_LIVROS);
    if (n < 0) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo '%s'.\n",
                argv[1]);
        return 1;
    }

    /* Etapa 2: o bucketSort reordena o vetor de PONTEIROS, nao os livros.
     * Assim o vetor 'chegada' continua intacto, na ordem do CSV, e serve
     * para a secao "chegada" do JSON. */
    for (i = 0; i < n; i++) {
        ordenados[i] = &chegada[i];
    }
    if (n > 0) {
        bucketSort(ordenados, n);
    }

    /* Etapa 3: "chegada" - ordem original do arquivo. */
    printf("{\n  \"chegada\": [");
    for (i = 0; i < n; i++) {
        if (i > 0) {
            printf(",");
        }
        printf("\n    ");
        json_livro(&chegada[i]);
    }
    printf("\n  ],\n");

    /* Etapa 4: "baldes" - sempre os 27 (#, A..Z), mesmo os vazios.
     * Depois do bucketSort o vetor ja esta agrupado por balde; aqui so
     * filtramos cada balde pelo indiceBalde do titulo. */
    printf("  \"baldes\": [");
    for (b = 0; b < NUM_BALDES; b++) {
        char letra = (b == 0) ? '#' : (char) ('A' + b - 1);
        int  primeiro = 1;

        if (b > 0) {
            printf(",");
        }
        printf("\n    {\"letra\":\"%c\",\"livros\":[", letra);

        for (i = 0; i < n; i++) {
            if (indiceBalde(ordenados[i]->nome) != b) {
                continue;
            }
            if (!primeiro) {
                printf(",");
            }
            primeiro = 0;
            json_livro(ordenados[i]);
        }

        printf("]}");
    }
    printf("\n  ]\n}\n");

    return 0;
}
