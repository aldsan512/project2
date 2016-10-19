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
unsigned page_hash_func(const struct hash_elem* e, void* aux);
bool page_less_func (const struct hash_elem *a, const struct hash_elem* b, void* aux);


#endif //VM_PAGES_H
