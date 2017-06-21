#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include <math.h>
#include "data_structures.h"

void get_tag_and_index(struct cons_cache_controller *cache_cont, int *tag_and_index, int addr)
{
	int tag = addr >> (*cache_cont).tag_filter_bits;
	int index = addr >> (*cache_cont).index_filter_bits;
	index = index % (*cache_cont).set_numbers;

	tag_and_index[0] = tag;
	tag_and_index[1] = index;
}

// Declaration of cache_controller
void update_LRU(struct cons_cache_controller *self, int index, int target_way)
{
	int i;
	for (i = 0; i < (*self).way_numbers; i++)
	{
		if ( (*self).LRU[index][i] < (*self).LRU[index][target_way] )
		{
			(*self).LRU[index][i]++;
		}
	}
	(*self).LRU[index][target_way] = 0;	
}

int find_victim(struct cons_cache_controller *self, int index)
{
	int victim = 0;
	int i;
	for (i = 0; i < (*self).way_numbers; i++)
	{
		if ( (*self).LRU[index][i] == ((*self).way_numbers - 1) )
		{
			victim = i;
			break;
		}
	}

	return victim;
}

void cache_write(struct cons_cache_controller *self, struct cons_cache *cache, int addr)
{
	int tag_and_index[2];
	get_tag_and_index(self, tag_and_index, addr);
	int tag = tag_and_index[0];
	int index = tag_and_index[1];

	int target_way = (*self).find_victim(self, index);
	(*cache).data[index][target_way][0] = tag;  // Writing tag
	(*cache).data[index][target_way][1] = 1;    // Now this block is dirty 
	(*self).update_LRU(self, index, target_way);
}

void cache_read(struct cons_cache_controller *self, struct cons_cache *cache, int addr)
{
	int tag_and_index[2];
	get_tag_and_index(self, tag_and_index, addr);
	int tag = tag_and_index[0];
	int index = tag_and_index[1];

	int target_way = (*self).search(self, cache, addr);
       	(*self).update_LRU(self, index, target_way);	
}


void cache_fill(struct cons_cache_controller *self, struct cons_cache *cache, struct statistics *stat, int addr)
{
	int tag_and_index[2];
	get_tag_and_index(self, tag_and_index, addr);
	int tag = tag_and_index[0];
	int index = tag_and_index[1];

	int target_way = (*self).find_victim(self, index);
	if ( (*cache).data[index][target_way][2] == 1)
	{
		if ( (*cache).data[index][target_way][1] == 1)
		{
			(*stat).Dirty_evictions++;
		}
		else
		{
			(*stat).Clean_evictions++;
		}
	}

	// Load desired block from RAM
	(*cache).data[index][target_way][0] = tag;  // Write new data to the block
	(*cache).data[index][target_way][1] = 0;    // This is clean block
	(*cache).data[index][target_way][2] = 1;    // This block is now valid
	(*self).update_LRU(self, index, target_way);
}

int search(struct cons_cache_controller *self, struct cons_cache *cache, int addr)
{
	// Search whether that block exists or not
	int target_way = - 1;

	int tag_and_index[2];
	get_tag_and_index(self, tag_and_index, addr);
	int tag = tag_and_index[0];
	int index = tag_and_index[1];

	int way;
	for (way = 0; way < (*self).way_numbers; way++)
	{
		if ( (*cache).data[index][way][0] == tag && (*cache).data[index][way][2] == 1)
		{
			target_way = way;
			break;
		}
	}

	return target_way;
}

bool read_try(struct cons_cache_controller *self, struct cons_cache *cache, struct statistics *stat, int addr)
{
	// Try to read from cache[ <addr> ]
	(*stat).Read_accesses++; // Count read access
	int tag_and_index[2];
	get_tag_and_index(self, tag_and_index, addr);
	int tag = tag_and_index[0];
	int index = tag_and_index[1];

	int target_way = (*self).search(self, cache, addr);
	if ( target_way == -1)
	{
		(*stat).Read_misses++;
		return false;
	}
	else
	{
		// (*self).update_LRU(self, index, target_way);
		return true;
	}		
}

