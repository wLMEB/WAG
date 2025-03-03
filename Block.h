#include <iostream>
#include <typeinfo>
#include <cmath>

class Block
{
public:
    int data[8];     // 32 byte (8 word) block size
    bool referenced; // Used by clock operation
    bool dirty;      // Tells if the data has been edited, and must then be written
    int addr;        // address of first byte in the block (must be a multiple of 32)
    bool in_use;     // is the block in use

    Block();
};