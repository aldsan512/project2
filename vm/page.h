#ifndef VM_PAGES_H
#define VM_PAGES_H


struct spte {
	boolean disk;
	boolean swap;
	boolean memory; 	//need?
	
	//bytes read, etc. for load segment
	//swap index???
	
	struct thread* t;
	
	void* vaddr;
	
	struct hash_elem elem;	
};


#endif //VM_PAGES_H
