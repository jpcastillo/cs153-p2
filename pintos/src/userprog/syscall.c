#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  unsigned int _fd;
  void * readbuffer;
  void * writebuffer;
  unsigned int _size;
  
  unsigned int * x = f-> esp;
  //printf ("system call!\n");
  if( *x == SYS_HALT)  {
		printf("HALT");
  }
  else if(*x ==  SYS_EXIT) {
 		x += sizeof(int);
		S__EXIT(*x);
		//printf("EXIT");
  }
   else if(*x ==  SYS_WAIT) {
		printf("WAIT");
  }
 else if(*x ==  SYS_CREATE) {
		printf("CREATE");
  }
 else if(*x ==  SYS_REMOVE) {
		printf("REMOVE");
  }
 else if(*x ==  SYS_OPEN) {
		printf("OPEN");
  }
 else if(*x ==  SYS_FILESIZE) {
		printf("FILESIZE");
  }
 else if(*x ==  SYS_READ) {
		printf("READ");
		x += sizeof(int);
		_fd = *x;
		x += 1;
		readbuffer = x;
		x += sizeof(unsigned);
		_size = *x; 
		f -> eax = S__READ(_fd, readbuffer, _size);
  }
 else if(*x ==  SYS_WRITE) {
		//printf("WRITE\n");
		x += 1;
		_fd = *x;
		//printf("%X\n", x);
		x += 1;
		writebuffer = *x;
		//printf("%X\n", x);
		x += 1;
		//printf("%u\n", *x);
		_size = *x;
		f -> eax = S__WRITE(_fd, writebuffer, _size); 
  }
   else if(*x ==  SYS_SEEK) {
		printf("SEEK");
  }
   else if(*x ==  SYS_TELL) {
		printf("TELL");
  }
   else if(*x ==  SYS_CLOSE) {
		printf("CLOSE");
  }
  else {
	printf("ERROR");
  }

  //asm volatile ("call %1; ret;" :"=r" (&f->eax) : "g" (&f->esp));
  
  //thread_exit ();
}


int S__READ( int fd, void * buffer, unsigned size)
{
	ASSERT ( buffer != NULL && buffer > PHYS_BASE );
	//while
	return -1;


}

int S__WRITE(unsigned int fd, const void * buffer, unsigned int size)
{
	//ASSERT ( buffer != NULL && buffer > PHYS_BASE);
	//printf("WRITE");
	if (fd != 1)
	{
		//printf("%d", fd);
		return 0; //ERROR
	}
	
	putbuf(buffer, size);
	return size;

}

void S__EXIT(int status)
{
	struct thread *cur = thread_current();
	struct thread *entry = NULL;
	struct list_elem *e = NULL;
	for(e = list_begin(&cur -> children); e != list_end(&cur -> children); e = list_next(e))
   	{
		entry = list_entry(e, struct thread, children_elem);
		if(entry != NULL && entry -> parent == cur)
	  	{
			entry -> parent = NULL;
			entry -> status = status;
		}
 	}
   	cur->exit = true;
        //if(!cur -> argv)
	printf ("%s: exit(%d)\n", cur -> argv, status);
	//else
	//{
	//	printf ("%s", cur->name);
	//	int i;
	//	for(i = 1; i < cur -> argc - 1 && i < 4; ++i)
	//	{
	//		printf(" %s", cur -> argv[i]);
	//	}
	//	printf (": exit(%d)\n", status);

	//}	
	thread_exit();
	  //DEBUG
}

