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
	int swapLoc;	//AS i think loc and swapLoc are needed idk why we need the 4 data types below
	 int page_read_bytes;
     int page_zero_bytes;
     bool writeable;
     struct file* file;
     int offset;
	
	//bytes read, etc. for load segment
	//swap index???
	
	struct thread* t;
	
	void* vaddr;
	
	struct list_elem elem;	
};

void spt_init(struct thread* t);
void spt_destroy(struct thread* t);
struct spte* getSPTE(void* vadrr);
struct spte* create_new_spte(void* vaddr, location loc, int read_bytes, int zero_bytes, struct file* file, bool writeable, int offset );
bool load_page(void* vaddr, void* esp);




#endif //VM_PAGES_H
