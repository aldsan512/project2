#ifndef VM_FRAMES_H
#define VM_FRAMES_H
#include <stdlib.h>
#include <stdio.h>

typedef struct{
	void* framePT;
	struct thread* owner;
	bool isModified;

}FrameEntry;
void initFrame(size_t numFrames);
void* getFrame(struct thread* owner);
bool releaseFrame(struct thread* owner);
#endif /* vm/frames.h */

