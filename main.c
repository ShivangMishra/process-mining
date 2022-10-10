#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

/* #DEFINE'S -----------------------------------------------------------------*/

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

/* WHERE IT ALL HAPPENS ------------------------------------------------------*/
int main(int argc, char *argv[])
{
    trace_t **trcs;
    initFromFile("test0.txt");
}