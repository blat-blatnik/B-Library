/*
  bmem.h - v0.1 - public domain memory management utilities

  by Blat Blatnik

  NO WARRANTY IMPLIED - USE AT YOUR OWN RISK

  For licence information see end of file.

  Do this:
	#define B_MEM_IMPLEMENTATION
  before you include this file in *ONE* C or C++ file to create the implementation.

  //i.e. it should look something like this:
  #include ...
  #include ...
  #include B_MEM_IMPLEMENTATION
  #include "bmem.h"

  -----------------------------
  ----- TEMPORARY STORAGE -----
  -----------------------------

  This library provides two sets of memory utilities. The first set of utilities
  provides a thread-local small "temporary storage" memory which can linearly allocated
  with the talloc function. I got this idea from Jonathan Blow's Jai programing
  language which provides a similar utility. The basic idea is to have a small thread-local
  memory arena which is used for allocating memory which doesn't need to live for very
  long. 
  
  For example in a game you could use it to allocate memory that is only used within
  the frame of allocation, and then the allocator would be completely reset at the end of
  the frame, freeing up the memory to be used again for the next frame. This allocation
  scheme should in theory be very fast, if the memory arena is sufficiently small, most of
  it should fit in cache.

  This temporary memory is not meant to be allocated and then freed like you do with
  malloc/free. Rather you are supposed to talloc memory to your hearts content, and then call
  tempReset(0) ONCE IN A WHILE - for example every second - to clear out ALL of the temporary
  memory at once. If you know how long the talloce'd memory will last and you don't want to
  waste space in the temporary storage, you can use the following pattern:

  int mark = tempMark();         // save current state of the temporary storage
  void *memory = talloc(...);    // allocate more temp storage memory
  // use 'memory'
  tempReset(mark);               // reset the temporary storage back to original state - effectively freeing 'memory'

  Since the temporary storage memory is thread-local, it is inherently THREAD-SAFE since
  each thread gets its own memory arena. By default the temporary storage has a capacity of
  64KiB, however you can

  #define B_TEMP_MEM_SIZE [size in bytes]

  If you want to increase this limit. If you try to talloc memory over the capacity of the
  temp storage, for example if you try to talloc 3 x 32KiB, then the first two tallocs will
  work as normal, but the last one will overflow. The overflow will be detected inside of
  talloc, and it will fall back to allocating memory from the heap with malloc. Since you
  aren't generally supposed to call 'free' on memory returned by talloc, this overflowed memory
  will leak, but your application will not crash and you will get the amount of memory you requested.
  
  The overflow will be logged out to the console, so you WILL be notified exactly when
  it happens, and exactly how many bytes got leaked. You can then adjust the temporary storage
  size accordingly so that the leaks stop happening. You can also call 'getTempMemStats' to
  get some useful statistics from your usage patterns, such as the total number of talloc calls,
  or the highest number of temp bytes ever active in your application.

  --------------------------
  ----- HEAP DEBUGGING -----
  --------------------------

  The second set of memory utilities aim to help track down memory leaks and buffer overruns.
  the functions debugAlloc, debugRealloc, and debugFree act like malloc, realloc and free do
  but they also allocate additional memory to store some meta-info about each allocation, such
  as the file and line in which each allocation happened.

  In debug builds (when NDEBUG is not defined) malloc, realloc, and free will be #defined like this:
  
  #define malloc  debugAlloc
  #define realloc debugRealloc
  #define free    debugFree
  
  so that this file can be just dropped into an existing codebase and everything should continue
  to work as expected. You can call debugMemDump at any time to print out the currently active
  heap allocations in your program. So you can for example call debugMemDump at the end of
  your program to check for memory leaks.

  You can also call debugGetFirstHeapBlock to get a pointer to a the first element of a doubly
  linked list holding information about all of the allocations. Check the implementation of
  debugMemDump for an example of how to use debugGetFirstHeapBlock. You can also call debugGetHeapStats
  to get some information about how your application uses heap memory - such as how many times
  you called malloc, the average number of bytes per allocation, etc.

  If you need to access the default malloc/realloc/free functions inside of a debug build
  you can either disable the leak-checking by
  
  #define B_DONT_LEAK_CHECK
  
  before including this file, or you can call heapAlloc/heapRealloc/heapFree, which are simple
  aliases to malloc/realloc/free.

  NOTE: debugAlloc/debugRealloc/debugFree are NOT thread-safe. DO NOT use them if you 
        have a multi-threaded application where each thread calls malloc/realloc/free.

  -------------------
  ----- OPTIONS -----
  -------------------

  #define any of these before including this file for them to take effect:

  #define B_TEMP_MEM_SIZE [size-in-bytes]
  - Change the capacity of the thread-local temporary storage.

  #define B_ASSERT(condition) your-assertion-function(condition)
  - Avoid #include <assert.h> by defining your own assertion function.
    Note that if you #include "bdebug.h" before this file, the ASSERT from
	bdebug will automatically be used.

  #define B_LOG(message) your-logging-function(message)
  - Instead of printing information to stdout, use a custom logging utility.
    Note that if you #include "bdebug.h" before this file, the debugLog from
    bdebug will automatically be used.

  #define B_DONT_LEAK_CHECK 
  - Don't #define malloc/realloc/free to debugAlloc/debugRealloc/debugFree.
  
  #define B_DONT_CLEAR_TEMP_MEM
  - Don't clear temporary storage memory to 0 when calling tempReset, leave the contents unchanged.
    This will improve performance of tempReset.

  #define NDEBUG
  - Equivalent to both B_DONT_LEAK_CHECK and B_DONT_CLEAR_TEMP_MEM combined.

  #define B_ALWAYS_LEAK_CHECK
  - Ignore B_DONT_LEAK_CHECK and always #define malloc/realloc/free

  #define B_ALWAYS_CLEAR_TEMP_MEM
  - Ignore B_DONT_CLEAR_TEMP_MEM and always clear temp memory to 0 when calling tempReset.

  #define B_THREAD_LOCAL ...
  - If your compiler does not support the standard _Thread_local qualifier, then you
    can define this so that something like:
	B_THREAD_LOCAL int x;
	will correctly compile on your compiler.

  #define B_MEM_PREFIX(name) [your-name-prefix] ## name
  #define B_PREFIX(name) [your-name-prefix] ## name
  - Add a prefix to all functions/variables/types declared by this file.
    Useful for avoiding name-conflicts. By default no prefix is added.
*/

