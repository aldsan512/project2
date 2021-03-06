#include "vm/swap.h"
#include "devices/block.h"
#include "vm/frames.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include <string.h>
#include "threads/synch.h"
#include "threads/thread.h"
//store_to_swap takes address of frame
//should return which swap index it used
//it should copy over the data in the frame to each block (8 blocks per swap index i think) to the swap index
//no eviction algo, just kernel panics if full

//remove_from_swap takes swap index and address of frame
//it should copy over the blocks in the swap index to the frame
static swapTE* swapTable;
static int numSwapEntries;
static struct block* swapArea;
static struct lock lock;

//frame entry has hook to spte, don't need to pass
//should only pass one spte, there is no eviction in swap, if full it kernel panics
void* swapFrame(struct spte* victim,FrameEntry* frameEntry,struct spte* newGuy){
	lock_acquire(&lock);
	for(int i=0;i<numSwapEntries; i++){
		//put the victim here
		if(swapTable[i].isOccupied==false){
			//copy data in phys memory onto the swap area. one page=8 swap sectors
			swapTable[i].isOccupied=true;
			block_sector_t sector=i*SECTORS_PER_PAGE;
			char* buffer=(char*)frameEntry->framePT;
			for(int j=0;j<SECTORS_PER_PAGE;j++){
				block_write(swapArea,sector,(void*)buffer);
				buffer=buffer+BLOCK_SECTOR_SIZE;
				sector++;
			}
			//anything else we need to update on evicted frame??
			victim->loc=SWAP;
			victim->swapLoc = i;
			pagedir_clear_page(victim->t->pagedir, victim->vaddr);
			memset (frameEntry->framePT,0,PGSIZE);
			frameEntry->pte=newGuy;
			newGuy->loc=MEM;
			void* result = frameEntry->framePT;
			lock_release(&lock);
			return result;
		}
	}
	lock_release(&lock);
	PANIC ("Frame could not be evicted because swap is full!");
}
bool retrieveFromSwap(struct spte* retrieved, void* framePT){
	lock_acquire(&lock);
	if(retrieved->loc!=SWAP){
		lock_release(&lock);
		return false;
	}
	block_sector_t sector=retrieved->swapLoc*SECTORS_PER_PAGE;
	char* buffer=(char*)framePT;
	for(int i=0;i<SECTORS_PER_PAGE;i++){
		block_read(swapArea,sector,buffer);
		buffer=buffer+BLOCK_SECTOR_SIZE;
		sector++;
	}
	swapTable[retrieved->swapLoc].isOccupied=false;
	retrieved->loc=MEM;
	lock_release(&lock);
	return true;
}
void initSwapTable(void){
	lock_init(&lock);
	swapArea=block_get_role(BLOCK_SWAP);
	if(!swapArea){
		return; 	//failure
	}
	numSwapEntries=block_size(swapArea)/ SECTORS_PER_PAGE;
	//problem above ???
	swapTable=(swapTE*) malloc(sizeof(swapTE)*numSwapEntries);
	for(int i=0;i<numSwapEntries;i++){
		swapTable[i].isOccupied=false;
	}
	//initialize the swap area//
	
}
