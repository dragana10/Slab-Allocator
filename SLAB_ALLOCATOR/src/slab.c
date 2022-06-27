#include"slab.h"
#include"cache.h"
#include"buddy.h"
#include"windows.h"
#include"stdio.h"
#include<math.h>

//int i = 1;
//int unis = 0;

//auxiliary functions********************************************************************************************

void print_cache_system() {

	kmem_cache_s* tmp = ptr_to_cache_organizer->cache_all;
	int i = 0;
	printf("\n");
	while (tmp) {
		printf("[%d]: %s -> ", i, tmp->name);
		tmp = tmp->next;
		i++;
	}
	printf("\n");

}

int is_obj_in_slab(void* objp, kmem_cache_t* cache, slab* slb) {
	if (((unsigned char*)slb < (unsigned char*)objp) && (((unsigned char*)slb + cache->slab_size_in_blocks * BLOCK_SIZE) > (void*)objp)) {
		return 1;
	}
	else return 0;
}

void wait_main() {
	WaitForSingleObject(ptr_to_cache_organizer->mutex_main, INFINITE);
}

void signal_main() {
	ReleaseMutex(ptr_to_cache_organizer->mutex_main);
}



//main functions*************************************************************************************************
void kmem_init(void* space, int block_num) {
	buddy* bud = (buddy*)0;
	bud=init_buddy(space, block_num);
	ptr_to_cache_organizer = (cache_organizer*)get_blocks(1,bud);
	ptr_to_cache_organizer->ptr_to_buddy_alloc = bud;
	ptr_to_cache_organizer->mutex_main = CreateMutex(NULL, FALSE, NULL);
	ptr_to_cache_organizer->num_of_all_caches = 0;
	ptr_to_cache_organizer->cache_all = (kmem_cache_s*)0;

	////we create first cache
	//wait_main();
	//kmem_cache_t* cache = get_blocks(1);
	//signal_main();

	//cache->size_of_obj = sizeof(kmem_cache_t);
	//cache->obj_in_use = 0;

	//int for_1_obj = ceil((double)cache->size_of_obj / BLOCK_SIZE);
	//int lev_for_1 =ceil( log(for_1_obj) / log(2.0));
	//int num_blocks_needed = pow(2.0, lev_for_1);

	//int mem_needed = sizeof(slab);
	//int num_objs = 0;
	//while (1) {
	//	//there are 1 unsigned int for every obj, 
	//	//which we use to access to objs
	//	if ((mem_needed + cache->size_of_obj + sizeof(unsigned int)) < (num_blocks_needed * BLOCK_SIZE)) {
	//		mem_needed += cache->size_of_obj + sizeof(unsigned int);
	//		num_objs++;
	//	}
	//	else break;
	//}
	//if(num_objs)cache->num_of_obj_in_slab = num_objs;
	//else {
	//	cache->num_of_obj_in_slab = 1;
	//	num_blocks_needed *= 2;
	//}
	//cache->slab_size_in_blocks = num_blocks_needed;
	//cache->surplus = (num_blocks_needed * BLOCK_SIZE) - mem_needed;

	//cache->colour = floor(cache->surplus / CACHE_L1_LINE_SIZE);
	//cache->colour_off = 0;
	//if (cache->colour)cache->colour_next = 1;
	//else cache->colour_next = 0;

	//cache->growing = 0;
	//cache->error = 0;
	//strcpy(cache->err_msg, "No errors.");

	//cache->ctor = 0;
	//cache->dtor = 0;

	//strcpy(cache->name, "main_cache");
	//cache->next = (kmem_cache_t*)0;
	//cache->prev = (kmem_cache_t*)0;

	//cache->mutex = CreateMutex(NULL, FALSE, NULL);

	//cache->slabs_full = (slab*)0;
	//cache->slabs_half_filled = (slab*)0;
	//wait_main();
	//slab* new_slab = get_blocks(1);
	//new_slab->list = 0;
	//new_slab->num_of_free_slots = cache->num_of_obj_in_slab;
	//new_slab->active_obj = 0;
	//new_slab->free_next = 0;
	//for (int i = 0; i < new_slab->num_of_free_slots; i++) {
	//	slab_bufctl(new_slab)[i] = (i + 1);
	//}
	//slab_bufctl(new_slab)[new_slab->num_of_free_slots - 1] = UINT_MAX;
	//new_slab->colour_off = 0;
	//new_slab->start = (unsigned char*)new_slab + sizeof(slab) + new_slab->num_of_free_slots * sizeof(unsigned int);
	//new_slab->next = (slab*)0;
	//signal_main();
	//cache->slabs_free = new_slab;

	//ptr_to_cache_organizer->cache_all = cache;

	for (int i = 0; i < 13; i++) {
		char name[10];
		sprintf(name, "size-2^%d", i + 5);
		//printf(name);
		//printf(" \n ");
		size_t size = pow(2.0, i + 5);
		//printf("sizeN_caches[%d]=%d\n", i, size);
		ptr_to_cache_organizer->cache_sizeN[i] = kmem_cache_create(name, size, 0, 0);
		//ptr_to_cache_organizer->cache_sizeN[i] = (kmem_cache_s*)0;
	}

}

