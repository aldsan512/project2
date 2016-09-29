#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/vaddr.h"
#include "threads/pte.h"
#include "userprog/pagedir.h"

#define EOF -1

bool valid_pointer(void* ptr, bool write, struct intr_frame* f){
	struct thread* thread = thread_current();
	if(ptr == NULL){
		f->eax = -1;
		return false;
	} else if(!is_user_vaddr(ptr)){
		f->eax = -1;
		return false;
	} else if (!pagedir_is_present(thread->pagedir, ptr)){
		f->eax = -1;
		return false;
	} else if (!pagedir_is_read_write(thread->pagedir, ptr) && write){
		f->eax = -1;
		return false;
	} else {
		return true;
	}
}

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) {
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/*bool check_page_fault (int fd, struct intr_frame **f) {
	int error = 0;
	//compute error
	*f->error_code = error;
	return error != 0;
}

struct file* get_file(int fd){
	
}*/

//Terminates Pintos by calling shutdown_power_off() (declared in threads/init.h). This should be seldom used, because you lose some information about possible deadlock situations, etc.
void halt (void) {
	shutdown_power_off();
}

//Terminates the current user program, returning status to the kernel. If the process's parent waits for it (see below), this is the status that will be returned. Conventionally, a status of 0 indicates success and nonzero values indicate errors.
void exit (int status) {
	struct thread* t = thread_current();
	t->exit_status=status;
	process_exit();
}

//Runs the executable whose name is given in cmd_line, passing any given arguments, and returns the new process's program id (pid). Must return pid -1, which otherwise should not be a valid pid, if the program cannot load or run for any reason. Thus, the parent process cannot return from the exec until it knows whether the child process successfully loaded its executable. You must use appropriate synchronization to ensure this.
tid_t exec (const char *cmd_line) {
	tid_t tid = process_execute(cmd_line); 
	return tid;
}

/*Waits for a child process pid and retrieves the child's exit status.
If pid is still alive, waits until it terminates. Then, returns the status that pid passed to exit. If pid did not call exit(), but was terminated by the kernel (e.g. killed due to an exception), wait(pid) must return -1. It is perfectly legal for a parent process to wait for child processes that have already terminated by the time the parent calls wait, but the kernel must still allow the parent to retrieve its child's exit status, or learn that the child was terminated by the kernel.

wait must fail and return -1 immediately if any of the following conditions is true:

pid does not refer to a direct child of the calling process. pid is a direct child of the calling process if and only if the calling process received pid as a return value from a successful call to exec.
Note that children are not inherited: if A spawns child B and B spawns child process C, then A cannot wait for C, even if B is dead. A call to wait(C) by process A must fail. Similarly, orphaned processes are not assigned to a new parent if their parent process exits before they do.

The process that calls wait has already called wait on pid. That is, a process may wait for any given child at most once.
Processes may spawn any number of children, wait for them in any order, and may even exit without having waited for some or all of their children. Your design should consider all the ways in which waits can occur. All of a process's resources, including its struct thread, must be freed whether its parent ever waits for it or not, and regardless of whether the child exits before or after its parent.

You must ensure that Pintos does not terminate until the initial process exits. The supplied Pintos code tries to do this by calling process_wait() (in userprog/process.c) from main() (in threads/init.c). We suggest that you implement process_wait() according to the comment at the top of the function and then implement the wait system call in terms of process_wait().

Implementing this system call requires considerably more work than any of the rest.
*/
int wait (tid_t pid) {

	//TODO
	return NULL;
}

//Creates a new file called file initially initial_size bytes in size. Returns true if successful, false otherwise. Creating a new file does not open it: opening the new file is a separate operation which would require a open system call.
bool create (const char *file, unsigned initial_size) {
	if(file==NULL){return false;}	
	if(strlen(file)>14){return false;} 
	return filesys_create(file,initial_size);
}

//Deletes the file called file. Returns true if successful, false otherwise. A file may be removed regardless of whether it is open or closed, and removing an open file does not close it. See Removing an Open File, for details.
bool remove (const char *file) {
	if(file==NULL){return false;}
	if(strlen(file)>14){return false;}
	return 	filesys_remove(file);
}

/*Opens the file called file. Returns a nonnegative integer handle called a "file descriptor" (fd), or -1 if the file could not be opened.
File descriptors numbered 0 and 1 are reserved for the console: fd 0 (STDIN_FILENO) is standard input, fd 1 (STDOUT_FILENO) is standard output. The open system call will never return either of these file descriptors, which are valid as system call arguments only as explicitly described below.

Each process has an independent set of file descriptors. File descriptors are not inherited by child processes.

When a single file is opened more than once, whether by a single process or different processes, each open returns a new file descriptor. Different file descriptors for a single file are closed independently in separate calls to close and they do not share a file position.
*/
int open (const char *file) {
	if(strlen(file)>14){return -1;}
	struct file* filePt = filesys_open(file);
	if(filePt!=NULL){
		struct thread* thread = thread_current();
		int i;	//stderr also reserved
		for(i = 2; i < thread->fileTableSz; i++){
			if(thread->fileTable[i] == NULL){
				thread->fileTable[i]=filePt;
				return i;
			}
		}
	}
	return -1;	
}

//Returns the size, in bytes, of the file open as fd.
int filesize (int fd) {
	if(fd<=1){return -1;}
	struct thread* thread= thread_current();
	struct file* file=thread->fileTable[fd];
	if(file==NULL){return -1;}
	return file_length(file);
}

//Reads size bytes from the file open as fd into buffer. Returns the number of bytes actually read (0 at end of file), or -1 if the file could not be read (due to a condition other than end of file). Fd 0 reads from the keyboard using input_getc().
int read (int fd, void *buffer, unsigned size) {
	int bytes = 0;
	char* read_buffer = (char*) buffer;
	if (fd == 1 || fd < 0){
		bytes = -1;
	} else if (fd == 0){
		int i;
		for(i = 0; i < size; i++) {
			char byte = input_getc();
			if(byte == EOF){
				read_buffer[i] = NULL;
				return i;
			}
			read_buffer[i] = byte;
		}
		read_buffer[size] = NULL; //in case ran out of room before EOF
		bytes = size;
	} else{
		struct thread* thread= thread_current();	
		struct file* file=thread->fileTable[fd];
		if(file==NULL){return -1;}
		bytes=(int)file_read(file, buffer,size);
	}
	return bytes;
}

/*Writes size bytes from buffer to the open file fd. Returns the number of bytes actually written, which may be less than size if some bytes could not be written.
Writing past end-of-file would normally extend the file, but file growth is not implemented by the basic file system. The expected behavior is to write as many bytes as possible up to end-of-file and return the actual number written, or 0 if no bytes could be written at all.

Fd 1 writes to the console. Your code to write to the console should write all of buffer in one call to putbuf(), at least as long as size is not bigger than a few hundred bytes. (It is reasonable to break up larger buffers.) Otherwise, lines of text output by different processes may end up interleaved on the console, confusing both human readers and our grading scripts.
*/
int write (int fd, const void *buffer, unsigned size) {
	int bytes = 0;
	if (fd <= 0) {
		bytes = -1;
	} else if (fd == 1) {
		//putbut in chunks
		putbuf (buffer, size);
		bytes = size;	
	} else {
		struct thread* thread= thread_current();
		struct file* file= thread->fileTable[fd];
		if(file==NULL){return -1;}
		bytes=file_write(file,buffer,size);
	}
	return bytes;
}

/*Changes the next byte to be read or written in open file fd to position, expressed in bytes from the beginning of the file. (Thus, a position of 0 is the file's start.)
A seek past the current end of a file is not an error. A later read obtains 0 bytes, indicating end of file. A later write extends the file, filling any unwritten gap with zeros. (However, in Pintos files have a fixed length until project 4 is complete, so writes past end of file will return an error.) These semantics are implemented in the file system and do not require any special effort in system call implementation.
*/
void seek (int fd, unsigned position) {
	struct thread* thread= thread_current();
	struct file* file=thread->fileTable[fd];
	if(file!=NULL){
		file_seek(file, (off_t) position);
	}
}

//Returns the position of the next byte to be read or written in open file fd, expressed in bytes from the beginning of the file.
unsigned tell (int fd) {
	struct thread* thread= thread_current();
	struct file* file=thread->fileTable[fd];
	if(file!=NULL){
	return file_tell(file);	
	}
	return 0;
}

//Closes file descriptor fd. Exiting or terminating a process implicitly closes all its open file descriptors, as if by calling this function for each one.
void close (int fd) {
	if(fd <= 1){ return;}
	struct thread* thread= thread_current();
	struct file* file = thread->fileTable[fd];
	if(file==NULL){return;}
	thread->fileTable[fd]=NULL;
	file_close(file); 
}

static void
syscall_handler (struct intr_frame *f) {
	int status, fd;
	unsigned size, position;
	char *file, *command;
	void *buffer;
	tid_t pid;

	uint32_t* sp = f->esp;
	bool failure = false;
	if(!valid_pointer(sp, false, -1)){ return; }
	if(!valid_pointer(f->eax, false, -1)){ return; }	//needed???
	uint32_t sys_call = *sp; 	
	sp++;
	switch (sys_call) {
		case SYS_HALT:                   /* Halt the operating system. */
			halt ();
			break;
		case SYS_EXIT:                   /* Terminate this process. */
			status = *sp;
			exit (status);
			break;
		case SYS_EXEC:                   /* Start another process. */
			command = (char*) *sp;
			if(!valid_pointer(command, false, f)){ return; }
			f->eax = (uint32_t) exec (command);
			break;
		case SYS_WAIT:                   /* Wait for a child process to die. */
			pid = *sp;
			f->eax = (uint32_t) wait (pid);
			break;
		case SYS_CREATE:                /* Create a file. */
			file = (char*) *sp;
			if(!valid_pointer(file, false, f)){ return; }
			sp++;
			size = *sp;
			f->eax = (uint32_t) create(file, size);
			break;
		case SYS_REMOVE:             /* Delete a file. */
			file = (char*) *sp;
			if(!valid_pointer(file, false, f)){ return; }
			f->eax = (uint32_t) remove (file);
			break;
		case SYS_OPEN:               /* Open a file. */
			file = (char*) *sp;
			if(!valid_pointer(sp, false, f)){ return; }
			open (file);
			break;
		case SYS_FILESIZE:          /* Obtain a file's size. */
			fd = *sp;
			f->eax = (uint32_t) filesize (fd);
			break;
		case SYS_READ:            /* Read from a file. */
			fd = *sp;
			sp++;
			buffer = (char*) *sp;
			if(!valid_pointer(buffer, false, f)){ return; }
			sp++;
			size = *sp;
			//check if returns -1 and terminate process if so
			f->eax = (uint32_t) read (fd, buffer, size);
			break;
		case SYS_WRITE:             /* Write to a file. */
			fd = *sp;
			sp++;
			buffer = (char*) *sp;
			if(!valid_pointer(buffer, true, f)){ return; }
			sp++;
			size = *sp;
			//check if returns -1 and terminate process if so
			f->eax = (uint32_t) write (fd, buffer, size);
			break;
		case SYS_SEEK:              /* Change position in a file. */
			fd = *sp;
			sp++;
			position = *sp;
			seek (fd, position);
			break;
		case SYS_TELL:               /* Report current position in a file. */
			fd = *sp;
			f->eax = (uint32_t) tell (fd);
			break;
		case SYS_CLOSE:              /* Close a file. */
			fd = *sp;
			close (fd);
			break;
		default:
			f->eax = -1;
			break;
	}
	
}
