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

// prints a given trace
void printTrace(trace_t *tr)
{
    event_t *evt = tr->head;
    while (1)
    {
        if (isalpha(evt->actn))
            printf("%c", evt->actn);
        else
            printf("%d", evt->actn);
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

trace_t *loadTrace(char *str)
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

// loads all the tracees from file
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
        trcs[i] = loadTrace(line);
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

// find all the distinct events in the given traces
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
    for (int step = 0; step < nDist - 1; step++)
    {
        int min_idx = step;
        for (int i = step + 1; i < nDist; i++)
        {
            // Select the minimum element in each loop.
            // if (array[i] < array[min_idx])
            //     min_idx = i;
            if (distEvts[i] < distEvts[min_idx])
                min_idx = i;
        }
        // put min at the correct position
        action_t temp = distEvts[min_idx];
        distEvts[min_idx] = distEvts[step];
        distEvts[step] = temp;
    }
    *nDistEvts = nDist;
    return distEvts;
}

// counts the total number of events in the given tracecs
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

// calculates the frequencies of all the events in the given traces
// returns an array of frequencies.
// The index of frequency in the returned array corresponds to the event's
// index in lexicographical order
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

// checks whether two traces are equal
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

// counts all the distinct traces in the given traces
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

// calculates the frequencies of the given traces
//  returns an array of frequencies.
//  The index of frequency in the returned array corresponds to the trace's
//  index in lexicographical order
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

// returns the trace with the maximum frequency
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

// Initializes and returns the directly follows matrix
DF_t initDFMatrix(action_t *distEvts, int nDistEvts, trace_t **trcs, int trSize)
{
    // printf("initdf ndist = %d", nDistEvts);

    action_t max = distEvts[0],
             min = distEvts[0];

    for (int i = 0; i < nDistEvts; i++)
    {
        if (distEvts[i] > max)
            max = distEvts[i];
        if (distEvts[i] < min)
            min = distEvts[i];
    }
    // if (nDistEvts == 4)
    // {
    //     printf("\n MAX = %d , MIN = %d", max, min);
    //     return NULL;
    // }
    DF_t matrix = malloc(sizeof(action_t *) * (max + 1 - min));

    for (int r = 0; r < nDistEvts; r++)
    {
        matrix[distEvts[r] - distEvts[0]] = (int *)malloc((max + 1 - min) * sizeof(action_t));
        // printf("%d row init\n", r);
    }

    for (int r = 0; r < nDistEvts; r++)
    {
        for (int c = 0; c < nDistEvts; c++)
        {
            // printf("%d,%d = %d", r, c, 0);
            matrix[distEvts[r] - distEvts[0]][distEvts[c] - distEvts[0]] = 0;
            // printf("%d,%d = %d\n", r, c, 0);
        }
    }

    int count = 0;
    for (int i = 0; i < trSize; i++)
    {
        event_t *cur = trcs[i]->head;
        while (1)
        {
            if (cur == trcs[i]->foot)
            {
                break;
            }

            for (int evt1Idx = 0; evt1Idx < nDistEvts; evt1Idx++)
            {
                for (int evt2Idx = 0; evt2Idx < nDistEvts; evt2Idx++)
                {
                    action_t evt1 = distEvts[evt1Idx];
                    action_t evt2 = distEvts[evt2Idx];
                    if (cur->actn == evt1 && cur->next->actn == evt2)
                    {
                        int row = evt1 - distEvts[0];
                        int col = evt2 - distEvts[0];
                        matrix[row][col]++;
                    }
                }
            }
            cur = cur->next;
        }
    }
    return matrix;
}

// support function
// returns the support for event x and y, internally uses the Directly Follows matrix to retrieve the supports
// Note that sup function takes events as inputs
// sup(x, y) is NOT the same as matrix[x][y] as indices of the matrix start from 0 and go up until the number of elements
int sup(action_t x, action_t y, action_t *distEvts, DF_t matrix)
{
    // printf("at 0 %d, ", distEvts[0]);
    // return 0;
    int row = x - distEvts[0];
    int col = y - distEvts[0];
    // printf("%c, %c,  %d, %d\n", x, y, row, col);
    return (int)(matrix[row][col]);
}

// pd function
int pd(action_t x, action_t y, action_t *distEvts, DF_t matrix)
{
    int supxy = sup(x, y, distEvts, matrix);

    int supyx = sup(y, x, distEvts, matrix);
    // printf("just before sup(%c,%c)  = %d", y, x, supyx);
    int max = supxy > supyx ? supxy : supyx;
    // printf("max = %d", max);
    // printf("calculating result");
    int result = max > 0 ? (100 * abs(supxy - supyx)) / max : 0;
    // printf("%d result", result);
    return result;
}

// weight function
int w(action_t x, action_t y, action_t *distEvts, DF_t matrix)
{
    int pdxy = pd(x, y, distEvts, matrix);
    int supxy = sup(x, y, distEvts, matrix);
    int supyx = sup(y, x, distEvts, matrix);
    int max = supxy > supyx ? supxy : supyx;
    return abs(50 - pdxy) * max;
    // w(x, y) = abs(50 − pd(x, y)) × max(sup(x, y), sup(y, x));
}

// prints the Directly Follows matrix
void printDFMatrix(DF_t seqMatrix, action_t *distEvts, int nDistEvts)
{
    printf("     ");
    for (int i = 0; i < nDistEvts; i++)
    {
        if (isalpha(distEvts[i]))
            printf("%5c", distEvts[i]);
        else
            printf("%5d", distEvts[i]);
    }
    printf("\n");
    for (int row = 0; row < nDistEvts; row++)
    {
        if (isalpha(distEvts[row]))
            printf("%5c", distEvts[row]);
        else
            printf("%5d", distEvts[row]);
        for (int col = 0; col < nDistEvts; col++)
        {
            printf("%5d", seqMatrix[distEvts[row] - distEvts[0]][distEvts[col] - distEvts[0]]);
        }
        printf("\n");
    }
}

void replace(action_t x, action_t code, trace_t **trcs, int trSize)
{
    for (int i = 0; i < trSize; i++)
    {
        event_t *cur = trcs[i]->head;
        while (1)
        {
            if (cur->actn == x)
                cur->actn = code;
            if (cur == trcs[i]->foot)
                break;
            cur = cur->next;
        }
    }
}

int abstractPair(action_t z, trace_t **trcs, int trSize)
{
    int nEvts = 0;
    for (int i = 0; i < trSize; i++)
    {
        event_t *e1 = trcs[i]->head;
        event_t *e2 = trcs[i]->head->next;

        while (1)
        {
            if (e1->actn != z || e2->actn != z)
            {
                if (e2 == trcs[i]->foot)
                    break;
                e1 = e1->next;
                e2 = e2->next;
                // if (i == 0)
                //     printf("e1 = %d, e2 = %d", e1->actn, e2->actn);
                // continue;
            }
            else
            {
                // if (x == 'e')
                // {
                //     printf("char %d -> %d\t", e1->actn, e2->actn);
                // }
                // e1->actn = z;
                if (e2 != trcs[i]->foot)
                {
                    e1->next = e2->next;
                    free(e2);
                    e2 = e1->next;
                    // if (x == 'e')
                    // {
                    //     printf("char %d -> %d -> %d\n", e1->actn, e2->actn, e2->next->actn);
                    // }
                    nEvts++;
                }
                else
                {
                    e1->next = NULL;
                    trcs[i]->foot = e1;
                    nEvts++;
                    free(e2);
                    // if (x == 'e')
                    // {
                    //     printf("char %d -> %d -> %d\n", e1->actn, e2->actn, e2->next->actn);
                    // }
                    break;
                }
            }
        }
    }
    // number of events removed
    return nEvts;
}

void getSeq(action_t *outX, action_t *outY, action_t *distEvts, int nDistEvts, DF_t seqMatrix)
{
    action_t x = 0, y = 0;
    for (int row = 0; row < nDistEvts; row++)
    {
        for (int col = 0; col < nDistEvts; col++)
        {
            // printf("row = %d, col = %d\n", row, col);
            if (row == col)
                continue;

            action_t rowEvt = distEvts[row];
            action_t colEvt = distEvts[col];
            if (x == 0 || y == 0)
            {
                x = rowEvt;
                y = colEvt;
                continue;
            }

            if (pd(rowEvt, colEvt, distEvts, seqMatrix) <= 70)
                continue;

            int wxy = w(x, y, distEvts, seqMatrix);
            int wRowCol = w(rowEvt, colEvt, distEvts, seqMatrix);
            if (wRowCol > wxy)
            {
                x = rowEvt;
                y = colEvt;
            }
        }
    }
    *outX = x;
    *outY = y;
}

/* Stage 2 ----------------------------------------------------------------------------------s*/
void get2(action_t *outX, action_t *outY, int *outType, action_t *distEvts, int nDistEvts, DF_t seqMatrix)
{
    action_t x = 0, y = 0;
    int maxWeight = 0;
    for (int row = 0; row < nDistEvts; row++)
    {
        for (int col = 0; col < nDistEvts; col++)
        {
            // printf("row = %d, col = %d\n", row, col);

            action_t rowEvt = distEvts[row];
            action_t colEvt = distEvts[col];
            if (x == 0 || y == 0)
            {
                x = rowEvt;
                y = colEvt;
                continue;
            }
            if (row == col)
                continue;
            int supxy = sup(rowEvt, colEvt, distEvts, seqMatrix);
            int supyx = sup(colEvt, rowEvt, distEvts, seqMatrix);
            int max = supxy > supyx ? supxy : supyx;

            // check choice pattern
            if (max <= nDistEvts / 100)
            {
                int weight = nDistEvts * 100;
                if (weight > maxWeight)
                {
                    maxWeight = weight;
                    x = rowEvt;
                    y = colEvt;
                    *outType = 0;
                }
            }
            // check candidate concurrency pattern
            else if (supxy > 0 && supyx > 0 && pd(rowEvt, colEvt, distEvts, seqMatrix) < 30)
            {

                int weight = 100 * w(rowEvt, colEvt, distEvts, seqMatrix);
                if (weight > maxWeight)
                {
                    maxWeight = weight;
                    x = rowEvt;
                    y = colEvt;
                    *outType = 1;
                }
            }
            else if (supxy > supyx && pd(rowEvt, colEvt, distEvts, seqMatrix) > 70)
            {
                int weight = w(rowEvt, colEvt, distEvts, seqMatrix);
                if (isalpha(rowEvt) && isalpha(colEvt))
                {
                    weight *= 100;
                }
                if (weight > maxWeight)
                {
                    maxWeight = weight;
                    x = rowEvt;
                    y = colEvt;
                    *outType = 2;
                }
            }
        }
    }
    *outX = x;
    *outY = y;
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
    printf("==STAGE 1============================\n");
    int nDistEvtsInit = nDistEvts;
    // DF_t seqMatrix;
    int code = 256;
    for (int i = 1; i <= nDistEvtsInit / 2; i++)
    {
        int nDistEvtsi;

        action_t *distEvtsi = findDistinctEvents(trcs, size, &nDistEvtsi);

        int *evtFreqsi = calcEvtFreq(trcs, size, distEvtsi, nDistEvtsi);

        // printf(" i = %d ndist = %d", i, nDistEvtsi);

        DF_t seqMatrix = initDFMatrix(distEvtsi, nDistEvtsi, trcs, size);
        // if (i == 3)
        //     break;

        action_t x, y;

        getSeq(&x, &y, distEvtsi, nDistEvtsi, seqMatrix);
        if (!(isalpha(x) && isalpha(y)))
            break;

        if (i != 1)
            printf("=====================================\n");
        printDFMatrix(seqMatrix, distEvtsi, nDistEvtsi);

        replace(x, code, trcs, size);
        replace(y, code, trcs, size);
        int n = abstractPair(code, trcs, size);
        for (int i = 0; i < size; i++)
            printTrace(trcs[i]);
        free(distEvtsi);
        free(evtFreqsi);

        distEvtsi = findDistinctEvents(trcs, size, &nDistEvtsi);

        evtFreqsi = calcEvtFreq(trcs, size, distEvtsi, nDistEvtsi);
        printf("-------------------------------------\n");
        printf("%d = SEQ(%c,%c)\n", code, x, y);
        printf("Number of events removed: %d\n", n);
        for (int i = 0; i < nDistEvtsi; i++)
        {
            if (isalpha(distEvtsi[i]))
                printf("%c = %d\n", distEvtsi[i], evtFreqsi[i]);
            else
                printf("%d = %d\n", distEvtsi[i], evtFreqsi[i]);
        }
        code++;
    }
#pragma endregion
    // for (int i = 0; i < size; i++)
    //     printTrace(trcs[i]);
    // return 0;
#pragma region stage2
    printf("==STAGE 2============================\n");
    for (int i = 1; i <= size / 2; i++)
    {
        int nDistEvtsi;

        action_t *distEvtsi = findDistinctEvents(trcs, size, &nDistEvtsi);

        int *evtFreqsi = calcEvtFreq(trcs, size, distEvtsi, nDistEvtsi);

        // printf(" i = %d ndist = %d", i, nDistEvtsi);

        DF_t seqMatrix = initDFMatrix(distEvtsi, nDistEvtsi, trcs, size);
        action_t x, y;
        int pType;
        get2(&x, &y, &pType, distEvtsi, nDistEvtsi, seqMatrix);

        if (i != 1)
            printf("=====================================\n");
        printDFMatrix(seqMatrix, distEvtsi, nDistEvtsi);

        printf("-------------------------------------\n");
        char *typeStr;

        switch (pType)
        {
        case 0:
            typeStr = "CHC";
            break;
        case 1:
            typeStr = "CON";
            break;
        case 2:
            typeStr = "SEQ";
        }
        if (isalpha(x))
        {
            printf("%d = %s(%c,%c)\n", code, typeStr, x, y);
        }
        else
        {
            printf("%d = %s(%d,%d)\n", code, typeStr, x, y);
        }
        // printf("Number of events removed: %d\n", n);

        replace(x, code, trcs, size);
        replace(y, code, trcs, size);

        int n = abstractPair(code, trcs, size);

        // for (int i = 0; i < size; i++)
        //     printTrace(trcs[i]);
        // break;
        free(distEvtsi);
        free(evtFreqsi);

        distEvtsi = findDistinctEvents(trcs, size, &nDistEvtsi);
        evtFreqsi = calcEvtFreq(trcs, size, distEvtsi, nDistEvtsi);

        printf("Number of events removed: %d\n", n);
        for (int i = 0; i < nDistEvtsi; i++)
        {
            if (isalpha(distEvtsi[i]))
                printf("%c = %d\n", distEvtsi[i], evtFreqsi[i]);
            else
                printf("%d = %d\n", distEvtsi[i], evtFreqsi[i]);
        }
        // if (i == 3)
        //     break;
        code++;
    }
#pragma endregion
}