kmem_cache_t* kmem_cache_create(const char* name, size_t size, void (*ctor)(void*), void (*dtor)(void*)) {
	wait_main();

	kmem_cache_t* cache = get_blocks(1,ptr_to_cache_organizer->ptr_to_buddy_alloc);

	cache->prev = (kmem_cache_t*)0;

	if (ptr_to_cache_organizer->cache_all == 0) {
		cache->next = (kmem_cache_t*)0;
		ptr_to_cache_organizer->cache_all = cache;
	}
	else {
		ptr_to_cache_organizer->cache_all->prev = cache;
		cache->next = ptr_to_cache_organizer->cache_all;
		ptr_to_cache_organizer->cache_all = cache;
	}

	ptr_to_cache_organizer->num_of_all_caches += 1;

	signal_main();


	cache->slabs_full = (slab*)0;
	cache->slabs_half_filled = (slab*)0;
	cache->slabs_free = (slab*)0;

	cache->size_of_obj = size;
	cache->obj_in_use = 0;

	int for_1_obj = ceil((double)cache->size_of_obj / BLOCK_SIZE);
	int lev_for_1 = ceil(log(for_1_obj) / log(2.0));
	int num_blocks_needed = pow(2.0, lev_for_1);

	int mem_needed = sizeof(slab);
	int num_objs = 0;
	while (1) {
		//there are 1 unsigned int for each obj, 
		//which we use to access to objs
		if ((mem_needed + cache->size_of_obj + sizeof(unsigned int)) < (num_blocks_needed * BLOCK_SIZE)) {
			mem_needed += cache->size_of_obj + sizeof(unsigned int);
			num_objs++;
		}
		else break;
	}
	if (num_objs)cache->num_of_obj_in_slab = num_objs;
	else {
		cache->num_of_obj_in_slab = 1;
		num_blocks_needed *= 2;
	}
	cache->slab_size_in_blocks = num_blocks_needed;
	cache->surplus = (num_blocks_needed * BLOCK_SIZE) - mem_needed;
	cache->allocated_slabs = 0;

	cache->colour = floor(cache->surplus / CACHE_L1_LINE_SIZE);
	cache->colour_off = 0;
	//if (cache->colour)cache->colour_next = (cache->colour_next + 1) % cache->colour;
	//else cache->colour_next = 0;
	cache->colour_next = 0;

	cache->growing = 0;
	cache->error = 0;
	strcpy(cache->err_msg, "No errors.");

	cache->ctor = ctor;
	cache->dtor = dtor;

	strcpy(cache->name, name);

	cache->mutex = CreateMutex(NULL, FALSE, NULL);

	//printf("Created cache for:%s    colour:%d   obj_size:%d\n\n",cache->name,cache->colour,cache->size_of_obj);
	//print_cache_system();
	return cache;
	
}

