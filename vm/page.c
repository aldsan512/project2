#include "vm/page.h"
#include "vm/frames.h"
#include "threads/thread.h"
#include <hash.h>

/* Computes and returns the hash value for hash element E, given
   auxiliary data AUX. */
unsigned page_hash_func (const struct hash_elem *e, void *aux){
	struct spte* sup_pte = hash_entry(e, struct spte, elem);
	
	return hash_int((int) sup_pte->vaddr);
}

/* Compares the value of two hash elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
bool page_less_func (const struct hash_elem *a,
                             const struct hash_elem *b,
                             void *aux){
	struct spte* sup_pte_a = hash_entry(a, struct spte, elem);
	struct spte* sup_pte_b = hash_entry(b, struct spte, elem);

	int a = (int) sup_pte_a->vaddr;
	int b = (int) sup_pte_b->vaddr;	
	
	return a < b;	 						 
}

/* Performs some operation on hash element E, given auxiliary
   data AUX. */
void page_action_func (struct hash_elem *e, void *aux){
	struct thread* t = current_thread();
	releaseFrame(t); 	//wrong
}

void spt_init(struct thread* t){
	hash_init(&t->hash, page_hash_func, page_less_func, NULL);
}

//on thread destructino call this to destroy hash table
void spt_destroy(struct thread* t){
	hash_destroy(&t->hash, page_action_func);
}

struct spte* getSPTE(void* vadrr){
	
	
	
}

//add file to page table method???

//called from page fault handler
void load_page(void* vaddr){
	struct* spte = getSPTE(vaddr);
	//load page
	if(spte->disk){
		//load page from disk
		//get frame
	} else if (spte->swap){
		//load page from swap
	} else {
		
	}
}

