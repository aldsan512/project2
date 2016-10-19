#include "swap.h"

//store_to_swap takes address of frame
//should return which swap index it used
//it should copy over the data in the frame to each block (8 blocks per swap index i think) to the swap index
//no eviction algo, just kernel panics if full

//remove_from_swap takes swap index and address of frame
//it should copy over the blocks in the swap index to the frame

//we'll have to use the device/blocks header to do the block stuff