void* kmem_cache_alloc(kmem_cache_t* cachep) {
	wait_main();

	slab* slb = (slab*)0;
	void* obj = (void*)0;

	if (cachep == (kmem_cache_t*)0) {
		printf("Wrong arguments for kmem_cache_alloc.\n");
		signal_main();
		return(void*)0;
	}
	WaitForSingleObject(cachep->mutex, INFINITE);
	//wait_main();
	if (cachep->slabs_half_filled != (slab*)0) {
		slb = cachep->slabs_half_filled;
	}
	else if (cachep->slabs_free != (slab*)0) {
		slb = cachep->slabs_free;
		cachep->slabs_free = cachep->slabs_free->next;

		slb->next = cachep->slabs_half_filled;
		cachep->slabs_half_filled = slb;
		slb->list = 1;
	}
	else { //we need to allocate one slab
		cachep->growing = 1;
		cachep->allocated_slabs += 1;
		slb = get_blocks(cachep->slab_size_in_blocks,ptr_to_cache_organizer->ptr_to_buddy_alloc);
		if (slb == (slab*)0) {
			printf("Error. There are not enough memory.\n");
			cachep->error = -1;
			strcpy(cachep->err_msg, "No enough memory.");
			//signal_main();
			ReleaseMutex(cachep->mutex);
			signal_main();
			return (void*)0;
		}

		slb->list = 1;
		slb->num_of_free_slots = cachep->num_of_obj_in_slab;
		slb->active_obj = 0;
		slb->free_next = 0;
		for (int i = 0; i < slb->num_of_free_slots-1; i++) {
			slab_bufctl(slb)[i] = (i + 1);
		}
		slab_bufctl(slb)[slb->num_of_free_slots - 1] = UINT_MAX;

		//printf("\n");
		//for (int i = 0; i < slb->num_of_free_slots; i++) {
		//	printf("%d,   ", slab_bufctl(slb)[i]);
		//}
		//printf("\n");


		//slb->colour_off = cachep->colour_off;
		slb->start = (unsigned char*)slb + sizeof(slab) + slb->num_of_free_slots * sizeof(unsigned int) + CACHE_L1_LINE_SIZE * cachep->colour_next;
		if (cachep->colour)cachep->colour_next = (cachep->colour_next + 1) % cachep->colour;
		else cachep->colour_next = 0;
		slb->next = cachep->slabs_half_filled;
		cachep->slabs_half_filled = slb;
		//printf("\nnew slab for cache:%s   colour_next:%d   blocks:%d   obj_size:%d\n", cachep->name, cachep->colour_next, cachep->slab_size_in_blocks,cachep->size_of_obj);
		//printf("\nobjs:%d\n", slb->num_of_free_slots);

	}

	if (slb != (slab*)0) {
		obj = (unsigned char*)slb->start + slb->free_next * cachep->size_of_obj;
		slb->free_next = slab_bufctl(slb)[slb->free_next];
		slb->active_obj += 1;
		slb->num_of_free_slots -= 1;
		cachep->obj_in_use += 1;
		//if (cachep->obj_in_use == 128) {
		//	printf("\nSAD\n");
		//}
		//if(slb->num_of_free_slots<0){
		//printf("\nu %s ima obj akt :%d  free slots:%d   ukup obj:%d   obj_size:%d\n", cachep->name, cachep->obj_in_use, slb->num_of_free_slots, cachep->num_of_obj_in_slab,cachep->size_of_obj);
		//printf("\nobjs:%d    objs:%d    objs:%d      objs:%d      objs:%d\n", cachep->obj_in_use, cachep->obj_in_use, cachep->obj_in_use, cachep->obj_in_use, cachep->obj_in_use);
		//}
		if (cachep->ctor) {
			//printf("\nkons se zove %d put !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", i);
			cachep->ctor(obj);
			//i++;
		}

		if (slb->num_of_free_slots == 0) {
			cachep->slabs_half_filled = cachep->slabs_half_filled->next;
			slb->list = 2;
			slb->next = cachep->slabs_full;
			cachep->slabs_full = slb;
		}
	}

	//signal_main();
	ReleaseMutex(cachep->mutex);
	signal_main();
	return obj;
}

