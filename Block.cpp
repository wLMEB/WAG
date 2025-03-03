#include "Block.h"

class Block
{
public:
    int data[8];         // 32 byte (8 word) block size
    bool referenced = 0; // Used by clock operation
    bool dirty = 0;      // Tells if the data has been edited, and must then be written
    int addr = 0;        // address of first byte in the block (must be a multiple of 32)
    bool in_use = 0;         // is the block in use

    Block()
    {
        for (int i = 0; i < 8; i++)
        {
            data[i] = 0;
        }
    };
};