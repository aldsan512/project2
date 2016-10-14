#include "frames.h"
#include "threads/palloc.h"
#include <stdlib.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <string.h>

static FrameEntry** frameTable;
void initFrame(size_t numFrames){
	frameTable=(FrameEntry**)malloc(sizeof(FrameEntry*)*numFrames);
	for(int i=0;i < numFrames;i++){
		frameTable[i]=(FrameEntry*)malloc(sizeof(FrameEntry));
		frameTable[i]->framePT=palloc_get_page(PAL_USER);
		frameTable[i]->isModified=false;
		frameTable[i]->owner=NULL;
	}
	
}
void* getFrame(struct Thread* owner){
	for(int i=0;i<numFrames;i++){
		if(frameTable[i]->owner==NULL){
			frameTable[i]->owner=owner;
			return frameTable[i]->framePt;
		}
	}
}
bool releaseFrame(struct Thread* owner){
	for(int i=0;i<numFrames;i++){
		if(frameTable[i]->owner->tid==owner->tid){
			frameTable[i]->owner=NULL;
			return true;
		}
	}
	return false;	
}
