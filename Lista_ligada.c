#include <stdio.h>
#include <stdlib.h>
#include "Lista_ligada.h"

// Fun��o que remove o primeiro elemento da lista
lista *remover(lista *apontador)
{
	lista *lap = (lista *)apontador->proximo;
	free(apontador);
	return lap;
}

// Fun��o que adiciona novo elemento � lista, ordenando a mesma por tempo
lista *adicionar(lista *apontador, int n_tipo, double n_tempo)
{
	lista *lap = apontador;
	lista *ap_aux, *ap_next;
	if (apontador == NULL)
	{
		apontador = (lista *)malloc(sizeof(lista));
		apontador->proximo = NULL;
		apontador->tipo = n_tipo;
		apontador->tempo = n_tempo;
		return apontador;
	}
	else
	{
		if (apontador->tempo > n_tempo)
		{
			ap_aux = (lista *)malloc(sizeof(lista));
			ap_aux->tipo = n_tipo;
			ap_aux->tempo = n_tempo;
			ap_aux->proximo = (struct lista *)apontador;
			return ap_aux;
		}

		ap_next = (lista *)apontador->proximo;
		while (apontador != NULL)
		{
			if ((ap_next == NULL) || ((ap_next->tempo) > n_tempo))
				break;
			apontador = (lista *)apontador->proximo;
			ap_next = (lista *)apontador->proximo;
		}
		ap_aux = (lista *)apontador->proximo;
		apontador->proximo = (struct lista *)malloc(sizeof(lista));
		apontador = (lista *)apontador->proximo;
		if (ap_aux != NULL)
			apontador->proximo = (struct lista *)ap_aux;
		else
			apontador->proximo = NULL;
		apontador->tipo = n_tipo;
		apontador->tempo = n_tempo;
		return lap;
	}
}

// Fun��o que imprime no ecra todos os elementos da lista
void imprimir(lista *apontador)
{
	if (apontador == NULL)
		printf("Lista vazia!\n");
	else
	{
		while (apontador != NULL)
		{
			printf("Tipo=%d\tTempo=%lf\n", apontador->tipo, apontador->tempo);
			apontador = (lista *)apontador->proximo;
		}
	}
}
