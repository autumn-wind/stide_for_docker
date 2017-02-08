#ifndef __SBUF_H
#define __SBUF_H

#include <malloc.h>

#include "syscall_info.h"

typedef struct {
    pro_con_bfr *buf;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;

static inline void sbuf_init(sbuf_t *sp, int n) {
    sp->buf = (pro_con_bfr *)malloc(n * sizeof(pro_con_bfr));
    sp->n = n;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
    sem_init(&sp->slots, 0, n);
    sem_init(&sp->items, 0, 0);
}

static inline void sbuf_free(sbuf_t *sp) {
    free(sp->buf);
    sem_destroy(&sp->mutex);
    sem_destroy(&sp->slots);
    sem_destroy(&sp->items);
}

static inline void sbuf_insert(sbuf_t *sp, pro_con_bfr item) {
    sem_wait(&sp->slots);
    sem_wait(&sp->mutex);
    sp->buf[(sp->rear++) % (sp->n)] = item;
    sem_post(&sp->mutex);
    sem_post(&sp->items);
}

static inline pro_con_bfr sbuf_remove(sbuf_t *sp) {
    pro_con_bfr item;
    sem_wait(&sp->items);
    sem_wait(&sp->mutex);
    item = sp->buf[(sp->front++) % (sp->n)];
    sem_post(&sp->mutex);
    sem_post(&sp->slots);
    return item;
}

#endif
