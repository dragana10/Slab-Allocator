#include<math.h>
#include<stdio.h>
#include<string.h>
#include"buddy.h"
#include <stdlib.h>


//auxiliary variables*****************************************************************************************
#define MASK 0xFFFFF000
#define NEXT_BLOCK 0x00001000
void* start_addr;
int num_of_all_blocks;
unsigned to_start;
HANDLE mutex;



//auxiliary functions*****************************************************************************************
int get_level(int n) {
	int base = 1;
	int deg = 0;
	while (base < n) {
		base *= 2;
		deg += 1;
	}
	return deg;
}

void print_buddy_system(buddy* ptr) {
	buddy* ptr_to_buddy = ptr;
	WaitForSingleObject(ptr_to_buddy->mutex_buddy, INFINITE);
	int n = ptr_to_buddy->num_of_entries;
	printf("\nlevels %d\n", n);
	printf("first block[%x]\n", ptr_to_buddy->first);
	//printf("all blocks %d\n", ptr_to_buddy->all_blocks);
	//printf("free blocks %d \n\n", ptr_to_buddy->free_blocks);

	for (int i = 0; i <= n; i++) {
		if (ptr_to_buddy->list_of_blocks[i] == 0) {
			printf("list_of_blocks[%d] is empty \n", i);
		}
		else {
			block* tmp = ptr_to_buddy->list_of_blocks[i];
			printf("list_of_blocks[%d]:", i);
			while (tmp) {
				printf(" %x ->", tmp);
				tmp = tmp->next;
			}
			printf("\n");
		}
	}
	ReleaseMutex(ptr_to_buddy->mutex_buddy);
}

void* is_buddy_on_this_level(void* addr, buddy* ptr, int num) {
	int lev = ceil((double)log(num) / log(2));
	//my index
	int my_indx = ((unsigned char*)addr - (unsigned char*)ptr->first) / (num * BLOCK_SIZE);
	//my buddy's index
	int my_buddy_indx;
	if (my_indx % 2 == 0)my_buddy_indx = my_indx + 1;
	else my_buddy_indx = my_indx - 1;
	//printf("my indx %d  my_buddy_indx %d\n", indx, my_buddy_indx);

	block* my_buddy_addr = (block*)(my_buddy_indx * num * BLOCK_SIZE + (unsigned char*)ptr->first);

	//we start our search from the beginning of desired array
	block* curr = ptr->list_of_blocks[lev];

	while (curr) {
		if (curr == my_buddy_addr) {
			return my_buddy_addr;
		}
		curr = curr->next;
	}
	return (void*)0;
}

block* find_prev(block* first_on_lev, block* target) {
	block* prev = (block*)0;

	while (first_on_lev) {
		if (first_on_lev->next == target) {
			return prev;
		}
		prev = first_on_lev;
		first_on_lev = first_on_lev->next;
	}
}



//main functions**********************************************************************************************
void* init_buddy(void* space, int num_of_blocks)
{
	start_addr = space;
	num_of_all_blocks = num_of_blocks;

	//we are looking for a suitable starting address
	to_start = ((unsigned)start_addr) & (MASK);
	printf("start_addr is %x  ", start_addr);
	printf("to_start is %x  ", to_start);
	printf("num_of_all_bloks is %d\n", num_of_all_blocks);

	//our adaress is not a starting address of a block, 
	//so we are starting from second(next) block
	if ((unsigned)start_addr != to_start) {
		to_start = NEXT_BLOCK + to_start; //next block
		start_addr = (void*)to_start;
		num_of_all_blocks -= 1;
		//printf("After moving, start_addr is %x  and num_of_all_blocks is %d \n", start_addr, num_of_all_blocks);
		//since we couldn't start from the desired address, 
		//we have to start from the beginning of the next block, 
		//so we lose first block
	}

	//a new first block is for ptr_to_buddy, 
	//the buddy_system will start from a second block actually
	num_of_all_blocks--;
	ptr_to_buddy = (buddy*)start_addr;
	ptr_to_buddy->mutex_buddy = CreateMutex(0, FALSE, 0);
	int levels = get_level(num_of_all_blocks);
	ptr_to_buddy->num_of_entries = levels;
	/*		ptr_to_buddy->all_blocks = pow(2, levels);
			ptr_to_buddy->free_blocks = 0; *///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!MOZDA DA SE MENJA U 2 NA LEVELS
	ptr_to_buddy->first = (block*)start_addr + 1;
	printf("Buddy system on [%x]\n", start_addr);
	printf("First block on [%x]\n", ptr_to_buddy->first);
	printf("Levels %d\n", levels);

	//mutex = CreateMutex(0, FALSE,0);

	//the bigest block is size of 2^levels
	for (int i = 0; i <= levels; i++) {
		ptr_to_buddy->list_of_blocks[i] = 0;
	}

	block* tmp = ptr_to_buddy->first;
	for (int i = 0; i < num_of_all_blocks; i++) {
		tmp->next = 0;
		add_blocks(tmp, 1, ptr_to_buddy);
		tmp++;
	}
	//ptr_to_buddy->mutex_buddy = CreateMutex(0,FALSE,0);

	printf("\nAfter initializing, buddy state is:\n");
	print_buddy_system(ptr_to_buddy);

	return ptr_to_buddy;

}

