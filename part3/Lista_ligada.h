#ifndef _LISTA
#define _LISTA

#define CHEGADA 0
#define PARTIDA 1
#define GP 2
#define GP_AS 3
#define AS 4

// Defini��o da estrutura da lista
typedef struct{
	int tipo;
	double tempo;
	struct lista * proximo;
} lista;

lista * remover (lista * apontador);
lista * adicionar (lista * apontador, int n_tipo, double n_tempo);
void imprimir (lista * apontador);

#endif