#ifndef VM_PAGES_H
#define VM_PAGES_H
#include <stdlib.h>
#include <hash.h>

typedef enum {
	EMPTY, 
	DISK,
	SWAP,
	MEM
} location ;

struct spte {
	location loc;	//need?
	
	 int read_bytes;
     int zero_bytes;
     bool writeable;
     struct file* file;
	
	//bytes read, etc. for load segment
	//swap index???
	
	struct thread* t;
	
	void* vaddr;
	
	struct hash_elem elem;	
};

void spt_init(struct thread* t);
void spt_destroy(struct thread* t);
struct spte* getSPTE(void* vadrr);
void create_new_spte(void* vaddr, location loc, int read_bytes, int zero_bytes, struct file* file, bool writeable );
void load_page(void* vaddr);




#endif //VM_PAGES_H