void add_blocks(void* addr, int num_of_blocks, buddy* ptr) {
	WaitForSingleObject(ptr->mutex_buddy, INFINITE);

	buddy* ptr_to_buddy = ptr;
	int num = num_of_blocks;
	int lev = ceil(log(num) / log(2));

	void* new_begin = addr;
	block* my_buddy_addr = (block*)0;

	block* tmp = (block*)0;
	block* prev = (block*)0;
	int found = 0;

	int indx = 0;
	int my_buddy_indx = 0;

	//my index
	indx = ((unsigned char*)new_begin - (unsigned char*)ptr_to_buddy->first) / (num * BLOCK_SIZE);
	//new blok's buddy's index
	if (indx % 2 == 0)my_buddy_indx = indx + 1;
	else my_buddy_indx = indx - 1;
	//printf("my indx %d  my_buddy_indx %d\n", indx, my_buddy_indx);

	my_buddy_addr = (block*)(my_buddy_indx * num * BLOCK_SIZE + (unsigned char*)ptr_to_buddy->first);
	tmp = ptr_to_buddy->list_of_blocks[lev];

	if (ptr_to_buddy->list_of_blocks[lev] == 0) {
		//printf("INSERTING:ptr_to_buddy->list_of_blocks[%d]=%x\n", lev, (block*)start);
		((block*)new_begin)->next = (block*)0;
		ptr_to_buddy->list_of_blocks[lev] = (block*)new_begin;
		//printf("\nAfter adding %x blocks,system is:", addr);
		//print_buddy_system(ptr);
		ReleaseMutex(ptr->mutex_buddy);
		return;
	}

	//we are searching for buddy
	while (tmp) {
		if (tmp == my_buddy_addr) {
			//we found a buddy
			found = 1;
			if (prev) {
				//we need to bind the list
				prev->next = tmp->next;
			}
			if (ptr_to_buddy->list_of_blocks[lev] == tmp) {
				//I and my buddy will be on the higher level
				//We need to bind the list on this level
				//If we were first on this level, we should update ptr to first element on this level
				ptr_to_buddy->list_of_blocks[lev] = tmp->next;
			}
			//we will be on the higher level
			//in this moment, on this level, our next is 0
			//because we are leaving this level
			tmp->next = 0;
			//my_addr = new_begin;
			if (indx % 2)new_begin = my_buddy_addr;
			else new_begin = addr;
			ReleaseMutex(ptr->mutex_buddy);
			add_blocks(new_begin, num * 2, ptr_to_buddy);
			WaitForSingleObject(ptr->mutex_buddy, INFINITE);
			break;
		}
		//we continue our search
		prev = tmp;
		tmp = tmp->next;
	}

	if (found == 0) {
		//we didn't find our buddy,
		//so we will be first element on this level
		((block*)new_begin)->next = ptr_to_buddy->list_of_blocks[lev];
		ptr_to_buddy->list_of_blocks[lev] = (block*)new_begin;
	}
	//printf("\nAfter adding %x blocks,system is:",addr);
	//print_buddy_system(ptr);

	ReleaseMutex(ptr->mutex_buddy);

}

//After we take one block from buddy
//we need return other blocks, which remain free 
void connect_the_blocks(int lev, int size_req, block* start) {
	int num_of_iterations = 0;
	int size = pow(2.0, lev);

	while (size_req < size) {
		num_of_iterations++;
		size_req *= 2;
	}

	for (int i = 0; i < num_of_iterations; i++, lev--) {
		int curr_size = pow(2.0, lev);
		//sec_addr is address of a second half which stays free
		//and we need to bind it
		//we need to move it in lower level
		block* sec_addr = (block*)((unsigned char*)start + curr_size * BLOCK_SIZE / 2);
		add_blocks(sec_addr, curr_size / 2, ptr_to_buddy);
	}
}

block* get_blocks(int num_of_blocks, buddy* ptr) {
	buddy* ptr_to_buddy = ptr;

	WaitForSingleObject(ptr_to_buddy->mutex_buddy, INFINITE);
	//printf("\nAfter taking %d blocks, system is:", num_of_blocks);
	//print_buddy_system(ptr);

	int level = get_level(num_of_blocks);
	int size_req = pow(2.0, level); //required size of a block
	int num_of_blocks_which_we_take = size_req;

	if (level > ptr_to_buddy->num_of_entries) {
		printf("NO MEMORY(req(blocks):%d,entries:%d)", num_of_blocks, ptr_to_buddy->num_of_entries);
		ReleaseMutex(ptr_to_buddy->mutex_buddy);
		return 0;
	}

	buddy* budd = ptr_to_buddy;
	int temp_level = level;
	block* blck = (block*)0;

	while (1) {

		if (budd->list_of_blocks[temp_level] == 0) {
			//there are no free blocks of the desired size
			//so we seek at the higher level
			temp_level++;
			continue;
		}
		if (temp_level > ptr_to_buddy->num_of_entries) {
			//printf("\nNO MEMORY(req(blocks):%d,entries:%d)\n", num_of_blocks, ptr_to_buddy->num_of_entries);
			//printf("\ntemp_level:%d num_of_entries:%d\n", temp_level, ptr_to_buddy->num_of_entries);
			ReleaseMutex(ptr_to_buddy->mutex_buddy);
			return 0;
		}

		//there are enough memory
		//printf("THERE ARE ENOUGH MEMORY(req(blocks):%d,free(blocks):%d)", num_of_blocks, ptr_to_buddy->num_of_entries);
		//print_buddy_system(ptr);
		blck = budd->list_of_blocks[temp_level];
		budd->list_of_blocks[temp_level] = budd->list_of_blocks[temp_level]->next;
		blck->next = 0;

		connect_the_blocks(temp_level, size_req, blck);

		ReleaseMutex(ptr_to_buddy->mutex_buddy);
		return blck;

	}
}