#ifndef B_MEM_DEFINITION
#define B_MEM_DEFINITION

#ifndef B_MEM_PREFIX
#   ifdef B_PREFIX
#       define B_MEM_PREFIX(name) B_PREFIX(name)
#   else
#       define B_MEM_PREFIX(name) name
#   endif
#endif

/* we NEED to include stdlib.h BEFORE setting up the
   macro replacements for malloc, realloc, and free.
   stdlib will freak out otherwise.. */
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

typedef struct B_MEM_PREFIX(HeapBlockInfo) {
	size_t      size,  size0; /* allocation size in bytes */
	time_t      time,  time0; /* allocation timestamp */
	const char *file, *file0; /* file where allocation occured */
	const char *func, *func0; /* function where allocation occured */
	int         line,  line0; /* line where allocation occured */
	struct B_MEM_PREFIX(HeapBlockInfo) *prev, *next;
} B_MEM_PREFIX(HeapBlockInfo);

typedef struct B_MEM_PREFIX(HeapStats) {
	size_t totalNumAllocs;    /* number of calls to malloc since program start */
	size_t totalNumReallocs;  /* number of calls to realloc ... */
	size_t totalNumFrees;     /* number of calls to free    ... */
	size_t totalBytesAlloced; /* number of bytes alloced    ... */
	size_t totalBytesFreed;   /* number of bytes freed      ... */
	size_t currNumAllocs;     /* number of currently active heap allocations */
	size_t currBytesAlloced;  /* number of currently active heap allocated bytes */
	size_t maxNumAllocs;      /* highest number of active heap allocations ever reached during runtime */
	size_t maxBytesAlloced;   /* highest number of active heap allocated bytes ... */
	double avgAllocLifespan;  /* average time in seconds allocations last before being freed */
} B_MEM_PREFIX(HeapStats);

