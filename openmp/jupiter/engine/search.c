#include <stdio.h>
#include <string.h>
#include "notation.h"
#include "board.h"
#include "logging.h"

#include <stdlib.h>

int MAX_DEPTH;
int n = 1;
char* asd;

static int32_t get_minimax(Node_t *node)
{
    Node_t *aux = node->child;
    int32_t minmax = -10000 * node->turn;

    while (aux != NULL) {
        if ((node->turn == BLACK && aux->value < minmax) ||
            (node->turn == WHITE && aux->value > minmax)) { 
            minmax = aux->value;
        }
        aux = aux->next;
    }

    return minmax;
}

/* static void search_ser(Node_t *root) */
/* { */
/*     Node_t *aux = root; */

/*     while(aux != NULL) { */
/*         if (aux->child != NULL) { */
/*             if (aux->child->child == NULL) { */
/*                 aux->value = get_minimax(aux); */
/*             } else { */
/*                 search_ser(aux->child); */
/*                 aux->value = get_minimax(aux); */
/*             } */
/*         } */
/*         aux = aux->next; */
/*     } */
/* } */

static void search_aux(Node_t *root, int* n)
{
    Node_t *aux = root;

    while(aux != NULL) {
        if (aux->child != NULL) {
            if (aux->child->child == NULL) {
                aux->value = get_minimax(aux);
            } else {
#pragma omp atomic
                (*n)++;
#pragma omp task if (*n < MAX_DEPTH)
                search_aux(aux->child, n);
#pragma omp taskwait
                aux->value = get_minimax(aux);
            }
        }
        aux = aux->next;
    }
}

static void search(Node_t *root)
{
        MAX_DEPTH = strtol(getenv("MAX_DEPTH"), &asd, 10);
        n = 1;
        search_aux(root, &n);
}

static retval_t get_move(Node_t *node, char *move)
{
    Node_t *aux = node->child;
    int32_t score = node->value;

    while (aux->value != score && aux != NULL) {
        aux = aux->next;
    }

    if (aux == NULL) {
        memset(move, 0x0, NOTATION_STR_LEN);
        return RV_ERROR;
    }
    strncpy(move, aux->notation, NOTATION_STR_LEN);
    return RV_SUCCESS;
}

void get_best_move(Node_t *node, char *move)
{
    search(node);
    get_move(node, move);
}

