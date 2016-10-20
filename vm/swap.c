#include "vm/swap.h"
#include "devices/block.h"
#include "vm/frames.h"
#include "threads/vaddr.h"

//store_to_swap takes address of frame
//should return which swap index it used
//it should copy over the data in the frame to each block (8 blocks per swap index i think) to the swap index
//no eviction algo, just kernel panics if full

//remove_from_swap takes swap index and address of frame
//it should copy over the blocks in the swap index to the frame
swapTE* swapTable;
int numSwapEntries;
struct block* swapArea;
//frame entry has hook to spte, don't need to pass
//should only pass one spte, there is no eviction in swap, if full it kernel panics
void* swapFrame(struct spte* victim,FrameEntry* frameEntry,struct spte* newGuy){
	for(int i=0;i<numSwapEntries; i++){
		//put the victim here
		if(swapTable[i].isOccupied==false){
			//copy data in phys memory onto the swap area. one page==8 swap sectors
			swapTable[i].isOccupied=true;
			block_sector_t sector=i*8;
			char* buffer=(char*)frameEntry->framePT;
			for(int j=0;j<8;j++){
				block_write(swapArea,sector,(void*)buffer);
				buffer=buffer+512;
				sector++;
			}
			//anything else we need to update on evicted frame??
			victim->loc=SWAP;
			memset (frameEntry->framePT,0,PGSIZE);
			frameEntry->pte=newGuy;
			newGuy->loc=MEM;
			return frameEntry->framePT;
		}
			//swap slot taken up
	}
	//kernel panic all slots are taken
}
bool retrieveFromSwap(struct spte* retrieved, void* framePT){
	if(retrieved->loc!=SWAP){
		return false;
	}
	block_sector_t sector=retrieved->swapLoc*8;
	char* buffer=(char*)framePT;
	for(int i=0;i<8;i++){
		block_write(swapArea,sector,buffer);
		buffer=buffer+512;
		sector++;
	}
	swapTable[retrieved->swapLoc].isOccupied=false;
	retrieved->loc=MEM;
	return true;
}
void initSwapTable(void){
	swapArea=block_get_role(BLOCK_SWAP);
	int numSwapEntries=block_size(swapArea)/8;
	swapTable=malloc(sizeof(swapTE)*numSwapEntries);
	for(int i=0;i<numSwapEntries;i++){
		swapTable[i].isOccupied=false;
	}
	//initialize the swap area
	
}
