#include"list.hpp"

const index_t POISONINDEX = ~0L;

struct list_t
{
    index_t*    next;
    index_t*    prev;
    listelem_t* data;

    state_t state;

    index_t size;
    index_t free;
};

enum ListError
{
    ListAllocError = 11,
    ListSizeError  = 102,
    ListBroken     = 192,
    ListBadCounter = 194,
    ListBadHead    = 196,
    ListBadTail    = 198,
    ListBadFree    = 200,
    ListBadIndex   = 202,
};

enum ListState
{
    LStateOk          = 0,
    LStateOkSorted    = 1,
    LStateEndWork     = 2,
    LStateBroken      = 16,
    LStateAllocErr    = 17,
    LStateBadHead     = 18,
    LStateBadTail     = 19,
    LStateBadHeadTail = 20,
};

#define LSTATE(state) (state_t) LState##state

int ListInit(list_t &self, index_t size)
{
    ++size;

    if(size < 2)
        return ListSizeError;

    self.state = LSTATE(OkSorted);

    self.size = size;
    self.free = 0; //1;

    self.next = (index_t*) calloc(size, sizeof(index_t));

    if(self.next == nullptr)
    {
        self.next = (index_t*) ListAllocError;
        self.prev = nullptr;
        self.data = nullptr;

        self.state = LSTATE(AllocErr);
    
        return ListAllocError;
    }

    self.prev = (index_t*) calloc(size, sizeof(index_t));

    if(self.prev == nullptr)
    {
        free(self.next);

        self.next = nullptr;
        self.prev = (index_t*) ListAllocError;
        self.data = nullptr;

        self.state = LSTATE(AllocErr);

        return ListAllocError;
    }

    self.data = (listelem_t*) calloc(size, sizeof(listelem_t));

    if(self.data == nullptr)
    {
        free(self.next);
        free(self.prev);

        self.next = nullptr;
        self.prev = nullptr;
        self.data = (listelem_t*) ListAllocError;

        self.state = LSTATE(AllocErr);

        return ListAllocError;
    }

    self.next[0] = 0;
    self.prev[0] = 0;

    ListInitFree_(self, 1, size - 1);

    return 0;
}

int ListFinalize(list_t &self)
{
    free(self.next);
    free(self.prev);
    free(self.data);

    self.state = LSTATE(EndWork);

    self.free = 0;
    self.size = 0;

    return 0;
}

int ListDump(const list_t &self)
{
    FILE* dumpfile = fopen("dump.dot", "w");

    fprintf(dumpfile, "graph list_t {\n");

    fprintf(dumpfile, "    rankdir=\"LR\";\n");
    fprintf(dumpfile, "    head[shape=circle, label=\"Head\"];\n");
    fprintf(dumpfile, "    tail[shape=circle, label=\"Tail\"];\n");
    fprintf(dumpfile, "    node[shape=record];\n");
    
    index_t counter = 0;
    index_t absCurrent = self.next[0];

    while(absCurrent != 0)
    {
        fprintf(dumpfile, "    l_elem" INDEX_T " [label=\"Value: " LISTELEM_T "|{<prev>PREV: " INDEX_T "|<next>NEXT: " INDEX_T "}\", xlabel=" INDEX_T "];\n",
                counter, self.data[absCurrent], self.prev[absCurrent], self.next[absCurrent], absCurrent);
        
        counter++;
        absCurrent = self.next[absCurrent];
    }

    fprintf(dumpfile, "    head");

    for(index_t i = 0; i < counter; ++i)
    {
        fprintf(dumpfile, " -- l_elem" INDEX_T ": <prev>;\n", i);
        fprintf(dumpfile, "    l_elem" INDEX_T ": <next>", i);
    }

    fprintf(dumpfile, " -- tail;");

    fprintf(dumpfile, "}");

    fclose(dumpfile);

    std::system("dot -Tpng dump.dot -o dump.png");

    return 0;
}

int ListDump_(const list_t &self, const char* name)
{
    printf("Dump for list_t \"%s\"\n\n", name);

    printf("size = " INDEX_T "\n", self.size);
    printf("free = " INDEX_T "\n\n", self.free);

    printf("state = " STATE_T "\n\n", (int) self.state);

    for(index_t i = 0; i < self.size; ++i)
    {
        printf("[" INDEX_T "] {\n", i);
        printf("    next = " INDEX_T "\n", self.next[i]);
        printf("    prev = " INDEX_T "\n", self.prev[i]);
        printf("    data = " LISTELEM_T "\n", self.data[i]);
        printf("}\n\n");
    }

    return 0;
}

