#ifndef VM_FRAMES_H
#define VM_FRAMES_H
#include <stdlib.h>
#include <stdio.h>
#include "vm/page.h"

typedef struct{
	void* framePT;
	struct spte* pte;
	//bool isModified;

}FrameEntry;
void initFrame(size_t numFrames);
void* getFrame(struct spte* owner);
bool releaseFrame(struct spte* owner);
void* evictFrame(struct spte* owner);
#endif /* vm/frames.h */

