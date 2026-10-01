#ifndef BUCKET_H
#define BUCKET_H

#include "livro.h"

#define NUM_BALDES 27

/* indice do balde do titulo: 0 = '#', 1..26 = A..Z */
int indiceBalde(const char *titulo);

/* ordena os ponteiros de arr pelo campo nome */
void bucketSort(Livro *arr[], int n);

#endif
