#ifndef cache_h
#define cache_h

#include"buddy.h"
#include"windows.h"

#define NUM_OF_MEM_BUFFERS 13

typedef unsigned int kmem_bufctl_t;
#define slab_bufctl(slabp) \
		((kmem_bufctl_t *)(((slab*)slabp)+1))


typedef struct slab {
	//which list this slab belongs to
	//0 free,1 half filled,2 full
	int list;
	//the address of the 1st obj in this slab
	//we calculate it as:
	//(unsigned char*)new_slab+sizeof(slab) ----------------here we have 1st address after [new_slab]
	//+new_slab->num_of_free_slots*sizeof(unsigned int)-----here we save space for array which we use to access objects
	//+new_slab->colour_off---------------------------------here we add offset
	void* start;
	//number of free slots
	unsigned num_of_free_slots;
	//num of active object in slab
	unsigned active_obj;
	//number of elements in this array
	//is the same as the number of objs on the slab
	kmem_bufctl_t* bufctl;
	//at the beginning free_next will be 0
	//bcs first free obj has index 0
	//(for obj n, next_free obj will be stored in bufstl[n])
	//beginning:1st free obj on slab->free_next
	//			2nd free obj on bufctl[slab->free_next]
	//obj=slab->start+slab->free_next*obj_size
	//slab->free_next=bufctl(slab)[slab->free_next]
	kmem_bufctl_t free_next;
	//the colour offset from the base address 
	//of the first object within the slab
	//the address od the 1st obj is start+colour_off
	//unsigned colour_off;
	//slab is part of a list
	struct slab* next;
}slab;


//we have 1 cache for each type of object
typedef struct kmem_cache_s {
	slab* slabs_full;
	slab* slabs_half_filled;
	slab* slabs_free;

	//size of obj is equal to 
	//the size of the object for 
	//which this cache is intended
	//that is size of 1 slot
	//when we want to alocate 1 obj, 
	//we need to take 1 slot
	unsigned size_of_obj;
	unsigned obj_in_use;
	unsigned num_of_obj_in_slab; //num of slots in a slab
	unsigned slab_size_in_blocks; //in blocks
	unsigned surplus; //wasted space
	unsigned allocated_slabs;

	unsigned colour;
	unsigned colour_off;
	unsigned colour_next; //next colour line to use

	unsigned growing; //whether the cache grew up or not
	int error;
	char err_msg[70];

	void (*ctor)(void*); //ptr to constructor function of new obj
	void (*dtor)(void*); //ptr to destructor function of new obj

	char name[50];
	struct kmem_cache_s* next;
	struct kmem_cache_s* prev;

	HANDLE mutex;
}kmem_cache_s;

typedef struct cache_organizer {
	struct buddy* ptr_to_buddy_alloc;
	unsigned num_of_all_caches;
	struct kmem_cache_s* cache_all;
	//struct kmem_cache_s* cache_chain;
	struct kmem_cache_s* cache_sizeN[NUM_OF_MEM_BUFFERS];
	HANDLE mutex_main;
}cache_organizer;

struct cache_organizer* ptr_to_cache_organizer;

#endif // !cache_h