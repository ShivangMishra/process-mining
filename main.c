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
    event_t *evt = tr->head;
    while (1)
    {
        printf("%c", evt->actn);
        if (evt == tr->foot)
        {
            printf("\n");
            break;
        }
        evt = evt->next;
    }
}

void addEvt(trace_t *tr, event_t *evt)
{
    if (tr->head == NULL)
    {
        tr->head = evt;
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
    trace_t *tr = malloc(sizeof(trace_t *));
    for (int i = 0; str[i]; i++)
    {
        if (isalpha(str[i]))
        {
            action_t a = str[i];
            event_t *evt = malloc(sizeof(event_t *));
            evt->actn = a;
            addEvt(tr, evt);
        }
    }
    return tr;
}

trace_t **initTrcsFromFile(trace_t **trcs, int *trcsCap, char *filename)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (i == *trcsCap)
        {
            *trcsCap = *trcsCap * 2;
            trcs = realloc(trcs, sizeof(trace_t *) * *trcsCap);
        }
        trcs[i] = getTrace(line);
        i++;
    }

    fclose(fp);
    if (line)
        free(line);
    *trcsCap = i;
    trcs = realloc(trcs, sizeof(trace_t *) * *trcsCap);
    return trcs;
}

/* Stage 0 -------------------------------------------------------------------------------------*/

action_t *findDistinctEvents(trace_t **trcs, int size, int *nDistEvts)
{
    int distCap = 2;
    int nDist = 0;
    action_t *distEvts = malloc(sizeof(action_t) * distCap);

    for (int i = 0; i < size; i++)
    {
        event_t *cur = trcs[i]->head;
        while (1)
        {
            int isDist = 1;
            for (int j = 0; j < nDist; j++)
            {
                if (distEvts[j] == cur->actn)
                {
                    isDist = 0;
                    break;
                }
            }
            if (isDist)
            {
                if (nDist == distCap)
                {
                    distCap *= 2;
                    distEvts = realloc(distEvts, distCap);
                }
                distEvts[nDist++] = cur->actn;
            }
            if (cur == trcs[i]->foot)
            {
                break;
            }
            cur = cur->next;
        }
    }
    *nDistEvts = nDist;
    return distEvts;
}

int countEvts(trace_t **trcs, int size)
{
    int nEvts = 0;
    for (int i = 0; i < size; i++)
    {
        event_t *cur = trcs[i]->head;
        while (1)
        {
            nEvts++;
            if (cur == trcs[i]->foot)
            {
                break;
            }
            cur = cur->next;
        }
    }
    return nEvts;
}

int *calcEvtFreq(trace_t **trcs, int size, action_t *actns, int nDistEvts)
{
    int *evtFreqs = malloc(sizeof(int) * nDistEvts);
    for (int j = 0; j < nDistEvts; j++)
    {
        evtFreqs[j] = 0;
    }
    for (int i = 0; i < size; i++)
    {
        event_t *cur = trcs[i]->head;
        while (1)
        {
            for (int j = 0; j < nDistEvts; j++)
            {
                if (actns[j] == cur->actn)
                {
                    evtFreqs[j]++;
                }
            }
            if (cur == trcs[i]->foot)
            {
                break;
            }
            cur = cur->next;
        }
    }
    return evtFreqs;
}

int equals(trace_t *tr1, trace_t *tr2)
{
    event_t *evt1 = tr1->head;
    event_t *evt2 = tr2->head;
    // printf("--------------------\nComparing traces : \n");
    // printTrace(tr1);
    // printTrace(tr2);

    int result = 0;
    while (1)
    {
        if (evt1 == tr1->foot && evt2 == tr2->foot)
        {
            result = evt1->actn == evt2->actn;
            break;
        }
        // printf("%c", evt->actn);
        if ((evt1 == tr1->foot) || (evt2 == tr2->foot))
        {
            result = 0;
            break;
        }
        if (evt1->actn != evt2->actn)
        {
            result = 0;
            break;
        }
        evt1 = evt1->next;
        evt2 = evt2->next;
    }
    return result;
}

int countDistinctTraces(trace_t **trcs, int size)
{
    int nDist = 1;
    for (int i = 1; i < size; i++)
    {
        int j = 0;
        for (j = 0; j < i; j++)
        {
            if (equals(trcs[i], trcs[j]))
                break;
        }

        // If not printed earlier, then print it
        if (i == j)
            nDist++;
    }
    return nDist;
}

void calcTrcsFreq(trace_t **trcs, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (!equals(trcs[i], trcs[j]))
                continue;

            if (j < i)
            {
                trcs[i]->freq = trcs[j]->freq;
                break;
            }
            trcs[i]->freq++;
        }
    }
}

trace_t *getMaxFreqTrace(trace_t **trcs, int size)
{
    trace_t *maxTr = NULL;
    for (int i = 0; i < size; i++)
    {
        if (maxTr == NULL || trcs[i]->freq > maxTr->freq)
        {
            maxTr = trcs[i];
        }
    }
    return maxTr;
}

/* WHERE IT ALL HAPPENS ------------------------------------------------------*/
int main(int argc, char *argv[])
{
#pragma region stage0
    int size = DEFAULT_TOTAL_CAPACITY;
    trace_t **trcs = malloc(sizeof(trace_t *) * size);
    trcs = initTrcsFromFile(trcs, &size, "test0.txt");
    calcTrcsFreq(trcs, size);
    trace_t *maxTr = getMaxFreqTrace(trcs, size);
    int nDistTrcs = countDistinctTraces(trcs, size);

    // for (int i = 0; i < size; i++)
    // {
    //     printf("------------\n");
    //     printTrace(trcs[i]);
    //     printf("Freq = %d", trcs[i]->freq);
    // }

    int nEvts = countEvts(trcs, size);
    int nDistEvts;
    action_t *distEvts = findDistinctEvents(trcs, size, &nDistEvts);
    int *evtFreqs = calcEvtFreq(trcs, size, distEvts, nDistEvts);

    printf("==STAGE 0============================\n");
    printf("Number of distinct events: %d\n", nDistEvts);
    printf("Number of distinct traces: %d\n", nDistTrcs);
    printf("Total number of events: %d\n", nEvts);
    printf("Total number of traces: %d\n", size);
    printf("Most frequent trace frequency: %d\n", maxTr->freq);
    printTrace(maxTr);
    for (int i = 0; i < nDistEvts; i++)
    {
        printf("%c = %d\n", distEvts[i], evtFreqs[i]);
    }
#pragma endregion

#pragma region stage1

#pragma endregion
}