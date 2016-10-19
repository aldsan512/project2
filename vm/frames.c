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
static FrameEntry** frameTable;
static int numFrames;
void initFrame(size_t numF){
	numFrames=numF-1;
	frameTable=(FrameEntry**)malloc(sizeof(FrameEntry*)*numFrames);
	for(int i=0;i < numFrames;i++){
		frameTable[i]=(FrameEntry*)malloc(sizeof(FrameEntry));
		frameTable[i]->framePT=palloc_get_page(PAL_USER| PAL_ZERO);
		frameTable[i]->isModified=false;
		frameTable[i]->pte=NULL;
	}
	
}
void* getFrame(struct spte* owner){
	for(int i=0;i<numFrames;i++){
		if(frameTable[i]->pte==NULL){
			frameTable[i]->pte=owner;
			return frameTable[i]->framePT;
		}
	}
	//if above fails, frame evict and return the replaced frame
	return NULL;
}
//should be void* address, multiple frames per owner
bool releaseFrame(struct spte* owner){
	for(int i=0;i<numFrames;i++){
		if(frameTable[i]->pte->vaddr==owner->vaddr){
			frameTable[i]->pte=NULL;
			memset (frameTable[i]->framePT,0,PGSIZE);
			return true;
		}
	}
	return false;	
}
void* evictFrame(){
	for(int i=0;i<=numFrames;i++){
		pagedir_is_accessed(frameTable[i]->pte->t->pagedir,frameTable[i]->pte->vaddr);

	}
}

