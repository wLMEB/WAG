#include "DRAM.h"


class DRAM
{
public:
    int response_time; // number of cycles required for access
    Block *block_list;
    int current_cycles = 1; // how long has this request been serviced.
    int current_id = 0;     // the id of the pipeline stage being serviced, 0 for no id
    int size = 0;   
    DRAM(int _size, int _response_time)
    {
        size = _size;
        response_time = _response_time;
        block_list = new Block[_size];
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
        current_cycles++;
    }
    bool get_block(int addr, Block *upper_level_block, int request_id) // must be a multiple of 32
    {
        if (!time_to_service(request_id))
        { // check if reached response time or not
            return 0;
        }
        Block dram_block = block_list[(addr / 32)]; // perform task only response time has passed
        for (int i = 0; i < 8; i++)
        {
            upper_level_block->data[i] = dram_block.data[i];
        }
        current_cycles = 0; // accepting new services
        current_id = 0;
        return 1;
    }
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
};