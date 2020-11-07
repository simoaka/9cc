#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "9cc.h"

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); /* output pos number of spaces. */
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void enque(Vector *vector, void *data)
{
    Element *e = calloc(1, sizeof(Element));
    e->data = data;

    if (vector->tail)
        vector->tail->next = e;
    else
        vector->head = e;
    vector->tail = e;
}

void * deque(Vector *vector)
{
    if (!vector->head)
        return NULL;

    void *data = vector->head->data;
    vector->head = vector->head->next;
    return data;
}
