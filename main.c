#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

/* #DEFINE'S -----------------------------------------------------------------*/
#define DEFAULT_TOTAL_CAPACITY 15
#define DEFAULT_DISTINCT_CAPACITY 5

/* TYPE DEFINITIONS ----------------------------------------------------------*/
typedef unsigned int action_t; // an action is identified by an integer

typedef struct event event_t; // an event ...
struct event
{                  // ... is composed of ...
    action_t actn; // ... an action that triggered it and ...
    event_t *next; // ... a pointer to the next event in the trace
};

typedef struct
{                  // a trace is a linked list of events
    event_t *head; // a pointer to the first event in this trace
    event_t *foot; // a pointer to the last event in this trace
    int freq;      // the number of times this trace was observed
} trace_t;

typedef struct
{                  // an event log is an array of distinct traces
                   //     sorted lexicographically
    trace_t *trcs; // an array of traces
    int ndtr;      // the number of distinct traces in this log
    int cpct;      // the capacity of this event log as the number
                   //     of  distinct traces it can hold
} log_t;

typedef action_t **DF_t; // a directly follows relation over actions

void printTrace(trace_t *tr)
{
    printf("Printing trace\n");
    event_t *evt = tr->head;
    while (1)
    {
        printf("Started");
        printf("%c", evt->actn);
        if (evt == tr->foot)
        {
            printf("\n");
            break;
        }
        printf(",");
        evt = evt->next;
    }
}

void addEvt(trace_t *tr, event_t *evt)
{
    if (tr->head == NULL)
    {

        tr->head = evt;
        printf("Head = %d", tr->head->actn);
        tr->foot = tr->head;
    }
    else
    {
        tr->foot->next = evt;
        tr->foot = evt;
    }
}

/* Load all the events and traces-----------------------------------------------------------------*/

trace_t *getTrace(char *str)
{
    trace_t *tr = (trace_t *)malloc(sizeof(trace_t *));
    for (int i = 0; str[i]; i++)
    {
        if (isalpha(str[i]))
        {
            action_t a = str[i];
            event_t *evt = (event_t *)malloc(sizeof(event_t *));
            evt->actn = a;
            addEvt(tr, evt);
        }
    }
    return tr;
}

trace_t **initTrcsFromFile(char *filename)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // log_t *log = (log_t *)malloc(sizeof(log_t *));
    int trcsCap = DEFAULT_TOTAL_CAPACITY;
    trace_t **trcs = (trace_t **)malloc(sizeof(trace_t *) * trcsCap);
    printf("calc size = %d x %d = %d ; real size = %d\n", sizeof(trace_t *), trcsCap, sizeof(trace_t *) * trcsCap, sizeof(trcs));
    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        // printf("index  = %d \n", i);
        trace_t *tr = getTrace(line);
        if (i == trcsCap)
        {
            trcsCap = trcsCap * 2;
            realloc(trcs, sizeof(trace_t *) * trcsCap);
        }

        printTrace(trcs[i]);
        trcs[i++] = tr;
    }

    fclose(fp);
    if (line)
        free(line);
    realloc(trcs, sizeof(trace_t *) * (i));
    printf("Size =%d %d\n", sizeof(trcs) / sizeof(trace_t *), i);
    return trcs;
}

/* WHERE IT ALL HAPPENS ------------------------------------------------------*/
int main(int argc, char *argv[])
{
    trace_t **trcs = initTrcsFromFile("test0.txt");
    int size = sizeof(trcs) / sizeof(trace_t *);
    printf("%d", size);
    for (int i = 0; i < size; i++)
    {
        trace_t *tr = trcs[i];
        printTrace(tr);
    }
}