void* kmalloc(size_t size) {
	if (size < 32 || size>131072) {
		printf("\nWRONG ARGUMENTS for kmalloc.\nArguments have to be between 32 and 131072.\n");
		return (void*)0;
	}

	int lev = (ceil(log((double)size) / log(2.0))) - 5;
	return kmem_cache_alloc(ptr_to_cache_organizer->cache_sizeN[lev]);
}

//this function deallocate one object from cache
//we should update bufctl :
//unsigned int indx=(obj->slab->start)/obj_size;
//slab_bufctl(slab)[indx]=slab->free_next;
//slab->free_next=indx;
void kmem_cache_free(kmem_cache_t* cachep, void* objp) {
	if (cachep == (kmem_cache_t*)0 || objp == (void*)0) {
		printf("\nWrong arguments for kmem_cache_free.\n");
		return;
	}

	wait_main();
	WaitForSingleObject(cachep->mutex, INFINITE);

	//we need to find out which slabs list member
	//is this obj
	slab* tmp = cachep->slabs_full;
	int type = 0;
	int blks = cachep->slab_size_in_blocks;
	slab* req_slab = (slab*)0;
	slab* prev = 0;
	int i = 0;
	while (i < cachep->allocated_slabs && type==0) {
		if (tmp && type == 0) {
			if (is_obj_in_slab(objp, cachep, tmp)) {
				req_slab = tmp;
				type = 2;
				//printf("\nSLAB IS IN FULL   req_slab->start:%x   objp:%x\n", req_slab->start, objp);
				break;
			}
			else {
				i++;
				prev = tmp;
				tmp = tmp->next;
				//if(tmp)printf(" tmp=next %x=%x", tmp, tmp->next);
				continue;
			}
		}
		if(type==0)tmp = cachep->slabs_half_filled;
		if (tmp && type == 0) {
			if (is_obj_in_slab(objp, cachep, tmp)) {
				req_slab = tmp;
				type = 1;
				//printf("\nSLAB IS IN HALF   req_slab->start:%x   objp:%x   tmp:%x    tmp->next:%x   alloc_slb:%d\n", req_slab->start, objp,tmp,tmp->next,cachep->allocated_slabs);
				break;
			}
			else {
				i++;
				prev = tmp;
				tmp = tmp->next;
				//if(tmp)printf(" tmp=next %x=%x", tmp, tmp->next);
				continue;
			}
		}
	}

	//while (tmp != (slab*)0) {
	//	if (((unsigned char*)tmp<(unsigned char*)objp) && (((unsigned char*)tmp + blks * BLOCK_SIZE)>(void*)objp)) {
	//		req_slab = tmp;
	//		type = 2;
	//		printf("\nSLAB IS IN FULL   req_slab->start:%x   objp:%x\n", req_slab->start, objp);
	//		break;
	//	}
	//	//prev = tmp;
	//	tmp - tmp->next;
	//	printf(" tmp=next %x=%x", tmp, tmp->next);
	//}
	//if (type == 0) {
	//	tmp = cachep->slabs_half_filled;
	//	while (tmp != (slab*)0) {
	//		if (((unsigned char*)tmp < (unsigned char*)objp) && (((unsigned char*)tmp + blks * BLOCK_SIZE) > (void*)objp)) {
	//			req_slab = tmp;
	//			type = 1;
	//			printf("\nSLAB IS IN HALF   req_slab->start:%x   objp:%x\n", req_slab->start, objp);
	//			break;
	//		}
	//		tmp - tmp->next;
	//		printf(" tmp=next %x=%x", tmp, tmp->next);
	//	}
	//}
	if (type == 0) {
		//printf("\nObject not found.\n");
		//printf("\ncachep->half:%x   cachep->full:%x   objp:%x   cachep:%x\n",cachep->slabs_half_filled,cachep->slabs_full,objp,cachep);

		cachep->error = (-1);
		strcpy(cachep->err_msg, "Object not found.");
		ReleaseMutex(cachep->mutex);
		signal_main();
		return;
	}

	unsigned int indx=(((unsigned int)objp)-(unsigned int)req_slab->start)/cachep->size_of_obj;
	if (((unsigned int)req_slab->start + indx * cachep->size_of_obj)!=(unsigned int)objp) {
		printf("\nGreskaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("\nindx:%d   req_slab->start:%x   objp:%x\n",indx,req_slab->start,objp);
		ReleaseMutex(cachep->mutex);
		signal_main();
		return;
	}
	slab_bufctl(req_slab)[indx]= req_slab->free_next;
	req_slab->free_next=indx;

	req_slab->num_of_free_slots += 1;
	req_slab->active_obj -= 1;
	cachep->obj_in_use -= 1;

	if (cachep->dtor) {
		cachep->dtor(objp);
	}
	//we call constructor, 
	//because we want to leave this object in initialised state
	if (cachep->ctor) {
		cachep->ctor(objp);
	}

	//unis++;
	//printf(" %d", unis);

	slab* next = (slab*)0;
	slab* curr = (slab*)0;

	if (type == 2) {
		curr = cachep->slabs_full;
		//if (curr == req_slab) {
		//	prev = (slab*)0;
		//}
		//else {
		//	while (curr) {
		//		if (curr->next == req_slab) {
		//			prev = curr;
		//			break;
		//		}
		//		curr = curr->next;
		//		printf("\n we are looking for slabsFULL:%x   curr:%x   curr->next:%x        objp:%x \n", cachep->slabs_full,curr, curr->next,objp);
		//	}
		//}
		next = req_slab->next;
		if (prev != (slab*)0) {
			prev->next = next;
		}
		if (req_slab == cachep->slabs_full) {
			cachep->slabs_full = cachep->slabs_full->next;
		}
		if (req_slab->active_obj == 0) {
			req_slab->next = cachep->slabs_free;
			cachep->slabs_free = req_slab;
			req_slab->list = 0;
		}
		else {
			req_slab->next = cachep->slabs_half_filled;
			cachep->slabs_half_filled = req_slab;
			req_slab->list = 1;
		}
	}
	else {
		if (req_slab->active_obj == 0) {
			curr = cachep->slabs_half_filled;
			if (curr == req_slab) {
				prev = (slab*)0;
			}
			//else {
			//	while (curr) {
			//		if (curr->next == req_slab) {
			//			prev = curr;
			//			break;
			//		}
			//		curr = curr->next;
			//		printf("\n we are looking for slabsHALF:%x   curr:%x   curr->next:%x        objp:%x \n", cachep->slabs_half_filled, curr, curr->next, objp);

			//	}
			//}
			next = req_slab->next;
			if (prev != (slab*)0) {
				prev->next = next;
			}
			if (req_slab == cachep->slabs_half_filled) {
				cachep->slabs_half_filled = cachep->slabs_half_filled->next;
			}

			req_slab->next = cachep->slabs_free;
			cachep->slabs_free = req_slab;
			req_slab->list = 0;
			
		}
	}

	ReleaseMutex(cachep->mutex);
	signal_main();
}

void kfree(const void* objp) {
	if (objp == 0) {
		printf("\nWrong arguments for kfree.\n");
		return;
	}

	for (int i = 0; i < 13; i++) {
		kmem_cache_free(ptr_to_cache_organizer->cache_sizeN[i], objp);
	}
}

//this function should delete all slabs in slabs_empty list
//(unless the growing flag has set) 
//it returns number of pages freed 
int kmem_cache_shrink(kmem_cache_t* cachep) {
	if (cachep == (kmem_cache_t*)0) {
		printf("Wrong arguments for kmem_cache_shrink.\n");
		cachep->error = -1;
		strcpy(cachep->err_msg, "Wrong arguments.");
		return 0;
	}

	WaitForSingleObject(cachep->mutex, INFINITE);

	if (cachep->growing) {
		cachep->growing = 0;
		ReleaseMutex(cachep->mutex);
		return 0;
	}

	cachep->growing = 0;
	slab* slb = (slab*)0;
	slb = cachep->slabs_free;
	int ret = 0;
	void* obj = (void*)0;
	if (slb == (slab*)0) {
		return 0;
	}
	else {
		while (slb)
		{
			if (cachep->dtor) {
				for (int i = 0; i < slb->active_obj; i++) {
					obj =(void*) ((unsigned int)slb->start + cachep->size_of_obj * i);
					if (slab_bufctl(slb)[i] != UINT_MAX) {
						cachep->dtor(obj);
					}
				}
			}
			ret += (slb->num_of_free_slots + slb->active_obj);
			wait_main();
			add_blocks(slb, cachep->slab_size_in_blocks,ptr_to_cache_organizer->ptr_to_buddy_alloc);
			signal_main();
			cachep->allocated_slabs -= 1;
			slb = slb->next;
		}
		cachep->slabs_free = (slab*)0;
	}

	//ReleaseMutex(cachep->mutex);
	return ret;
}

void kmem_cache_destroy(kmem_cache_t* cachep) {
	if (cachep == (kmem_cache_t*)0) {
		printf("Wrong arguments for kmem_cache_destroy.\n");
		return;
	}

	WaitForSingleObject(cachep->mutex, INFINITE);
	wait_main();

	if (cachep == ptr_to_cache_organizer->cache_all) {
		ptr_to_cache_organizer->cache_all = ptr_to_cache_organizer->cache_all->next;
		ptr_to_cache_organizer->cache_all->prev = (kmem_cache_t*)0;
	}
	else {
		kmem_cache_t* prev = cachep->prev;
		kmem_cache_t* next = cachep->next;
		if (prev)prev->next = next;
		if (next)next->prev = prev;
	}
	cachep->prev = cachep->next = (kmem_cache_t*)0;

	slab* slb = (slab*)0;
	slb = cachep->slabs_free;
	int freed = 0;
	void* obj = (void*)0;

	if (slb != (slab*)0) {
		while (slb)
		{
			if (cachep->dtor) {
				for (int i = 0; i < slb->active_obj; i++) {
					obj = (void*)((unsigned int)slb->start + cachep->size_of_obj * i);
					if (slab_bufctl(slb)[i] != UINT_MAX) {
						cachep->dtor(obj);
					}
				}
			}
			freed += (slb->num_of_free_slots + slb->active_obj);
			add_blocks(slb, cachep->slab_size_in_blocks, ptr_to_cache_organizer->ptr_to_buddy_alloc);
			cachep->allocated_slabs -= 1;
			slb = slb->next;
		}
		cachep->slabs_free = (slab*)0;
	}

	slb = cachep->slabs_half_filled;

	if (slb != (slab*)0) {
		while (slb)
		{
			if (cachep->dtor) {
				for (int i = 0; i < slb->active_obj; i++) {
					obj = (void*)((unsigned int)slb->start + cachep->size_of_obj * i);
					if (slab_bufctl(slb)[i] != UINT_MAX) {
						cachep->dtor(obj);
					}
				}
			}
			freed += (slb->num_of_free_slots + slb->active_obj);
			add_blocks(slb, cachep->slab_size_in_blocks, ptr_to_cache_organizer->ptr_to_buddy_alloc);
			cachep->allocated_slabs -= 1;
			slb = slb->next;
		}
		cachep->slabs_half_filled = (slab*)0;
	}

	slb = cachep->slabs_full;

	if (slb !=(slab*)0){
		while (slb)
		{
			if (cachep->dtor) {
				for (int i = 0; i < slb->active_obj; i++) {
					obj = (void*)((unsigned int)slb->start + cachep->size_of_obj * i);
					if (slab_bufctl(slb)[i] != UINT_MAX) {
						cachep->dtor(obj);
					}
				}
			}
			freed += (slb->num_of_free_slots + slb->active_obj);
			add_blocks(slb, cachep->slab_size_in_blocks, ptr_to_cache_organizer->ptr_to_buddy_alloc);
			cachep->allocated_slabs -= 1;
			slb = slb->next;
		}
		cachep->slabs_full = (slab*)0;
	}

	add_blocks(cachep, 1, ptr_to_cache_organizer->ptr_to_buddy_alloc);

	ptr_to_cache_organizer->num_of_all_caches -= 1;
	if (ptr_to_cache_organizer->num_of_all_caches==0) {
		add_blocks(ptr_to_cache_organizer, 1, ptr_to_cache_organizer->ptr_to_buddy_alloc);
	}
	signal_main();
	ReleaseMutex(cachep->mutex);
}

void kmem_cache_info(kmem_cache_t* cachep) {
	if (cachep == (kmem_cache_t*)0) {
		printf("No cache to read information");
		return;
	}
	else {
		wait_main();
		WaitForSingleObject(cachep->mutex, INFINITE);
		//wait_main();
		int size = cachep->slab_size_in_blocks*BLOCK_SIZE;
		int total = cachep->allocated_slabs * size;
		double used = (double)((double)cachep->obj_in_use *100.0/((double)cachep->allocated_slabs * (double)cachep->num_of_obj_in_slab) );
		//int free = total - used;
		//slab* slb = cachep->slabs_free;
		//while (slb) {
		//	free += size;
		//	total += size;
		//	slb = slb->next;
		//}
		//slb = cachep->slabs_half_filled;
		//while (slb) {
		//	free += slb->num_of_free_slots;
		//	used += slb->active_obj;
		//	total += size;
		//	slb = slb->next;
		//}
		//slb = cachep->slabs_full;
		//while (slb) {
		//	free += slb->num_of_free_slots;
		//	used += slb->active_obj;
		//	total += size;
		//	slb = slb->next;
		//}
		printf("\n\n-------------------------------------------------------");
		printf("\n%55s\n","information about cache:");
		printf("%-23s", "name:");
		printf(cachep->name);
		printf("\n%-23s", "object size(B):");
		printf("%d", cachep->size_of_obj);
		printf("\n%-23s", "slab size (blocks):");
		printf("%d", cachep->slab_size_in_blocks);
		printf("\n%-23s", "slabs allocated:");
		printf("%d", cachep->allocated_slabs);
		printf("\n%-23s", "slots in one slab:");
		printf("%d", cachep->num_of_obj_in_slab);
		printf("\n%-23s", "active objects:");
		printf("%d", cachep->obj_in_use);
		printf("\n%-23s", "memory total(B/blocks):");
		printf("%d/%d", total,cachep->allocated_slabs*cachep->slab_size_in_blocks);
		printf("\n%-23s", "memory used(%):");
		printf("%.2f\n", used);
		printf("-------------------------------------------------------\n\n");
	}
	//signal_main();
	ReleaseMutex(cachep->mutex);
	signal_main();

}

int kmem_cache_error(kmem_cache_t* cachep) {
	printf("Error code:%d\n", cachep->error);
	printf(cachep->err_msg);
	printf("\n");
	return 1;
}