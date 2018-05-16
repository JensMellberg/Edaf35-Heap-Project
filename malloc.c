
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
	printf("adding %i to list\n", block);
  if (!head) {
    head = block;
  } else {
    block_t* current = head;
    while (current->next && current < block) {
      current = current->next;
    }
    if (current >= block) {
			if (current != head) {
      current->prev->next = block;
			block->prev = current->prev;
			printf("%i next is now %i\n", current->prev, block );
		  } else {
				head = block;
			}
      block->next = current;
      current->prev = block;
    } else {
      current->next = block;
      block->prev = current;
    }

  }
	print_free();
}

void remove_(block_t* block) {
	printf("removing block %i, it has prev %i\n", block, block->prev);
  if (block->prev == NULL) {
    if (block->next) {
			head = block->next;
			head->prev = NULL;
		} else {
			head = NULL;
		}
	} else {
		block->prev->next = block->next;
	}

  if (block->next) {
		block->next->prev = block->prev;
  }

	print_free();

}

void print_free() {
	block_t* current = head;
	printf("Current list: ");
	while (current) {
		printf("%i (%i) -> ",current, current->size);
		current = current->next;
	}
	printf("\n" );
}

void *split(block_t* block, size_t size) {
	if (size + sizeof(block_t) > block->size)
		return NULL;
  block_t *newblock = (block_t *) (((unsigned long)block) + size + sizeof(block_t));
  newblock->size = block->size - (size + sizeof(block_t));
  block->size = size;
	block->prev = NULL;
	block->next = NULL;
	printf("split created new block at: %i, with size = %i\n",newblock, newblock->size);
	printf("old block %i,  now has size %i", block, block->size);

  return newblock;
}


void *malloc(size_t size) {
	//printf("malloc call of size %i\n", size);
  if (size == 0) {
    return NULL;
  }
  block_t* current = head;
  while (current) {
    if (current->size >= size) {
			//printf("found free block %i, with size %i\n",current, current->size);
      remove_(current);
      if (current->size == size) {
        return BLOCK_MEM(current);
      }
      //Splitta
      block_t* newblock = split(current, size);
			newblock->prev = NULL;
			newblock->next = NULL;
			if (newblock) {
        add(newblock);
		  }
      return BLOCK_MEM(current);
    } else {
      current = current->next;
    }
  }
//Didn't find any free blocks
block_t* newblock = sbrk(size + sizeof(block_t));
newblock->size = size;
newblock->prev = NULL;
newblock->next = NULL;
//printf("Malloc created new block at: %i, with size = %i\n",newblock, size);
return BLOCK_MEM(newblock);
}

void free(void *ptr) {
	//printf("free call on memory %i, (block %i)\n", ptr, BLOCK_HEADER(ptr) );
	if (!ptr) {
		return NULL;
	}
  block_t* block = BLOCK_HEADER(ptr);
  add(block);
	printf("size of freed = %i\n",block->size);
  //Free memory if end of program
  if (block + block->size + sizeof(block_t) == sbrk(0)) {
    sbrk(-(block->size + sizeof(block_t)));
  }
	//Merge if next block is free
  if (((unsigned long)block) + block->size + sizeof(block_t) == block->next) {
    block->size = block->size + block->next->size + sizeof(block_t);
    block->next = block->next->next;
		if (block->next) {
			block->next->prev = block;
  	}
		printf("merging with next block\n");
  }
  //Merge if previous block is free
  if (block->prev && ((unsigned long)block) - (block->prev->size + sizeof(block_t)) == block->prev) {
    block->prev->size = block->prev->size + block->size + sizeof(block_t);
    block->prev->next = block->next;
		block->next->prev = block->prev;
		printf("merging with previous block\n");
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

    block_t* newblock = split(block, size);
		block->size = size;
		if (newblock) {
			add(newblock);
		}
    return ptr;
  } else {
    void* newblock = malloc(size);
    memcpy(newblock, ptr, block->size);
    free(ptr);
    return newblock;
  }
  return NULL;


}
