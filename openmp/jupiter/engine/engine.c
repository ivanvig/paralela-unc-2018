#include <stdio.h>
#include <omp.h>
#include "board.h"
#include "generation.h"
#include "evaluation.h"
#include "notation.h"
#include "engine.h"
#include "node.h"
#include "delay.h"
#include "search.h"
#include "fen.h"
#include "logging.h"

#include <stdlib.h> // getenv

/* #define MAX_DEPTH 4*10e10 */
char* asd;
unsigned int MAX_DEPTH;
unsigned int n;

static void (*log_func)(const char *info) = NULL;

typedef struct {
    char mov[NOTATION_STR_LEN];
    uint32_t nodes_count;
    uint32_t score;
    uint32_t nps;
    uint32_t time;
} engine_info_t;

static void generate_nodes_ser(Node_t *node)
{
    Node_t *aux = node;

    while(aux != NULL) {
        if (aux->child != NULL) {
            generate_nodes_ser(aux->child);
        } else {
            get_moves(aux);
        }
        aux = aux->next;
    }
}


/* ESTO ARRANCA CON UN NODO SIN HIJOS ENTONCES HACES UN GET_MOVES() */
/*Y TE GENERA TODOS LOS MOVIMIENTOS POSIBLES POR CADA PIEZA Y VUELVE, EN CASO DE SER EL DEPTH */
/* MAYOR QUE 1, VUELVE A ENTRAR Y VUELVE A GENERAR TOODS LOS MOVIMIENTOS POR CADA HIJO, */
/*CUANDO SE CUMPLE EL DEPTH, SE BUSCA LA MEJOR JUGADA Y SE DESTRUYE EL GRAFO */
static void generate_nodes_aux(Node_t *node, unsigned int* n)
{
    /* printf("%d\n",*n); */
    /* if (*n >= MAX_DEPTH) { */
    /*     generate_nodes_ser(node); */
    /*     return; */
    /* } */

    Node_t *aux = node;

    while(aux != NULL) {
        if (aux->child != NULL) {
#pragma omp atomic
            (*n)++;
            /* *n += 1; */
#pragma omp task if (*n < MAX_DEPTH)
            generate_nodes_aux(aux->child, n);
        } else {
            get_moves(aux);
        }
        aux = aux->next;
    }
}
static void generate_nodes(Node_t *node)
{
    MAX_DEPTH = strtol(getenv("MAX_DEPTH"), &asd, 10);
    n = 1;
    generate_nodes_aux(node, &n);
}
/* { */
/*     Node_t *aux = node; */

/*     while(aux != NULL) { */
/*         if (aux->child != NULL) { */
/* #pragma omp task */
/*             generate_nodes(aux->child); */
/*         } else { */
/*             get_moves(aux); */
/*         } */
/*         aux = aux->next; */
/*     } */
/* } */

static engine_info_t engine_think(Node_t *node)
{
    engine_info_t info;
    clock_t start, end;
    double start_time, end_time;

    start = get_clock_ms();

    start_time = omp_get_wtime();
    printf("N_THREADS: %d\n", omp_get_max_threads());
#pragma omp parallel
    {
#pragma omp single
        {
            generate_nodes(node);
#pragma omp taskwait
            get_best_move(node, info.mov);
        }
    }
    /* get_best_move(node, info.mov); */

    printf("TIEMPO: %f\n", omp_get_wtime() - start_time);

    end = get_clock_ms();
    info.time = clock_diff_ms(end, start);
    info.nodes_count = get_tree_count(node) - 1;
    if (info.time > 0) {
        info.nps = (info.nodes_count * 1000) / info.time;
    } else {
        info.nps = 0;
    }

    return info;
}

static bool keep_running(engine_cfg_t cfg, uint32_t elapsed_time, uint8_t current_depth)
{
    if (cfg.mode == ENGINE_INFINITE) {
        //ask for command stop
    }
    if (cfg.mode == ENGINE_TIME) {
        if ((cfg.data.time / 5) < elapsed_time) {
            return false;
        }
    }
    if (cfg.mode == ENGINE_DEPTH) {
        if (current_depth >= cfg.data.depth) {
            return false;
        }
    }
        
    return true;        
}

static void log_info(engine_info_t info, uint8_t depth, uint32_t elapsed_time)
{
    char msg[1000];

    if (log_func != NULL) {
        sprintf(msg, "info depth %d score cp %d time %d nodes %d nps %d",
                depth, info.score, elapsed_time, info.nodes_count,
                info.nps);
        log_func(msg);
    }
}

void engine_set_log_func(void *func)
{
    log_func = func;
}

char* engine_go(Node_t *node, engine_cfg_t cfg)
{
    static engine_info_t info;
    uint32_t elapsed_time = 0;
    uint8_t current_depth = 0;

    char fen[MAX_FEN_LEN];
    get_fen_from_node(node, fen);
    LOG2(DEBUG, "Board: ", fen);

    while (keep_running(cfg, elapsed_time, current_depth)) {
        info = engine_think(node);
        current_depth++;
        elapsed_time += info.time;
        log_info(info, current_depth, elapsed_time);
    } 

    delete_node(node);
    return info.mov;
} 
