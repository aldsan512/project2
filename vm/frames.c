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
static FrameEntry** frameTable;
static int numFrames;
//static struct lock* myLock;
void initFrame(size_t numF){	
	//lock_init(myLock);								
	numFrames=numF-1;
	frameTable=(FrameEntry**)malloc(sizeof(FrameEntry*)*numFrames);
	int i = 0;
	void* framePT = NULL;
	while(framePT = palloc_get_page(PAL_USER| PAL_ZERO) != NULL){
		frameTable[i]=(FrameEntry*)malloc(sizeof(FrameEntry));
		//frameTable[i]->framePT=palloc_get_page(PAL_USER| PAL_ZERO);
		frameTable[i]->isModified=false;
		frameTable[i]->pte=NULL;
		i++;
	}
	
}
void* getFrame(struct spte* owner){
	//lock_acquire(myLock);
	for(int i=0;i<numFrames;i++){
		if(frameTable[i]->pte==NULL){
			frameTable[i]->pte=owner;
			owner->loc=MEM;
			//lock_release(myLock);
			return frameTable[i]->framePT;
		}
	}
	//if above fails, frame evict and return the replaced frame
	void* result=evictFrame(owner);
	//lock_release(myLock);
	return result;
}
bool releaseFrame(struct spte* owner){
	//lock_acquire(myLock);
	for(int i=0;i<numFrames;i++){
		if(frameTable[i]->pte->vaddr==owner->vaddr){
			frameTable[i]->pte=NULL;
			memset (frameTable[i]->framePT,0,PGSIZE);
			//if install page was called do clear page
			//lock_release(myLock);
			return true;
		}
	}
	//lock_release(myLock);
	return false;	
}
void* evictFrame(struct spte* owner){
	for(int i=0;i<=numFrames;i++){
		if(i==numFrames){
			//roll over//
			i=0;
			if(pagedir_is_dirty(frameTable[i]->pte->t->pagedir,frameTable[i]->pte->vaddr)){
				swapFrame(frameTable[i]->pte, frameTable[i],owner);	
			}
			else{
				frameTable[i]->pte=owner;
				return frameTable[i]->framePT;
			}
		}
		if(pagedir_is_accessed(frameTable[i]->pte->t->pagedir,frameTable[i]->pte->vaddr)){
			//if it has been accessed set accessed to 0
			pagedir_set_accessed(frameTable[i]->pte->t->pagedir,frameTable[i]->pte->vaddr,0);
		}
		//page has been accessed
		else if( pagedir_is_dirty(frameTable[i]->pte->t->pagedir,frameTable[i]->pte->vaddr)){
				//page is dirty need to put it in swap table
				swapFrame(frameTable[i]->pte, frameTable[i],owner);					
		}
		else{
				//frame is code just evict and read from disk later	
                //page_dir_clear_page on old page that owned it 
                //install_page
				frameTable[i]->pte=owner;
				memset (frameTable[i]->framePT,0,PGSIZE);
				return frameTable[i]->framePT;
		}
 

	}
}
