typedef struct B_MEM_PREFIX(TempMemStats) {
	size_t totalNumAllocs;     /* number of calls to tempAlloc since program start */
	size_t totalNumFullResets; /* number of calls to tempReset(*0*) ... */
	size_t totalBytesAlloced;  /* number of temp bytes allocated    ... */
	size_t currBytesAlloced;   /* number temp bytes currently active in this cycle */
	size_t maxBytesAlloced;    /* highest number of temp bytes active at a time during one cycle */
	size_t totalNumLeaks;      /* number of times a temp memory leak occured because temp memory was full when calling tempAlloc */
	size_t totalBytesLeaked;   /* number of bytes which were leaked when tempAlloc was called while temp memory was full */
	size_t numAllocsSinceFullReset;      /* number of calls to tempAlloc since the last time tempReset(*0*) was called */
	size_t bytesAllocedSinceFullReset;   /* number of temp bytes allocated ... */
	double avgNumAllocsPerResetCycle;    /* average number of calls to tempAlloc during a single reset cycle */
	double avgBytesAllocedPerResetCycle; /* average number of temp bytes allocated ... */
} B_MEM_PREFIX(TempMemStats);

/* these are always exactly the same as the stdlib malloc, realloc, and free
   so that we can redefine them AND still be able to access them with these.. */
static inline void *B_MEM_PREFIX(heapAlloc)(size_t size) {
	return malloc(size);
}
static inline void *B_MEM_PREFIX(heapRealloc)(void *mem, size_t size) {
	return realloc(mem, size);
}
static inline void  B_MEM_PREFIX(heapFree)(void *mem) {
	free(mem);
}

void *                         B_MEM_PREFIX(debugAlloc)(size_t size, const char *file, const char *func, int line);
void *                         B_MEM_PREFIX(debugRealloc)(void *mem, size_t size, const char *file, const char *func, int line);
void                           B_MEM_PREFIX(debugFree)(void *mem, const char *file, const char *func, int line);
void                           B_MEM_PREFIX(debugHeapDump)(void);
B_MEM_PREFIX(HeapBlockInfo) *  B_MEM_PREFIX(debugGetFirstHeapBlock)(void);
B_MEM_PREFIX(HeapStats)        B_MEM_PREFIX(debugGetHeapStats)(void);

void *                         B_MEM_PREFIX(talloc)(size_t size, size_t align);
char *                         B_MEM_PREFIX(tsprintf)(const char *format, ...);
int                            B_MEM_PREFIX(tempMark)(void);
void                           B_MEM_PREFIX(tempReset)(int mark);
B_MEM_PREFIX(TempMemStats)     B_MEM_PREFIX(getTempMemStats)(void);

#if defined(B_ALWAYS_LEAK_CHECK) || (!defined(B_DONT_LEAK_CHECK) && !defined(NDEBUG))
#	define malloc(size)       B_MEM_PREFIX(debugAlloc)(size, __FILE__, __func__, __LINE__)
#	define realloc(mem, size) B_MEM_PREFIX(debugRealloc)(mem, size, __FILE__, __func__, __LINE__)
#	define free(mem)          B_MEM_PREFIX(debugFree)(mem, __FILE__, __func__, __LINE__)
#endif

#endif /* !B_MEM_DEFINITION */

/*
 * |                |
 * v Implementation v
 * |                |
 */

#ifdef B_MEM_IMPLEMENTATION
#ifndef B_MEM_IMPLEMENTED
#define B_MEM_IMPLEMENTED

#ifndef B_TEMP_MEM_SIZE
#	define B_TEMP_MEM_SIZE 65536
#endif

#ifndef B_ASSERT
#	ifdef B_DEBUG_DEFINITION
#		define B_ASSERT(condition) ASSERT(condition)
#	else
#		include <assert.h>
#		define B_ASSERT(condition) assert(condition)
#	endif
#endif

#ifndef B_LOG
#	ifdef B_DEBUG_DEFINITION
#		define B_LOG(message) B_DEBUG_PREFIX(debugLog)(message)
#	else
#		define B_LOG(message) printf(message)
#	endif
#endif

#ifndef B_THREAD_LOCAL
#	if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#		define B_THREAD_LOCAL __declspec(thread)
#	elif defined(__GNUC__) || defined(__clang__)
#		define B_THREAD_LOCAL __thread
#	elif defined(__cplusplus)
#		define B_THREAD_LOCAL thread_local /* c++ fallback */
#	else
#		define B_THREAD_LOCAL _Thread_local /* c fallback */
#	endif
#endif

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static B_MEM_PREFIX(HeapBlockInfo) *b__firstHeapBlock;
static B_MEM_PREFIX(HeapStats) b__heapStats;
static B_THREAD_LOCAL B_MEM_PREFIX(TempMemStats) b__tempMemStats;
static B_THREAD_LOCAL char b__tempMem[B_TEMP_MEM_SIZE];

