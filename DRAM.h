#include "Block.h"

class DRAM
{
public:
    int response_time; // number of cycles required for access
    Block *block_list;
    int current_cycles; // how long has this request been serviced.
    int current_id;     // the id of the pipeline stage being serviced, 0 for no id
    int size;   
    DRAM(int _size, int _response_time); 

    bool time_to_service(int request_id);
    // increments the cycle_count if the cache is working on a request
    bool increment_cycle_count();

    bool get_block(int addr, Block *upper_level_block, int request_id);

    bool write_block(int addr, Block *upper_level_block, int request_id);
};