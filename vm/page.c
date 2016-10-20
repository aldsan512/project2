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
	struct spte* sup_pte = hash_entry(e, struct spte, elem);
	releaseFrame(sup_pte); 
}

void spt_init(struct thread* t){
	hash_init(&t->hash, page_hash_func, page_less_func, NULL);
}

//on thread destructino call this to destroy hash table
void spt_destroy(struct thread* t){
	hash_destroy(&t->hash, page_action_func);
}

struct spte* getSPTE(void* vadrr){
	//lookup vaddr in spt hash table
	
	
}

//call in load_segment and setup_stack
//add parameters for every spte member
void create_new_spte(void* vaddr, location loc, int read_bytes, int zero_bytes, struct file* file, bool writeable ){
	struct spte* new_spte = (sttruct spte*) malloc(sizeof(struct spte));
	struct thread* t = current_thread();
	new_spte->thread = t;
	new_spte->loc = loc;
	new_spte->read_bytes = read_bytes;
	new_spte->zero_bytes = zero_bytes;
	new_spte->file = file;
	new_spte->writeable = writeable;
	hash_insert(&t->spt, &new_spte->elem);
}

//called from page fault handler
bool load_page(void* vaddr){
	//round down vaddr first
	struct spte* s_pte = getSPTE(vaddr);
	void* kpage = getFrame(s_pte);
	if(s_pte == NULL){
		//check if in stack space
	}
	//load page
	
	if(spte->loc == DISK){
      if (kpage == NULL)
        return false;

      /* Load this page. */
      if (file_read (s_pte->file, kpage, s_pte->page_read_bytes) != (int) s_pte->page_read_bytes)
        {
         releaseFrame(s_pte);
         return false; 
        }
      memset (kpage + s_pte->page_read_bytes, 0, s_pte->page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (s_pte->vaddr, kpage, s_pte->writable)) 
        {
          releaseFrame(s_pte);
		  return false
        }*/
		//load page from disk to frame
	} else if (spte->loc == SWAP){
		
		//load page from swap to frame
	} else if (spte->loc == STACK) {
		   
	}
}