typedef char b__WaterMark[8];

static int b__checkForOverrun(void *mem) {
	char *memory = (char *)mem;
	B_MEM_PREFIX(HeapBlockInfo) *block = (B_MEM_PREFIX(HeapBlockInfo) *)(memory - sizeof(B_MEM_PREFIX(HeapBlockInfo)) - sizeof(b__WaterMark));

	b__WaterMark *header = (b__WaterMark *)(memory - sizeof(B_MEM_PREFIX(HeapBlockInfo)));
	b__WaterMark *footer = (b__WaterMark *)(memory + block->size);
	int headerGood = memcmp(header, "ORHEADER", sizeof(b__WaterMark)) == 0;
	int footerGood = memcmp(footer, "ORFOOTER", sizeof(b__WaterMark)) == 0;

	return headerGood && footerGood;
}

void *B_MEM_PREFIX(debugAlloc)(size_t size, const char *file, const char *func, int line) {
	if (size == 0)
		return NULL;

	char *mem = (char *)B_MEM_PREFIX(heapAlloc)(size + sizeof(B_MEM_PREFIX(HeapBlockInfo)) + 2 * sizeof(b__WaterMark));
	B_MEM_PREFIX(HeapBlockInfo) *block = (B_MEM_PREFIX(HeapBlockInfo) *)mem;
	block->size0 = size;
	block->file0 = file;
	block->func0 = func;
	block->line0 = line;
	block->time0 = time(NULL);
	block->size  = size;
	block->file  = file;
	block->func  = func;
	block->line  = line;
	block->time  = block->time0;

	b__WaterMark *header = (b__WaterMark *)(mem + sizeof(B_MEM_PREFIX(HeapBlockInfo)));
	b__WaterMark *footer = (b__WaterMark *)(mem + sizeof(B_MEM_PREFIX(HeapBlockInfo)) + sizeof(b__WaterMark) + size);
	memcpy(header, "ORHEADER", sizeof(b__WaterMark));
	memcpy(footer, "ORFOOTER", sizeof(b__WaterMark));

	if (!b__firstHeapBlock) { /* this is the first memory allocation */
		b__firstHeapBlock = block;
		b__firstHeapBlock->prev = b__firstHeapBlock;
	}

	block->next = b__firstHeapBlock;
	block->prev = b__firstHeapBlock->prev;
	b__firstHeapBlock->prev->next = block;
	b__firstHeapBlock->prev = block;

	/* update heap stats.. */
	b__heapStats.totalNumAllocs++;
	b__heapStats.totalBytesAlloced += size;
	b__heapStats.currNumAllocs++;
	if (b__heapStats.currNumAllocs > b__heapStats.maxNumAllocs)
		b__heapStats.maxNumAllocs = b__heapStats.currNumAllocs;
	b__heapStats.currBytesAlloced += size;
	if (b__heapStats.currBytesAlloced > b__heapStats.maxBytesAlloced)
		b__heapStats.maxBytesAlloced = b__heapStats.currBytesAlloced;

	return mem + sizeof(B_MEM_PREFIX(HeapBlockInfo)) + sizeof(b__WaterMark);
}

