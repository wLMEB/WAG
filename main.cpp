#include <iostream>
#include <typeinfo>
#include <cmath>
#include <Block.h>
#include <Cache.h>
#include <DRAM.h>
using namespace std;



class Cache
{ // word is 32 bit = size of an integer 4byte
public:
    int response_time;         // number of cycles required for access
    int size;                  // size of the cache in blocks
    int current_cycles = 1;    // number of cycles I have been servicing memory request
    Cache *upper_cache = NULL; // cache above me (NULL if L1)
    Cache *lower_cache = NULL; // cache below me (NULL if lowest level cache)
    DRAM *dram = NULL;         // DRAM (NULL If not lowest level cache)
    Block *block_list;         // blocks within me
    int associativity;         // the associativity of the cache
    int current_id = 0;        // the id of the pipeline stage being serviced, 0 for no id
    bool requesting = 0;       // is the cache making a request of a lower level cache/DRAM

    Cache(int _size, int _response_time, int _associativity) // _size as in how many blocks
    {
        size = _size;
        response_time = _response_time;
        associativity = _associativity;
        // Block* block_list = (Block*) malloc(sizeof(Block) * size);
        block_list = new Block[size];
    };

    bool set_other_caches(Cache* _upper_cache, Cache* _lower_cache, DRAM* _dram)
    {
        upper_cache = _upper_cache;
        lower_cache = _lower_cache;
        dram = _dram;
    }
    bool time_to_service(int request_id)
    {
        if (current_id == 0)
        {
            current_id == request_id;
        }
        // if servicing a different request, return null
        else if (current_id != request_id)
        {
            return 0;
        }
        // wait until the cycles have been incremented appropriately.
        if (current_cycles < response_time)
        {
            return 0;
        }
        return 1;
    }
    // increments the cycle_count if the cache is working on a request
    bool increment_cycle_count()
    {
        if ((current_id != 0) && (requesting == 0))
        {
            current_cycles++;
        }
        if (requesting == 1)
        {
            if (lower_cache != NULL)
            {
                lower_cache->increment_cycle_count();
            }
            if (dram != NULL)
            {
                dram->increment_cycle_count();
            }
        }
    }

    //evict the block specified by addr
    //addr is the address stored in the block

    bool evict_block(int addr)
    {

        int set = (addr / 32) % (size / associativity); // the set we are going to be looking in, block# % #of sets
        int current_block = 0 + set;                  // the first block in that set


        //look for the block and delete it if found
        while (current_block < size)
        {
            if (block_list[current_block].addr == addr)
            {
                block_list[current_block].referenced = 0;
                block_list[current_block].in_use = 0;
                block_list[current_block].dirty = 0;
                block_list[current_block].addr = 0;
                for (int i = 0; i<8; i++){block_list[current_block].data[i] = 0;}

                if (upper_cache!=NULL){upper_cache->evict_block(addr);}

                return 1;
            }
            else
            {
                return 0;
            }
        }
        

    }
    // given an lowest byte address
    bool write_block(int addr, Block *upper_level_block, int request_id) 
    {
        if (!time_to_service(request_id))
        {
            return 0;
        }
        Block* dram_block = &block_list[(addr / 32)];
        for (int i = 0; i < 8; i++)
        {
            dram_block->data[i] = upper_level_block->data[i];
        }
        current_cycles = 0; // accepting new services
        current_id = 0;
        return 1;
    }
    // addr must be a multiple of 32, the lowest order byte in a block
    // *block_data[8] is a pointer to the array where the data will be sent
    // id is an integer corresponding to the stage of the pipeline accessing

    bool get_block(int addr, int *block_data[8], int id) // must be a multiple of 32
    {

        if (!time_to_service(id))
        {
            return 0;
        }

        int block_addr = addr;                                // the bottom address of data within the block we are looking for
        int set = (block_addr / 32) % (size / associativity); // the set we are going to be looking in, block# % #of sets
        int current_block = 0 + set;                          // the first block in that set

        // look for the data in the current cache, return it if found
        while (current_block < size)
        {
            if (block_list[current_block].addr == block_addr)
            {
                block_list[current_block].referenced = 1;
                for (int i = 0; i<8; i++){
                    *block_data[i] = block_list[current_block].data[i];
                }
            }
            else
            {
                current_block += size / associativity;
            }
        }

        int *new_block_data[8];

        // wait until the receive the block from lower-cache
        while (lower_cache->get_block(block_addr, new_block_data, id) == 0)
        {
            return 0;
        }

        // look for a block to evict
        current_block = 0 + set;
        while (block_list[current_block].referenced == 1)
        {
            block_list[current_block].referenced = 0;
            current_block += associativity;
            if (current_block > size)
            {
                current_block = set;
            }
        }

        evict_block(block_list[current_block].addr);

        // store new data in the block
        block_list[current_block].referenced = 1;
        block_list[current_block].in_use = 1; // valid bit
        block_list[current_block].addr = block_addr;
        for (int i; i < 8; i++)
        {
            block_list[current_block].data[i] = *new_block_data[i];
        }

        for (int i = 0; i<8; i++){
            *block_data[i] = block_list[current_block].data[i];
        }
        current_cycles = 0;                               // accepting new services
        current_id = 0;
        return 1;

    }
    // addr must be a multiple of 4, the lowest order byte in a word
    // word is a pointer to where the data will be sent
    // id is an integer corresponding to the stage of the pipeline accessing

    bool get_word(int addr, int *word, int id)
    {
        if (!time_to_service(id))
        {
            return 0;
        }

        int block_addr = (addr / 32) * 32;                    // the bottom address of data within the block we are looking for
        int set = (block_addr / 32) % (size / associativity); // the set we are going to be looking in, block# % #of sets
        int current_block = 0 + set;                          // the first block in that set

        // look for the data in the current cache, return it if found
        while (current_block < size)
        {
            if (block_list[current_block].addr == block_addr)
            {
                block_list[current_block].referenced = 1;
                *word = block_list[current_block].data[addr / 4];
            }
            else
            {
                current_block += size / associativity;
            }
        }

        int *new_block_data[8];

        // wait until the receive the block from lower-cache
        while (lower_cache->get_block(block_addr, new_block_data, id) == 0)
        {
            return 0;
        }

        // look for a block to evict
        current_block = 0 + set;
        while (block_list[current_block].referenced == 1)
        {
            block_list[current_block].referenced = 0;
            current_block += associativity;
            if (current_block > size)
            {
                current_block = set;
            }
        }

        // evict the block
        block_list[current_block].referenced = 1;
        block_list[current_block].in_use = 1; // valid bit
        block_list[current_block].addr = block_addr;
        for (int i; i < 8; i++)
        {
            block_list[current_block].data[i] = *new_block_data[i];
        }

        *word = block_list[current_block].data[addr / 4]; 
        current_cycles = 0;                               // accepting new services
        current_id = 0;
        return 1;
    }
};

void load(int addr){
    
}

int main(void)
{
    Cache L1(1000, 10, 1);  // 32KB, 10 cycles of delay, directmapped
    Cache L2(2000, 20, 2);  // 64KB, 60 cycles of delay, 2 way associate
    
    DRAM dram(20000, 200);  // 640KB, 200 cycle

    int data[8];
    L1.set_other_caches(NULL, &L2, NULL);
    L2.set_other_caches(&L1, NULL, &dram);

    L1.get_block(addr, data)
    

}