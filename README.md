# G37_Ordenacao — Livraria Virtual com Bucket Sort

**Disciplina:** Estruturas de Dados 2 (EDA2) — 2026.2
**Professor:** Maurício Serrano
**Trabalho:** T2 — Ordenação
**Grupo:** Gabriel Sampaio Fae (231011382)
**Repositório:** `github.com/eda2-2026/G37_Ordenacao_EDA2-2026.2`

---

## 1. Problema

Dado um catálogo de livros em CSV, exibir os livros **organizados em ordem
alfabética pelo título**, agrupados em **baldes de A a Z** (como as
prateleiras de uma livraria), além da lista final completamente ordenada.

Exemplo de saída esperada:

```
[A] A Hora da Estrela | Admirável Mundo Novo
[D] Drácula | Duna
[O] O Hobbit | O Pequeno Príncipe
[#] 1984
```

## 2. Técnica — Bucket Sort

O Bucket Sort funciona em três etapas:

1. **Distribuição:** cada livro vai para um balde de acordo com a primeira
   letra do título (`A`–`Z`), mais um balde extra `#` para títulos que
   começam com número ou símbolo (27 baldes no total).
2. **Ordenação interna:** cada balde é ordenado com **selection Sort**, que é
   eficiente para conjuntos pequenos e quase ordenados.
3. **Concatenação:** os baldes são percorridos em ordem (`#`, `A`, `B`, ...,
   `Z`) e unidos, formando a lista final ordenada.

```c
int  indice_balde(const char *titulo);          /* 0 = '#', 1..26 = A..Z */
void insertion_sort(Livro *v, int n);           /* ordena um balde */
void bucket_sort(Livro *v, int n, Balde baldes[27]);
```

Detalhes a documentar ao longo do desenvolvimento:

- **Normalização:** a comparação ignora maiúsculas/minúsculas e acentos
  (`"Ética"` cai no balde `E`). Como C trata UTF-8 byte a byte, usar uma
  tabela simples de conversão para os acentos mais comuns (á, à, â, ã, é, ê,
  í, ó, ô, õ, ú, ç).
- **Artigos iniciais:** decidir se `"O Hobbit"` fica em `O` ou em `H`
  (decisão do grupo: _a preencher_).
- **Desempate:** títulos iguais são desempatados por `id`.
- **Estrutura dos baldes:** vetor dinâmico por balde (`malloc`/`realloc`)
  ou lista encadeada (decisão do grupo: _a preencher_).

## 3. Linguagem e build

C (padrão **C11**), compilando **sem warnings** com `gcc -Wall -Wextra -std=c11`.

## 4. Estrutura do projeto

```
.
├── data/
│   └── livros.csv        # catálogo (id,nome,descricao,tipo)
├── src/
│   ├── livro.h / livro.c       # struct Livro e leitura do CSV
│   ├── bucket.h / bucket.c     # bucket sort + insertion sort
│   ├── bubble.c                # bubble sort (comparação)
│   ├── gerador.c               # gera CSVs grandes para o benchmark
│   └── main.c                  # interface
├── Makefile
└── README.md
```

Formato do CSV:

```
id,titulo,autor,ano
1,Duna,Frank Herbert,1965
2,Drácula,Bram Stoker,1897
```

## 5. Como executar

A aplicação tem três peças: o programa em C faz a ordenação, o `server.py`
executa esse programa e serve a página, e a página mostra o resultado.

```bash
make                    # compila e gera o executável "livraria"
python3 server.py       # sobe o servidor em http://localhost:8000
```

Depois abra **http://localhost:8000** no navegador. A página começa
mostrando só a seção **Chegada** (os livros na ordem do CSV); o botão
**Visualizar livros** revela a **Estante** com os 27 baldes.

O formulário *Adicionar livro* grava no fim do `data/livros.csv` e recarrega
os dados — o livro novo aparece na chegada. Para parar o servidor, `Ctrl+C`.

Rodar o programa em C sozinho, sem o servidor, imprime o JSON no terminal:

```bash
./livraria data/livros.csv
```

### Observações de ambiente

- **Não há compilador instalado no Windows desta máquina.** Compile pelo
  WSL (`wsl`, depois `cd /mnt/d/.Faculdade/G37_Ordenacao_EDA2-2026.2`), que
  tem gcc e make. Para um gcc nativo, `winget install -e --id
  BrechtSanders.WinLibs.POSIX.UCRT` — o comando passa a ser `mingw32-make`.
- No Windows o comando do Python geralmente é `python server.py`, não
  `python3`. O `server.py` detecta sozinho se o executável é `livraria.exe`
  ou `livraria`.
- O servidor escuta só em `127.0.0.1`: é uma aplicação local, não fica
  exposta na rede.
- Se a página mostrar *"Executavel 'livraria' nao encontrado"*, falta rodar
  o `make`.

## 6. Roteiro de desenvolvimento

| Dia | Tarefa |
| --- | --- |
| 1 | Criar repositório, `Makefile`, struct `Livro` e o `livros.csv` com ~30 livros |
| 2 | Leitura do CSV + função de normalização (minúsculas e sem acento) |
| 3 | `indice_balde`, distribuição nos baldes e insertion sort em cada balde |
| 4 | Concatenação, `main` com exibição dos baldes e lista final; preencher seção 5 |
| 5 | Selection sort em cada balde, gerador de CSV grande e medição de tempo (`clock()`) |
| 6 | Rodar benchmark, preencher seção 6 |