void *B_MEM_PREFIX(debugRealloc)(void *mem, size_t size, const char *file, const char *func, int line) {
	if (size == 0) {
		B_MEM_PREFIX(debugFree)(mem, file, func, line);
		return NULL;
	}
	if (mem == NULL)
		return B_MEM_PREFIX(debugAlloc)(size, file, func, line);

	int isOverrun = b__checkForOverrun(mem);
	B_ASSERT(!isOverrun);

	char *memory = (char *)B_MEM_PREFIX(heapRealloc)(
		(char *)mem - sizeof(b__WaterMark) - sizeof(B_MEM_PREFIX(HeapBlockInfo)),
		size + sizeof(B_MEM_PREFIX(HeapBlockInfo)) + 2 * sizeof(b__WaterMark));
	B_MEM_PREFIX(HeapBlockInfo) *block = (B_MEM_PREFIX(HeapBlockInfo) *)memory;

	/* block memory address changed so we need to update neighbors */
	block->prev->next = block;
	block->next->prev = block;
	if (block == b__firstHeapBlock)
		b__firstHeapBlock = block;

	block->size = size;
	block->file = file;
	block->func = func;
	block->line = line;
	block->time = time(NULL);

	/* update heap stats.. */
	size_t size0 = block->size0;
	b__heapStats.totalNumReallocs++;
	if (size > size0) {
		b__heapStats.totalBytesAlloced += size - size0;
		b__heapStats.currBytesAlloced += size - size0;
	} else {
		b__heapStats.totalBytesFreed += size0 - size;
		b__heapStats.currBytesAlloced -= size0 - size;
	}

	if (b__heapStats.totalBytesAlloced > b__heapStats.maxBytesAlloced)
		b__heapStats.maxBytesAlloced = b__heapStats.totalBytesAlloced;

	return memory + sizeof(B_MEM_PREFIX(HeapBlockInfo)) + sizeof(b__WaterMark);
}

void B_MEM_PREFIX(debugFree)(void *mem, const char *file, const char *func, int line) {
	(void)file;
	(void)func;
	(void)line;
	if (!mem)
		return;

	int isOverrun = b__checkForOverrun(mem);
	B_ASSERT(!isOverrun);

	B_MEM_PREFIX(HeapBlockInfo) *block = (B_MEM_PREFIX(HeapBlockInfo) *)((char *)mem - sizeof(b__WaterMark) - sizeof(B_MEM_PREFIX(HeapBlockInfo)));

	if (block->next == block->prev) { /* either 1 or 2 active allocations */
		B_MEM_PREFIX(HeapBlockInfo) *other = block->next;
		other->next = other;
		other->prev = other;
	} else {
		block->next->prev = block->prev;
		block->prev->next = block->next;
	}

	if (block == b__firstHeapBlock)
		b__firstHeapBlock = b__firstHeapBlock->next;
	if (block == b__firstHeapBlock)
		b__firstHeapBlock = NULL;

	b__heapStats.totalNumFrees++;
	b__heapStats.totalBytesFreed += block->size;
	b__heapStats.currNumAllocs--;
	b__heapStats.currBytesAlloced -= block->size;

	/* running-average: newAvg = oldAvg + (newVal - oldAvg) / (1 + newCount) */
	double lifespan = difftime(block->time, block->time0);
	b__heapStats.avgAllocLifespan += (lifespan - b__heapStats.avgAllocLifespan) / b__heapStats.totalNumFrees;

	B_MEM_PREFIX(heapFree)(block);
}

void B_MEM_PREFIX(debugHeapDump)(void) {
	B_MEM_PREFIX(HeapBlockInfo) *block = B_MEM_PREFIX(debugGetFirstHeapBlock)();
	if (!block) {
		printf("no allocated memory\n");
		return;
	}

	int index = 1;
	do {
		char timestamp[64];
		struct tm *t = localtime(&block->time);
		strftime(timestamp, sizeof(timestamp), "%T", t);

		printf("%d: %d bytes @ %s:%d (%s) %s\n",
			index, (int)block->size, block->file, block->line, block->func, timestamp);
		if (block->size != block->size0 ||
			block->file != block->file0 ||
			block->line != block->line0 ||
			block->func != block->func0 ||
			block->time != block->time0) 
		{
			t = localtime(&block->time);
			strftime(timestamp, sizeof(timestamp), "%T", t);
			printf("   .. realloced from %d bytes @ %s:%d (%s) %s\n",
				(int)block->size0, block->file0, block->line0, block->func0, timestamp);
		}

		block = block->next;
		++index;

	} while (block != b__firstHeapBlock);
}

B_MEM_PREFIX(HeapBlockInfo) *B_MEM_PREFIX(debugGetFirstHeapBlock)(void) {
	return b__firstHeapBlock;
}

B_MEM_PREFIX(HeapStats) B_MEM_PREFIX(debugGetHeapStats)(void) {
	return b__heapStats;
}

static size_t b__roundUpPow2(size_t x, size_t pow2) {
	if ((x & (pow2 - 1)) == 0)
		return x;
	return (x + pow2) & ~(pow2 - 1);
}