bool write_try(struct cons_cache_controller *self, struct cons_cache *cache, struct statistics *stat, int addr)
{
	// Try to read from cache[ <addr> ]
	(*stat).Write_accesses++;
	int tag_and_index[2];
	get_tag_and_index(self, tag_and_index, addr);
	int tag = tag_and_index[0];
	int index = tag_and_index[1];

	int target_way = (*self).search(self, cache, addr);
	if ( target_way == -1)
	{
		(*stat).Write_misses++;
		return false;
	}
	else
	{
		//(*cache).data[index][target_way][0] = tag;  // Save tag
		//(*cache).data[index][target_way][1] = 1;    // Dirty bit set
		//(*self).update_LRU(self, index, target_way);
		return true;
	}
}

bool cache_query(struct cons_cache_controller *cache_cont, struct cons_cache *cache, struct statistics *stat, int access_type, int addr)
{
	bool result;
	if (access_type == MemRead)
	{
		result = (*cache_cont).read_try(cache_cont, cache, stat, addr);
	}
	else if (access_type == MemWrite)
	{
		result = (*cache_cont).write_try(cache_cont, cache, stat, addr);
	}
	else
	{
		printf("Warning! Illegal request to cache controller: %d\n", access_type);
		result = false;
	}

	return result;
}

void cache_filler(struct cons_cache_controller *cache_cont, struct cons_cache *cache, struct statistics *stat, int addr)
{
	(*cache_cont).cache_fill(cache_cont, cache, stat, addr);
}

void cache_reader(struct cons_cache_controller *cache_cont, struct cons_cache *cache,  int addr)
{
	(*cache_cont).cache_read(cache_cont, cache, addr);
}

void cache_writer(struct cons_cache_controller *cache_cont, struct cons_cache *cache, int addr)
{
	(*cache_cont).cache_write(cache_cont, cache, addr);
}

void *cache_initializer(struct cache_config *config)
{
	int capacity = (*config).capacity;
	int block_size = (*config).block_size;
	int way_numbers = (*config).way_numbers;
	int set_numbers = (*config).set_numbers;
	
	// Initializing cache_controller *cache_cont 
	struct cons_cache_controller *cache_cont = malloc(sizeof(struct dummy) + ((sizeof(int) * way_numbers * set_numbers)));
	// Basic variables initialization
	(*cache_cont).capacity = capacity;
	(*cache_cont).block_size = block_size;
	(*cache_cont).way_numbers = way_numbers;
	(*cache_cont).set_numbers = set_numbers;
	
	// LRU data structure initilization
	int i, j;
	for (i = 0; i < set_numbers; i++)
	{
		for (j = 0; j < way_numbers; j++)
		{
			(*cache_cont).LRU[i][j] = way_numbers - (j + 1);
		}
	}

	// Methods initialization
	(*cache_cont).update_LRU = update_LRU;
	(*cache_cont).find_victim = find_victim;
	(*cache_cont).cache_fill = cache_fill;
	(*cache_cont).cache_read = cache_read;
	(*cache_cont).cache_write = cache_write;
	(*cache_cont).search = search;
	(*cache_cont).read_try = read_try;
	(*cache_cont).write_try = write_try;
	
	// Initializing cache *cache
	struct cons_cache *cache = calloc((3 * way_numbers * set_numbers), sizeof(int));
	
	// Initializing statistics *stat
	struct statistics *stat;
	(*stat).Read_accesses = 0;
	(*stat).Write_accesses = 0;
	(*stat).Read_misses = 0;
	(*stat).Write_misses = 0;
	(*stat).Clean_evictions = 0;
	(*stat).Dirty_evictions = 0;

	// Returning Declared & Initialized objects
	void **objects = malloc(sizeof(void *) * 3);
	objects[0] = cache_cont;
	objects[1] = cache;
	objects[2] = stat;
}
