#include "frames.h"
#include "threads/palloc.h"
#include <stdlib.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <string.h>
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "vm/swap.h"
#include "threads/synch.h"
static FrameEntry* frameTable;
static int numFrames;
static struct lock myLock;
int clock=0;
void initFrame(size_t numF){	
	lock_init(&myLock);								
	numFrames=numF-1; 	//put back to -1 later
	frameTable=(FrameEntry*)malloc(sizeof(FrameEntry)*numFrames);
	for(int i = 0; i < numFrames; i++){
		frameTable[i].framePT=palloc_get_page(PAL_USER | PAL_ZERO);
		frameTable[i].isModified=false;
		frameTable[i].pte=NULL;
	}
	
}
void* getFrame(struct spte* owner){
	lock_acquire(&myLock);
	for(int i=0;i<numFrames;i++){
		if(frameTable[i].pte==NULL){
			frameTable[i].pte=owner;
			//owner->loc=MEM;
			lock_release(&myLock);
			return frameTable[i].framePT;
		}
	}
	//if above fails, frame evict and return the replaced frame
	void* result=evictFrame(owner);
	lock_release(&myLock);
	return result;
}
bool releaseFrame(struct spte* owner){
	lock_acquire(&myLock);
	for(int i=0;i<numFrames;i++){
		if(frameTable[i].pte==owner){
			frameTable[i].pte=NULL;
			memset (frameTable[i].framePT,0,PGSIZE);
			//if install page was called do clear page
			lock_release(&myLock);
			return true;
		}
	}
	lock_release(&myLock);
	return false;	
}
void* evictFrame(struct spte* owner){
	//should we be setting the actual pte accessed bits or setting a boolean in the fte???
	//i=clock;
	//numFrames
//	int start = clock;
//	bool around = false;
//	int i = clock;
	while(true){
	/*	if(i == start && around){
			//roll over//
			if(frameTable[clock].pte->pinned){
				i = (i + 1) % numFrames;
				continue;
			}
			if(pagedir_is_dirty(frameTable[i].pte->t->pagedir,frameTable[i].pte->vaddr)){
				clock = i;
				return swapFrame(frameTable[i].pte, &frameTable[i],owner);	

			}
			else{
				frameTable[i].pte->loc = DISK;
				pagedir_clear_page(frameTable[i].pte->t->pagedir, frameTable[i].pte->vaddr);
				frameTable[i].pte=owner;
				memset (frameTable[i].framePT,0,PGSIZE);
				clock = i;
				return frameTable[i].framePT;
			}
		}*/
	//	around = true;
		//if(frameTable[clock].pte->pinned){
		//	clock = (clock + 1) % numFrames;
		//	continue;
		//}
		if(pagedir_is_accessed(frameTable[clock].pte->t->pagedir,frameTable[clock].pte->vaddr)){
			//if it has been accessed set accessed to 0
			pagedir_set_accessed(frameTable[clock].pte->t->pagedir,frameTable[clock].pte->vaddr,0);
		}
		//page has not been accessed
		else if( pagedir_is_dirty(frameTable[clock].pte->t->pagedir,frameTable[clock].pte->vaddr)|| frameTable[clock].pte->loc==SWAP){
				//page is dirty need to put it in swap table
				//clock = i;
				return swapFrame(frameTable[clock].pte, &frameTable[clock],owner);					
		} else {
				//frame is code just evict and read from disk later	
                //page_dir_clear_page on old page that owned it 
                //install_page
                frameTable[clock].pte->loc = DISK;
				pagedir_clear_page(frameTable[clock].pte->t->pagedir, frameTable[clock].pte->vaddr);
				frameTable[clock].pte=owner;
				memset (frameTable[clock].framePT,0,PGSIZE);
				//clock = i;
				return frameTable[clock].framePT;
		}
 
		 clock= (clock + 1) % numFrames;
	}
}
















