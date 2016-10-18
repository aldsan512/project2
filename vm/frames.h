#ifndef VM_FRAMES_H
#define VM_FRAMES_H
#include <stdlib.h>
#include <stdio.h>
#include "vm/page.h"

typedef struct{
	void* framePT;
	struct thread* owner;
	//struct spte* sup_pte;
	bool isModified;

}FrameEntry;
void initFrame(size_t numFrames);
void* getFrame(struct thread* owner);
bool releaseFrame(struct thread* owner);
#endif /* vm/frames.h */

