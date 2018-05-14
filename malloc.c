
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct block {
	size_t		      size;
	struct block   *next;
	struct block   *prev;
} block_t;

#define BLOCK_MEM(ptr) ((void *)((unsigned long)ptr + sizeof(block_t)))
#define BLOCK_HEADER(ptr) ((void *)((unsigned long)ptr - sizeof(block_t)))

static block_t *head = NULL;

void add(block_t* block) {
  if (!head) {
    head = block;
  } else {
    block_t* current = head;
    while (current->next && current < block) {
      current = current->next;
    }
    if (current->next) {
      current->prev->next = block;
      block->prev = current->prev;
      block->next = current;
      current->prev = block;
    } else {
      current->next = block;
      block->prev = current;
    }

  }
}

void remove_(block_t* block) {

  if (block->prev == NULL) {
    if (block->next) {
			head = block->next;
		} else {
			head = NULL;
		}
	} else {
		block->prev->next = block->next;
	}

  if (block->next) {
		block->next->prev = block->prev;
  }

}

void *split(block_t* block, size_t size) {

  block_t *newblock = (block_t *) (block + size + sizeof(block_t));
  newblock->size = block->size - (size + sizeof(block_t));
  block->size = size + sizeof(block_t);

  return newblock;
}

void *malloc(size_t size) {
  if (size == 0) {
    return NULL;
  }
  block_t* current = head;
  while (current) {
    if (current->size >= size + sizeof(block_t)) {
      remove_(current);
      if (current->size == size + sizeof(block_t)) {
        return BLOCK_MEM(current);
      }
      //Splitta
      block_t* newblock = split(current, size);
      add(newblock);
      return BLOCK_MEM(current);
    } else {
      current = current->next;
    }
  }
//Didn't find any free blocks
block_t* newblock = sbrk(size + sizeof(block_t));
newblock->size = size;
return BLOCK_MEM(newblock);
}

void free(void *ptr) {
  block_t* block = BLOCK_HEADER(ptr);
  add(block);
  //Free memory if end of program
  if (block + block->size + sizeof(block_t) == sbrk(0)) {
    sbrk(-(block->size + sizeof(block_t)));
  }
  //Merge if previous block is free
  if (block + block->size + sizeof(block_t) == block->next) {
    block->size = block->size + block->next->size + sizeof(block_t);
    block->next = block->next->next;
  }
  //Merge if next block is free
  if (block->prev && block - (block->prev->size + sizeof(block_t)) == block->prev) {
    block->prev->size = block->prev->size + block->size + sizeof(block_t);
    block->prev->next = block->next;
  }
}

void *calloc(size_t nmemb, size_t size)
{
  size_t total = nmemb * size;
  char *alloc = malloc(total);

  memset(alloc, 0, total);
  return alloc;
}

void *realloc(void *ptr, size_t size) {
  if (!ptr) {
    return malloc(size);
  }
  if (size == 0) {
    free(ptr);
  }
  block_t* block = BLOCK_HEADER(ptr);
  if (size <= block->size) {
    block->size = size;
    block_t* newblock = split(block, size);
    free(newblock);
    return ptr;
  } else {
    block_t* newblock = malloc(size);
    memcpy(newblock + sizeof(block_t), ptr, block->size);
    free(ptr);
    return BLOCK_MEM(newblock);
  }
  return NULL;


}
