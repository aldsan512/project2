#include "vm/page.h"
#include "vm/frames.h"
#include "threads/thread.h"
#include <hash.h>
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "vm/swap.h"
#include <string.h>
#include "threads/malloc.h"
#include "filesys/file.h"

int STACK_SIZE = 1<<23;

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

	int a_val = (int) sup_pte_a->vaddr;
	int b_val = (int) sup_pte_b->vaddr;	
	
	return a_val < b_val;	 						 
}

/* Performs some operation on hash element E, given auxiliary
   data AUX. */
void page_action_func (struct hash_elem *e, void *aux){
	struct spte* sup_pte = hash_entry(e, struct spte, elem);
	releaseFrame(sup_pte); 
}

void spt_init(struct thread* t){
	hash_init(&t->spt, page_hash_func, page_less_func, NULL);
}

//on thread destructino call this to destroy hash table
void spt_destroy(struct thread* t){
	hash_destroy(&t->spt, page_action_func);
}

struct spte* getSPTE(void* vaddr){
	//lookup vaddr in spt hash table
	//round dowm vaddr
	//create fake spte with this vaddr
	//do hash_entry with fake spte
	struct thread* t = thread_current();
	void* vaddress = pg_round_down(vaddr);
	struct spte* temp = (struct spte*) malloc(sizeof(struct spte));
	temp->vaddr = vaddress;
	struct hash_elem* e = hash_find(&t->spt, &temp->elem);
	return hash_entry(e, struct spte, elem);
}

//call in load_segment and setup_stack
//add parameters for every spte member
struct spte* create_new_spte(void* vaddr, location loc, int read_bytes, int zero_bytes, struct file* file, bool writeable ){
	//round vaddr down first???
	struct spte* new_spte = (struct spte*) malloc(sizeof(struct spte));
	struct thread* t = thread_current();
	new_spte->t = t;
	new_spte->loc = loc;
	new_spte->page_read_bytes = read_bytes;
	new_spte->page_zero_bytes = zero_bytes;
	new_spte->file = file;
	new_spte->writeable = writeable;
	hash_insert(&t->spt, &new_spte->elem);
	return new_spte;
}

//called from page fault handler
bool load_page(void* vaddress, void* esp){
	//round down vaddr first
	void* vaddr = pg_round_down(vaddress); 	//header???
	struct spte* s_pte = getSPTE(vaddr);
	void* kpage = getFrame(s_pte);
	printf("Loading page\n");
	if (kpage == NULL){
        return false;
	}
	if(s_pte == NULL  || s_pte->loc == EMPTY){
		if(vaddress >= esp + 32 && vaddress <= PHYS_BASE + STACK_SIZE){
			if(s_pte == NULL){
				s_pte = create_new_spte(vaddress, MEM, 0, 0, NULL, true);
			}
			memset (kpage, 0, PGSIZE);
			if (!install_page (s_pte->vaddr, kpage, s_pte->writeable)) 
				{
					releaseFrame(s_pte);
					return false;
				}
			s_pte->loc = MEM;
		} else {
			releaseFrame(s_pte);
			return false;
		}
		//check if vaddress in stack space
		//if so install page install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true) except with vaddr
		//else return false and release frame
		
	}
	//load page
	if(s_pte->loc == DISK){
     
      /* Load this page. */
      if (file_read (s_pte->file, kpage, s_pte->page_read_bytes) != (int) s_pte->page_read_bytes)
        {
         releaseFrame(s_pte);
         return false; 
        }
      memset (kpage + s_pte->page_read_bytes, 0, s_pte->page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (s_pte->vaddr, kpage, s_pte->writeable)) 
        {
          releaseFrame(s_pte);
		  return false;
        }
	} else if (s_pte->loc == SWAP){
		if(retrieveFromSwap(s_pte, kpage)) {
			releaseFrame(s_pte);
			return false;
		}
		if(!install_page(s_pte->vaddr, kpage, s_pte->writeable)){
			releaseFrame(s_pte);
			return false;
		}
		//memset???
		//install page???
	}
}

