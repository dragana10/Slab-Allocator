#ifndef buddy_h
#define buddy_h
#include "windows.h"
#include "cache.h"

#define BLOCK_SIZE 4096



//data structures**************************************************************
//size of union is equal to the size of largest member
//in this case, size of block is 4096
typedef union block {
	unsigned char block_data[BLOCK_SIZE];
	union block* next;
}block;

typedef struct buddy {
	int num_of_entries;
	//int all_blocks;
	//moguce da mi ne treba free blocks, 
	//jer kao free koristim all, a realno zauzimam all
	//int free_blocks;
	block* first;
	union block* list_of_blocks[32];
	HANDLE mutex_buddy;
}buddy;


//pointer to buddy system******************************************************
struct buddy* ptr_to_buddy;


//auxiliary functions**********************************************************
int get_level(int n);
void print_buddy_system(buddy* ptr);
void* is_buddy_on_this_level(void* addr, buddy* ptr, int num);
block* find_prev(block* first_on_lev, block* target);

//main functions***************************************************************
void* init_buddy(void* space, int num_of_blocks);
block* get_blocks(int num_of_blocks,buddy* ptr);
void add_blocks(void* addr, int num_of_blocks, buddy* ptr);

#endif // !buddy_h
