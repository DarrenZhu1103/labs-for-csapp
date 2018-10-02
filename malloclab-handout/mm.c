/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "XXXXXXX",
    /* First member's full name */
    "DarrenZhu",
    /* First member's email address */
    "@DarrenZhu1103",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8


/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_OTHER(p) (GET(p) & 0x2)
#define GET_LAST(p) (GET(p) & 0x3)

/* Given the block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define PVRP(bp) ((char *) (bp))
#define NTRP(bp) ((char *)(bp) + WSIZE)

/* Given the block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))
#define NEXT_FREE(bp) (GET(NTRP(bp)))
#define PREV_FREE(bp) (GET(PVRP(bp)))

/* Global Variables */
static char *heap_listp = 0;
static char *free_listp = 0;

/* Private helper functions */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *place(void *bp, size_t asize, int r);
static void *find_fit(size_t asize);
//static void checkheap(int verbose);
//static void printblock(void *bp);
//static void checkblock(void *bp);
static void connect(void *pre, void *next);
static void addToList(void *bp);
static void deleteFromList(void *bp);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;

    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 3));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 3));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 3));
    heap_listp += 2 * WSIZE;

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t newsize = ALIGN(size + sizeof(size_t));
    size_t extendsize;
    char *bp;

    if (heap_listp == 0)
        mm_init();

    if (size == 0)
        return NULL;
    
    if ((bp = find_fit(newsize)) != NULL) {
        return place(bp, newsize, 0);
        //return bp;
    }

    extendsize = MAX(newsize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    deleteFromList(bp);
    return place(bp, newsize, 0);
    //return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if (ptr == 0)
        return;

    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, GET_OTHER(HDRP(ptr))));
    PUT(FTRP(ptr), GET(HDRP(ptr)));

    coalesce(ptr);
}

/*
void mm_checkheap(int verbose) {
    checkheap(verbose);
}
*/

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 * front and next
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *newptr;
    void *nextptr = NEXT_BLKP(ptr);
    void *pptr = PREV_BLKP(ptr);
    size_t oldSize, newSize, copySize;
    
    //newptr = mm_malloc(size);
    if (ptr == NULL)
        return mm_malloc(size);

    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    newSize = ALIGN(size + sizeof(size_t));
    oldSize = GET_SIZE(HDRP(ptr));
    copySize = oldSize - WSIZE;
    
    
    if (newSize <= oldSize)
        return ptr;


    if (!GET_ALLOC(HDRP(nextptr))) {
        deleteFromList(nextptr);
        oldSize += GET_SIZE(HDRP(nextptr));
        PUT(HDRP(ptr), PACK(oldSize, GET_LAST(HDRP(ptr))));
    }
        
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(GET_SIZE(HDRP(NEXT_BLKP(ptr))), 3));
    
    
    if (!GET_OTHER(HDRP(ptr))) {
        pptr = PREV_BLKP(ptr);
        deleteFromList(pptr);
        memcpy(pptr, ptr, copySize);
        oldSize += GET_SIZE(HDRP(pptr));
        PUT(HDRP(pptr), PACK(oldSize, 3));
        ptr = pptr;
    }
    

    if (newSize <= oldSize) {
        return place(ptr, newSize, 1);
        //return ptr;
    } 
    else if (!GET_SIZE(HDRP(NEXT_BLKP(ptr)))) {
        if (extend_heap(newSize - oldSize) == NULL)
            return NULL;
        deleteFromList(NEXT_BLKP(ptr));
        oldSize += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(ptr), PACK(oldSize, GET_LAST(HDRP(ptr))));
        PUT(HDRP(NEXT_BLKP(ptr)), 3);
        return ptr;
    }

    newptr = mm_malloc(size);
    memcpy(newptr, ptr, copySize);
    mm_free(ptr);
    return newptr;
}

/* extend_heap - Extend heap with free block and return its block pointer */
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    /* make free block */
    PUT(HDRP(bp), PACK(size, GET_OTHER(HDRP(bp))));
    PUT(FTRP(bp), GET(HDRP(bp)));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

