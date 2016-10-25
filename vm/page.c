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
#include "userprog/pagedir.h"
#include <stdio.h>
int STACK_SIZE = 1<<23; 	//18???

/* Computes and returns the hash value for hash element E, given
   auxiliary data AUX. */
//unsigned page_hash_func (const struct hash_elem *e, void *aux){
	//struct spte* sup_pte = hash_entry(e, struct spte, elem);
	
	//return hash_int((int) sup_pte->vaddr);
//}

/* Compares the value of two hash elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
//bool page_less_func (const struct hash_elem *a,
                             //const struct hash_elem *b,
                             //void *aux){
	//struct spte* sup_pte_a = hash_entry(a, struct spte, elem);
	//struct spte* sup_pte_b = hash_entry(b, struct spte, elem);

	//int a_val = (int) sup_pte_a->vaddr;
	//int b_val = (int) sup_pte_b->vaddr;	
	
	//return a_val < b_val;	 						 
//}

/* Performs some operation on hash element E, given auxiliary
   data AUX. */
//void page_action_func (struct hash_elem *e, void *aux){
	//struct spte* sup_pte = hash_entry(e, struct spte, elem);
	//if(sup_pte->loc == MEM){
		//pagedir_clear_page(thread_current()->pagedir, sup_pte->vaddr);
		//releaseFrame(sup_pte); 
	//}
//}

void spt_init(struct thread* t){
	//hash_init(&t->spt, page_hash_func, page_less_func, NULL);
	list_init(&t->spt);
}

//on thread destructino call this to destroy hash table
void spt_destroy(struct thread* t){
	//hash_destroy(&t->spt, page_action_func);
	struct list_elem* head = list_head(&t->spt);
	struct list_elem* tail = list_tail(&t->spt);
	
	struct list_elem* node = list_next(head);
	while(node != NULL && node != tail){
		struct spte* s_pte = list_entry(node, struct spte, elem);
		if(s_pte != NULL && s_pte->loc == MEM){
			releaseFrame(s_pte);
			pagedir_clear_page(t->pagedir, s_pte->vaddr);
			//list_remove(node);
		}
		node = list_next(node);
	}
}

struct spte* getSPTE(void* vaddr){
	//lookup vaddr in spt hash table
	//round dowm vaddr
	//create fake spte with this vaddr
	//do hash_entry with fake spte
	struct thread* t = thread_current();
	void* vaddress = pg_round_down(vaddr);
	struct list_elem* head = list_head(&t->spt);
	struct list_elem* tail = list_tail(&t->spt);
	
	struct list_elem* node = list_next(head);
	while(node != NULL && node != tail){
		struct spte* s_pte = list_entry(node, struct spte, elem);
		if(s_pte != NULL && s_pte->vaddr == vaddress){
			return s_pte;
		}
		node = list_next(node);
	}
	return NULL;
}

//call in load_segment and setup_stack
//add parameters for every spte member
struct spte* create_new_spte(void* vaddr, location loc, int read_bytes, int zero_bytes, struct file* file, bool writeable , int offset){
	void* vaddress = pg_round_down(vaddr);
	struct spte* existing_spte = getSPTE(vaddress);
	if(existing_spte != NULL){
		//update info???
		//existing_spte->loc = loc;
		//existing_spte->page_read_bytes = read_bytes;
		//existing_spte->page_zero_bytes = zero_bytes;
		//existing_spte->file = file;
		//existing_spte->writeable = writeable;
		return existing_spte;
		
	}
	struct spte* new_spte = (struct spte*) malloc(sizeof(struct spte));
	struct thread* t = thread_current();
	
	new_spte->offset = offset;
	new_spte->vaddr = vaddress;
	new_spte->swapLoc = -1;
	new_spte->t = t;
	new_spte->loc = loc;
	new_spte->page_read_bytes = read_bytes;
	new_spte->page_zero_bytes = zero_bytes;
	new_spte->file = file;
	new_spte->writeable = writeable;
	
	list_push_back(&t->spt, &new_spte->elem);
	return new_spte;
}

//called from page fault handler
bool load_page(void* vaddress, void* esp){
	//round down vaddr first
	if(vaddress == NULL){
		return false;
	}
	void* vaddr = pg_round_down(vaddress); 	//header???
	struct spte* s_pte = getSPTE(vaddr);
	//if(s_pte == NULL){
		//printf("SPTE not found\n");
		//return false;
	//}
	//if(s_pte != NULL && s_pte->loc == MEM){
		//printf("Adress already mapped\n");
		//return true;
	//}
	uint8_t* kpage = getFrame(s_pte);
	//kpage = pg_round_down(kpage);
	//printf("Loading page\n"); 	//remove when done
	if (kpage == NULL){
		//printf("No user frames available.\n");
        return false;
	}
	if(s_pte == NULL  || s_pte->loc == EMPTY){
		//printf("Growing stack\n");
		if(vaddress >= esp - 32 && vaddress >= PHYS_BASE - STACK_SIZE){
			if(s_pte == NULL){
				s_pte = create_new_spte(vaddress, MEM, 0, 0, NULL, true, 0);
			}
			
			memset (kpage, 0, PGSIZE);
			if (!install_page (s_pte->vaddr, kpage, s_pte->writeable)) 
				{
					//printf("Failed to install page - stack\n");
					releaseFrame(s_pte);
					return false;
				}
			s_pte->loc = MEM;
		} else {
			//printf("Out of stack");
			releaseFrame(s_pte);
			return false;
		}		
	}
	//load page
	if(s_pte->loc == DISK){
     //printf("Loading from disk\n");
      /* Load this page. */
      //use same filesys lock from syscall here???
      if (file_read_at(s_pte->file, kpage, s_pte->page_read_bytes, s_pte->offset) != (int) s_pte->page_read_bytes)
        {
		 //printf("Failed to load file\n");
         releaseFrame(s_pte);
         return false; 
        }
      //printf("frame: %#010x\n", kpage);
     // hex_dump(kpage,kpage,s_pte->page_read_bytes,true);
      memset (kpage + s_pte->page_read_bytes, 0, s_pte->page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (s_pte->vaddr, kpage, s_pte->writeable)) 
        {
		  //printf("Failed to install page - file\n");
          releaseFrame(s_pte);
		  return false;
        }
        s_pte->loc = MEM;
        //printf("Loaded from disk\n");
	} else if (s_pte->loc == SWAP){
		//printf("Loading from swap\n");
		if(!install_page(s_pte->vaddr, kpage, s_pte->writeable)){
			//printf("Failed to install page - swap\n");
			releaseFrame(s_pte);
			return false;
		}
		if(retrieveFromSwap(s_pte, kpage)) {
			//printf("Failed to retrieve from swap\n");
			releaseFrame(s_pte);
			return false;
		}
		//printf("Loaded from swap\n");
		
		//memset???
	}
	return true;
}

