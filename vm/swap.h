#ifndef VM_SWAP_H
#define VM_SWAP_H
#include "devices/block.h"
#include "vm/page.h"
#include "vm/frames.h"

#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

typedef struct{
	bool isOccupied;
	struct block_sector_t* swapBlock;
}swapTE;

void* swapFrame(struct spte* victim, FrameEntry* frameEntry,struct spte* newGuy);
void initSwapTable(void);
bool retrieveFromSwap(struct spte* retrieved, void* framePT);
#endif