#define ListDump_(name) ListDump_(name, #name)

state_t ListOk(const list_t &self)
{
    if(self.state != LSTATE(Ok) && self.state != LSTATE(OkSorted))
        return self.state;
    
    return 0;
}

int ListAddAbs(list_t &self, index_t absIndex, listelem_t data)
{
    if(ListOk(self))
        return (int) self.state;
    
    if(self.free == 0)
        if(ListExtend_(self))
        {
            return ListAllocError;
        }

    index_t absCurrent = self.free;
    index_t absNext    = self.next[absIndex];
    index_t absPrev    = absIndex;

    self.free = self.next[absCurrent];
    self.prev[self.free] = self.prev[absCurrent];

    self.next[absCurrent] = absNext;
    self.prev[absCurrent] = absPrev;
    self.data[absCurrent] = data;

    self.next[absPrev] = absCurrent;
    self.prev[absNext] = absCurrent;

    if(self.state == LSTATE(OkSorted))
        self.state = LSTATE(Ok);
    
    return 0;
}

int ListRemoveAbs(list_t &self, index_t absIndex)
{
    if(ListOk(self))
        return (int) self.state;
    
    if(ListIsFree_(self, absIndex) || absIndex == 0)
        return ListBadIndex;
    
    index_t absCurrent = absIndex;
    index_t absNext    = self.next[absCurrent];
    index_t absPrev    = self.prev[absCurrent];

    self.next[absPrev] = self.next[absCurrent];
    self.prev[absNext] = self.prev[absCurrent];

    self.next[absCurrent] = self.free;
    self.prev[absCurrent] = self.next[absCurrent];

    self.free = absCurrent;

    if(self.state == LSTATE(OkSorted))
        self.state = LSTATE(Ok);

    return 0;
}

int ListAdd(list_t &self, index_t index, listelem_t data)
{
    index_t absIndex = ListGetAbsIndex_(self, index);

    return ListAddAbs(self, absIndex, data);
}

int ListRemove(list_t &self, index_t index)
{
    index_t absIndex = (ListGetAbsIndex_(self, index));

    return ListRemoveAbs(self, absIndex);
}

int ListExtend_(list_t &self)
{
    if(ListOk(self))
        return (int) self.state;

    if(self.free != 0)
        return ListBadFree;

    index_t newSize = ListNextSize_(self.size);

    assert(newSize > self.size);

    void* buf = nullptr;

    buf = realloc(self.next, newSize * sizeof(index_t));

    if(buf == nullptr)
    {
        return ListAllocError;
    }

    self.next = (index_t*) buf;

    buf = realloc(self.prev, newSize * sizeof(index_t));

    if(buf == nullptr)
    {
        self.next = (index_t*) realloc(self.next, self.size * sizeof(index_t));
        return ListAllocError;
    }

    self.prev = (index_t*) buf;

    buf = realloc(self.data, newSize * sizeof(listelem_t));

    if(buf == nullptr)
    {
        self.next = (index_t*) realloc(self.next, self.size * sizeof(index_t));
        self.prev = (index_t*) realloc(self.prev, self.size * sizeof(index_t));
        return ListAllocError;
    }

    self.data = (listelem_t*) buf;

    ListInitFree_(self, self.size, newSize);

    self.size = newSize;

    return 0;
}

int ListInitFree_(list_t &self, index_t absStart, index_t absEnd)
{
    if(absEnd < absStart)
        return ListBadFree;

    self.next[absEnd] = self.free;
    self.prev[absEnd] = self.next[absEnd];
    self.free = absStart;
    
    while(absStart < absEnd)
    {
        self.next[absStart] = absStart + 1;
        self.prev[absStart] = self.next[absStart];

        absStart++;
    }

    return 0;
}

index_t ListNextSize_(index_t size)
{
    if(size < 256)
        return 2 * size;

    if(size < 65536)
        return size + 256;
    
    return size + size / 2;
}

int ListIsFree_(const list_t &self, index_t absIndex)
{
    if(self.prev[absIndex] == self.next[absIndex] && absIndex != self.prev[0])
        return 1;
    
    return 0;
}

index_t ListGetAbsIndex_(const list_t &self, index_t index)
{
    if(self.state == LSTATE(OkSorted))
        return index;
    
    index_t absIndex = self.next[0];
    
    for(index_t i = 1; i < index; ++i)
        absIndex = self.next[absIndex];
    
    return absIndex;
}