void *B_MEM_PREFIX(talloc)(size_t size, size_t align) {
	if (align == 0)
		align = 8;

	B_ASSERT((align & (align - 1)) == 0); /* alignment must be a power of 2! */
	size_t memStart = b__roundUpPow2(b__tempMemStats.currBytesAlloced, align);
	size_t memEnd = memStart + size;

	if (memEnd > B_TEMP_MEM_SIZE) {
		/* temp mem leak! */
		char logBuffer[128];
		sprintf(logBuffer, "leaked %d bytes of temp memory!\n", (int)size);
		B_LOG(logBuffer);
		b__tempMemStats.totalNumLeaks++;
		b__tempMemStats.totalBytesLeaked += size;
		return malloc(size);
	}

	b__tempMemStats.bytesAllocedSinceFullReset += (memEnd - memStart);
	b__tempMemStats.numAllocsSinceFullReset++;
	b__tempMemStats.totalNumAllocs++;
	b__tempMemStats.currBytesAlloced = memEnd;
	b__tempMemStats.totalBytesAlloced += (memEnd - memStart);
	if (b__tempMemStats.currBytesAlloced > b__tempMemStats.maxBytesAlloced)
		b__tempMemStats.maxBytesAlloced = b__tempMemStats.currBytesAlloced;
	return &b__tempMem[memStart];
}

char *B_MEM_PREFIX(tsprintf)(const char *format, ...) {
	va_list args;
	va_start(args, format);
	int length = 1 + vsnprintf(NULL, 0, format, args);
	va_end(args);

	char *string = (char *)B_MEM_PREFIX(talloc)((size_t)length, 1);
	va_start(args, format);
	vsnprintf(string, (size_t)length, format, args);
	va_end(args);

	return string;
}

int B_MEM_PREFIX(tempMark)(void) {
	return (int)b__tempMemStats.currBytesAlloced;
}

void B_MEM_PREFIX(tempReset)(int mark) {
	size_t newMark = (size_t)mark;
	size_t currMark = b__tempMemStats.currBytesAlloced;
	B_ASSERT(newMark <= currMark);

#if defined(B_ALWAYS_CLEAR_TEMP_MEM) || (!defined(B_DONT_CLEAR_TEMP_MEM) && !defined(NDEBUG))
	/* zero out memory in debug builds */
	memset(&b__tempMem[newMark], 0, currMark - newMark);
#endif

	if (newMark == 0) {
		/* this is a full reset */
		/* running-average: newAvg = oldAvg + (newVal - oldAvg) / (1 + newCount) */
		b__tempMemStats.totalNumFullResets++;
		b__tempMemStats.avgBytesAllocedPerResetCycle +=
			(b__tempMemStats.bytesAllocedSinceFullReset - b__tempMemStats.avgBytesAllocedPerResetCycle) / (b__tempMemStats.totalNumFullResets);
		b__tempMemStats.avgNumAllocsPerResetCycle +=
			(b__tempMemStats.numAllocsSinceFullReset - b__tempMemStats.avgNumAllocsPerResetCycle) / (b__tempMemStats.totalNumFullResets);
		b__tempMemStats.numAllocsSinceFullReset = 0;
		b__tempMemStats.bytesAllocedSinceFullReset = 0;
	}

	b__tempMemStats.currBytesAlloced = newMark;
	b__tempMemStats.numAllocsSinceFullReset = 0;
}

B_MEM_PREFIX(TempMemStats) B_MEM_PREFIX(getTempMemStats)(void) {
	return b__tempMemStats;
}

#endif /* !B_MEM_IMPLEMENTED */
#endif /* B_MEM_IMPLEMENTATION */

/*
  ------------------------------------------------------------------------------
  This software is available under 2 licenses - choose whichever you prefer.
  ------------------------------------------------------------------------------
  ALTERNATIVE A - MIT License
  Copyright (c) 2020 Blat Blatnik
  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  ------------------------------------------------------------------------------
  ALTERNATIVE B - Public Domain (www.unlicense.org)
  This is free and unencumbered software released into the public domain.
  Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
  software, either in source code form or as a compiled binary, for any purpose,
  commercial or non-commercial, and by any means.
  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  ------------------------------------------------------------------------------
*/