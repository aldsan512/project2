#ifndef VM_SWAP_H
#define VM_SWAP_H
#include "devices/block.h"
typedef struct{
	bool isOccupied;
	struct block_sector_t* swapBlock;
}swapTE;

void* swapFrame(struct spte* victim, struct FrameEntry* frameEntry,struct spte* newGuy);
bool isBlockOccupied();
void initSwapTable();
#endif
