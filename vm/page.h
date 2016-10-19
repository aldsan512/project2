#ifndef VM_PAGES_H
#define VM_PAGES_H
#include <stdlib.h>
#include <hash.h>
struct spte {
	bool disk;
	bool swap;
	bool memory; 	//need?
	
	struct thread* t;
	
	void* vaddr;
	
	struct hash_elem elem;	
};


#endif //VM_PAGES_H