/* coalesce - Boundary tag coalescing. Return ptr to coalesced. */
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_OTHER(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        PUT(HDRP(NEXT_BLKP(bp)), PACK(GET_SIZE(HDRP(NEXT_BLKP(bp))), 1));
        addToList(bp);
        return bp;
    }

    else if (prev_alloc && !next_alloc) {
        deleteFromList(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 2));
        PUT(FTRP(bp), PACK(size, 2));
        addToList(bp);
    }

    else if (!prev_alloc && next_alloc) {
        deleteFromList(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 2));
        PUT(FTRP(bp), PACK(size, 2));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(GET_SIZE(HDRP(NEXT_BLKP(bp))), 1));
        bp = PREV_BLKP(bp);
        addToList(bp);
    }

    else {
        deleteFromList(PREV_BLKP(bp));
        deleteFromList(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 2));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 2));
        bp = PREV_BLKP(bp);
        addToList(bp);
    }

    return bp;
}


/*
 * place - Place block of asize bytes at start of free block bp and split
 *         if remainder would be at least minimum block size
 */
static void *place(void *bp, size_t asize, int r) {
    size_t csize = GET_SIZE(HDRP(bp));
    char *p;

    if ((csize - asize) < 2 * DSIZE) {
        PUT(HDRP(bp), PACK(csize, (GET_OTHER(HDRP(bp)) | 1)));
        //PUT(FTRP(bp), PACK(csize, 1));

        PUT(HDRP(NEXT_BLKP(bp)), PACK(GET(HDRP(NEXT_BLKP(bp))), 3));
        
    } else if (asize >= 96 && !r) {
        PUT(HDRP(bp), PACK(csize - asize, GET_OTHER(HDRP(bp))));
        PUT(FTRP(bp), PACK(csize - asize, GET_OTHER(HDRP(bp))));
        p = NEXT_BLKP(bp);
        PUT(HDRP(p), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(p)), PACK(GET_SIZE(HDRP(NEXT_BLKP(p))), 3));
        addToList(bp);
        return p;
    } else {
        PUT(HDRP(bp), PACK(asize, (GET_OTHER(HDRP(bp)) | 1)));
        //PUT(FTRP(bp), PACK(asize, 1));
        p = NEXT_BLKP(bp);
        PUT(HDRP(p), PACK(csize - asize, 2));
        PUT(FTRP(p), PACK(csize - asize, 2));
        addToList(p);
    }
    return bp;
}

/*
static void *find_fit(size_t asize) {
    void *bp = free_listp;
    
    if (bp == NULL)
        return NULL;

    do {
        if (asize <= GET_SIZE(HDRP(bp))) {
            deleteFromList(bp);
            return bp;
        }
        bp = (void *) NEXT_FREE(bp);
    } while (bp != free_listp);
    return NULL;
}
*/

static void *find_fit(size_t asize) {
    void *bp = free_listp, *best = NULL;
    size_t bsize = mem_heapsize();

    if (bp == NULL)
        return NULL;
    
    do {
        if (asize <= GET_SIZE(HDRP(bp)) && GET_SIZE(HDRP(bp)) < bsize) {
            best = bp;
            bsize = GET_SIZE(HDRP(bp));
        }
        bp = (void *) NEXT_FREE(bp);
    } while (bp != free_listp);
    if (best != NULL)
        deleteFromList(best);
    return best;
}
/*
static void checkheap(int verbose) {
    char *bp = heap_listp;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE || !GET_ALLOC(HDRP(heap_listp))))
        printf("Bad prologue header\n");

    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp) > 0); bp = NEXT_BLKP(bp)) {
        if (verbose)
            printblock(bp);
        checkblock(bp);
    }

    if (verbose)
        printblock(bp);

    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
}

static void checkblock(void *bp) {
    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET_ALLOC(HDRP(bp)) && GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
}

static void printblock(void *bp) {
    size_t hsize, halloc;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    //fsize = GET_SIZE(FTRP(bp));
    //falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p, header: [%d:%c]\n", bp,
           hsize, (halloc ? 'a' : 'f'));
}
*/


static void connect(void *pre, void *next) {
    PUT(PVRP(next), (unsigned int) pre);
    PUT(NTRP(pre), (unsigned int) next);
    //*(unsigned int *) (pre + 4) = (unsigned int) next;
    //*(unsigned int *) next = (unsigned int) pre;
}


static void addToList(void *bp) {
    if (free_listp == NULL)
        connect(bp, bp);
    else {
        connect((void *) PREV_FREE(free_listp), bp);
        connect(bp, free_listp);
    }
    free_listp = bp;
}

static void deleteFromList(void *bp) {
    if (bp == free_listp)
        free_listp = (char *) NEXT_FREE(bp);

    if (bp == (void *) NEXT_FREE(bp))
        free_listp = NULL;
    else
        connect((void *) PREV_FREE(bp), (void *) NEXT_FREE(bp));
}

