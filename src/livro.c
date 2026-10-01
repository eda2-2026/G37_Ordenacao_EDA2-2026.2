
#include <stdio.h>   /* FILE, fopen, fgets, fclose, printf */
#include <stdlib.h>  /* strtol */
#include <string.h>  /* strlen, strncpy, strtok */
#include <ctype.h>   /* tolower */

#include "livro.h"


#define TAM_LINHA 512

static void remover_fim_de_linha(char *linha)
{
    size_t n = strlen(linha);
    while (n > 0 && (linha[n - 1] == '\n' || linha[n - 1] == '\r')) {
        n--;
        linha[n] = '\0';
    }
}

static void copiar_campo(char *destino, size_t tamanho, const char *origem)
{
    strncpy(destino, origem, tamanho - 1);
    destino[tamanho - 1] = '\0';
}

static int igual_sem_caixa(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        unsigned char ca = (unsigned char) *a;
        unsigned char cb = (unsigned char) *b;
        if (tolower(ca) != tolower(cb)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static int linha_em_branco(const char *linha)
{
    while (*linha != '\0') {
        if (*linha != ' ' && *linha != '\t') {
            return 0;
        }
        linha++;
    }
    return 1;
}


Tipo tipo_de_texto(const char *texto)
{
    if (texto == NULL) {
        return ROMANCE;
    }
    if (igual_sem_caixa(texto, "MISTERIO")) {
        return MISTERIO;
    }
    if (igual_sem_caixa(texto, "AVENTURA")) {
        return AVENTURA;
    }
    return ROMANCE;
}

const char *tipo_para_texto(Tipo t)
{
    switch (t) {
        case MISTERIO: return "Misterio";
        case AVENTURA: return "Aventura";
        case ROMANCE:  return "Romance";
        default:       return "Romance";  /* nao deve acontecer */
    }
}

int ler_csv(const char *caminho, Livro *livros, int max)
{
    FILE *arquivo;
    char  linha[TAM_LINHA];
    int   total = 0;

    if (caminho == NULL || livros == NULL || max <= 0) {
        return -1;
    }

    arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        return -1;
    }

    if (fgets(linha, sizeof linha, arquivo) == NULL) {
        fclose(arquivo);
        return 0;
    }

    while (total < max && fgets(linha, sizeof linha, arquivo) != NULL) {
        char *campo_id;
        char *campo_nome;
        char *campo_descricao;
        char *campo_tipo;
        char *resto;
        long  valor_id;

        remover_fim_de_linha(linha);

        if (linha_em_branco(linha)) {
            continue;
        }

        campo_id        = strtok(linha, ";");
        campo_nome      = strtok(NULL, ";");
        campo_descricao = strtok(NULL, ";");
        campo_tipo      = strtok(NULL, ";");

        if (campo_id == NULL || campo_nome == NULL ||
            campo_descricao == NULL || campo_tipo == NULL) {
            continue;
        }

        valor_id = strtol(campo_id, &resto, 10);
        if (resto == campo_id || *resto != '\0') {
            continue;
        }

        livros[total].id = (int) valor_id;
        copiar_campo(livros[total].nome,
                     sizeof livros[total].nome, campo_nome);
        copiar_campo(livros[total].descricao,
                     sizeof livros[total].descricao, campo_descricao);
        livros[total].tipo = tipo_de_texto(campo_tipo);

        total++;
    }

    fclose(arquivo);
    return total;
}


void imprimir_livros(const Livro *livros, int n)
{
    int i;

    if (livros == NULL || n <= 0) {
        printf("Catalogo vazio.\n");
        return;
    }

    printf("%-4s %-32s %-9s %s\n", "ID", "NOME", "TIPO", "DESCRICAO");
    printf("---- -------------------------------- --------- ---------\n");

    for (i = 0; i < n; i++) {
        printf("%-4d %-32s %-9s %s\n",
               livros[i].id,
               livros[i].nome,
               tipo_para_texto(livros[i].tipo),
               livros[i].descricao);
    }

    printf("\nTotal: %d livro(s).\n", n);
}
