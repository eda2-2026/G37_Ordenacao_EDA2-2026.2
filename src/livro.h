
#ifndef LIVRO_H
#define LIVRO_H

typedef enum { ROMANCE, MISTERIO, AVENTURA } Tipo;

typedef struct {
    int  id;
    char nome[50];
    char descricao[100];
    Tipo tipo;
} Livro;

int ler_csv(const char *caminho, Livro *livros, int max);

Tipo tipo_de_texto(const char *texto);

const char *tipo_para_texto(Tipo t);

void imprimir_livros(const Livro *livros, int n);

#endif
