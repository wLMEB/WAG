#include "Block.h"



class Cache
{ // word is 32 bit = size of an integer 4byte
public:
    int response_time;         // number of cycles required for access
    int size;                  // size of the cache in blocks
    int current_cycles;    // number of cycles I have been servicing memory request
    Cache *upper_cache; // cache above me (NULL if L1)
    Cache *lower_cache; // cache below me (NULL if lowest level cache)
    DRAM *dram;         // DRAM (NULL If not lowest level cache)
    Block *block_list;         // blocks within me
    int associativity;         // the associativity of the cache
    int current_id;        // the id of the pipeline stage being serviced, 0 for no id
    bool requesting;       // is the cache making a request of a lower level cache/DRAM

    Cache(int _size, int _response_time, int _associativity); // _size as in how many blocks

    bool set_other_caches(Cache* _upper_cache, Cache* _lower_cache, DRAM* _dram);
    
    bool time_to_service(int request_id); // check if it is time to perform the request
    
    bool increment_cycle_count(); // increments the cycle_count if the cache is working on a request

    bool evict_block(int addr); 

    bool get_block(int addr, int *block_data[8], int id);

    bool get_word(int addr, int *word, int id);
};