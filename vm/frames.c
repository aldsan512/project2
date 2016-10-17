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
static FrameEntry** frameTable;
static int numFrames;
void initFrame(size_t numF){
	numFrames=numF;
	frameTable=(FrameEntry**)malloc(sizeof(FrameEntry*)*numFrames);
	for(int i=0;i < numFrames;i++){
		frameTable[i]=(FrameEntry*)malloc(sizeof(FrameEntry));
		frameTable[i]->framePT=palloc_get_page(PAL_USER| PAL_ZERO);
		frameTable[i]->isModified=false;
		frameTable[i]->owner=NULL;
	}
	
}
void* getFrame(struct thread* owner){
	for(int i=0;i<numFrames;i++){
		if(frameTable[i]->owner==NULL){
			frameTable[i]->owner=owner;
			return frameTable[i]->framePT;
		}
	}
	return NULL;
}
bool releaseFrame(struct thread* owner){
	for(int i=0;i<numFrames;i++){
		if(frameTable[i]->owner->tid==owner->tid){
			frameTable[i]->owner=NULL;
			memset (frameTable[i]->framePT,0,PGSIZE);
			return true;
		}
	}
	return false;	
}
