#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"

#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/directory.h"

#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "threads/synch.h"
/* K-th member of argv, casted as a TYPE */
#define KTH_ARG(FRAME, K, TYPE) ( *(((TYPE *) FRAME->esp) + K) )

/* Struct to hold data about an existing open fd.
 * Holds either a fd number.
 */
struct FD
{
  struct file *fd_file;  // file that FD has opened
  int fd;                // file descr. # of the file
};

#define MAX_FDS 128
static struct FD fds[MAX_FDS]; // keep track of open fds
static int init = 0;
struct FD getFileFromFDS(int fd);
static void syscall_handler (struct intr_frame *);
static struct lock f_lock;
int S__CREATE(const char *file, unsigned int size);
int S__READ(int fd, void * buffer, unsigned int size);
int S__WRITE(unsigned int fd, const void * buffer, unsigned int size);
void S__EXIT(int status);
int S__FILESIZE(int fd);
int S__OPEN(const char *file_name);
int S__TELL(int fd);
int S__WAIT(tid_t pid);
int S__EXEC(const char *cmd_line);
void validString ( const void * str);
void validArg(struct intr_frame *f, int n);
int validPtr(const void * ptr);
int getPagePtr(const void * v_ptr);
void getArgs ( struct intr_frame * f, int * args, int n);


static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  validArg(f,0);
  unsigned int _fd;
  void * readbuffer;
  void * writebuffer;
  unsigned int _size;
  int args[4];
	
  if (init == 0)
  {
    int i;
    
    // setup fds
    for (i = 0; i < MAX_FDS; ++i)
    {
      fds[i].fd = -1;
    }
    
    // init stdin, stdout
    fds[0].fd = 0;
    fds[1].fd = 1;
    fds[2].fd = 2;
    lock_init(&f_lock);
    init = 1;
  }
  
  unsigned int *x = f->esp;
  if(validPtr(x) == 0)
	S__EXIT(-1);
  switch(*x)
  {
  	case SYS_HALT:
		//printf("HALT");
 		shutdown_power_off(); // defined in shutdown.h
  		break;
  	case SYS_EXIT:
 		x += sizeof(int);
		S__EXIT(*x);
  		break;
  	case SYS_WAIT:
		//printf("WAIT.");
		validArg(f,1);
		S__WAIT(KTH_ARG(f,1,tid_t));
  		break;
  	case SYS_CREATE:
		//printf("CREATE");
		getArgs(f,args,2);
       		 validString( (const void *) args[0] ); // check for null files and stuff
        	f->eax = S__CREATE( (const char*)args[0], (unsigned int) args[1] );
  		break;
  	case SYS_REMOVE:
		printf("REMOVE");
  		break;
  	case SYS_OPEN:
		//printf("OPEN");
  		validArg(f,1);
		f->eax = S__OPEN(KTH_ARG(f,1,const char *));
  		break;
  	case SYS_FILESIZE:
		//printf("FILESIZE");
  		validArg(f,1);
      		f->eax = S__FILESIZE(KTH_ARG(f,1,int));
  		break;
  	case SYS_READ:
		//printf("READ");
		x += sizeof(int);
		_fd = *x;
		x += 1;
		readbuffer = *x;
		x += sizeof(unsigned);
		_size = *x;
		f -> eax = S__READ(KTH_ARG(f, 1, int), KTH_ARG(f, 2, const char *), KTH_ARG(f,3,unsigned int));
  		break;
  	case SYS_WRITE:
		
  
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
  		break;
  	case SYS_SEEK:
		validArg(f,1);
		validArg(f,2);
		S__SEEK(KTH_ARG(f,1,int), KTH_ARG(f,2,unsigned int));
//		printf("SEEK.");
  		break;
  	case SYS_TELL:
		//printf("TELL");
		validArg(f,1);
		f -> eax = S__TELL(KTH_ARG(f,1,int));
  		break;
  	case SYS_CLOSE:
		validArg(f,1);
		S__CLOSE(KTH_ARG(f,1,int));
	//	printf("CLOSE.");
  		break;
    case SYS_EXEC:
    	//printf("EXEC.");
   	validArg(f,1);
	S__EXEC(KTH_ARG(f, 1, const char *));
	break;
    default:
		//printf("UNDEFINED_SYSCALL.");
  	S__EXIT(-1);
}

}



/********************************************************************
*	SYSTEM CALL functions below
********************************************************************/

int S__CREATE(const char *file, unsigned int size)
{
  	return (int)filesys_create(file,size);
}

int S__EXEC(const char *cmd_line)
{
	if(cmd_line == NULL || is_user_vaddr(cmd_line)==0)
	{
		S__EXIT(-1);
	}
	lock_acquire(&f_lock);
	int ret = process_execute(cmd_line);
	lock_release(&f_lock);
	if (ret != TID_ERROR);
	//	process_wait(ret);
		//printf("s = %d\n", s)
		//S__EXIT(0);//process_wait(ret);
	return ret;

}

void S__CLOSE(int fd)
{
	if (fd > MAX_FDS - 1 || fd < 2)
	{
		S__EXIT(-1);
	}
	struct FD f = getFileFromFDS(fd);        
	if(f.fd_file == NULL || f.fd < 2 || f.fd > MAX_FDS - 1)
        {
		S__EXIT(-1);
	}
	
	file_close(f.fd_file);
	int i;
	for (i = 0; i < MAX_FDS; ++i)
        {
               // printf("fds[i].fd = %d", fds[i].fd);
                if (fds[i].fd == fd)
                {
                        fds[i].fd_file  = NULL;
			fds[i].fd = -1;
                        break;
                }
        }
}

int S__READ(int fd, void * buffer, unsigned int size) // jpc20140225
{

	// consider implementing a lock on stdin/stdout
	//ASSERT ( buffer != NULL && validPtr(buffer) );
	//unsigned int s = size;
	lock_acquire(&f_lock);
	if (validPtr(buffer)==0|| fd == 1 || fd < 0 || fd > MAX_FDS - 1)
	{
	//	printf("PTR ERROR\n fd = %d\n", fd);
		lock_release(&f_lock);
		S__EXIT(-1);//return -1;
	}
	//if (size==0)
	//{
	//	return 0;
///	}
	if (fd==0)//stdin
	{
		char c; // tmp to store character from stdin
	    unsigned int cnt = 0;
	    while (cnt < size)//read until size limit reached
	    {
			c = input_getc();
			((char *) buffer)[cnt] = c;
			cnt++;
	    }
		lock_release(&f_lock);
		return cnt;
	}
	struct FD f = getFileFromFDS(fd);
        if(f.fd_file == NULL || f.fd  < 2 || f.fd > MAX_FDS - 1)
        {

                lock_release(&f_lock);
		S__EXIT(-1);
	}
	else // (fd==1||fd==2||fd==??) -- stdout,stderr,other
	{
		int s = file_read(f.fd_file, buffer, size);
		lock_release(&f_lock);
		return s;
	}
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
	unsigned int s = size;
	//ASSERT ( buffer != NULL && buffer > PHYS_BASE);
	//printf("WRITE");
	lock_acquire(&f_lock);
	if(validPtr(buffer) == 0 )
	{
	//	printf("PTR ERROR\n");
		lock_release(&f_lock);
		S__EXIT(-1); //ERROR
	}
//	if (fd == 0)// consider allowing write to other FDs
//	{
		//printf("%d", fd);
//		S__EXIT(-1); //ERROR
//	}
	if (fd == 1)
	{
		putbuf(buffer, size);
		lock_release(&f_lock);
		return size;
	}
//	if(fd > MAX_FDS || fd < 0)
//	{
//		S__EXIT(-1);
//	}
	struct FD f =  getFileFromFDS(fd);
	if(f.fd_file == NULL || f.fd < 2 || f.fd > MAX_FDS - 1)
        {
	//	printf("OTHER ERROR\n");
		lock_release(&f_lock);
               // printf("s = %d", s);
                S__EXIT(-1); //ERROR
        }
	//if(fds[fd].fd_file == -1)
//	{	
//		S__EXIT(-1);
//	}
//	else
//	{
	//   printf("s = %d", s);
	   s = file_write(f.fd_file, buffer, size);
	//   printf("s = %d", s);
//	}
	lock_release(&f_lock);
	return s;
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
}

int S__FILESIZE(int fd)
{
	// check fd is valid
	if (fd < 2 || fd > MAX_FDS - 1) // avoid std fd's and used ones
	{
		return -1;
	}
	struct FD f = getFileFromFDS(fd);
	if( f.fd_file == NULL || f.fd < 2 || f.fd > MAX_FDS - 1)
		S__EXIT(-1);
	return (int)file_length(f.fd_file);
}

int S__OPEN(const char *file_name)
{
	if (!validPtr(file_name))
	{
		S__EXIT(-1);
	}

	int i;
	int curFd = -1;
	struct file *fileOpened;

	// find free fd
	for (i = 0; i < MAX_FDS; ++i)
	{
		if (fds[i].fd == -1)
		{
			curFd = i;
			break;
		}
	}

	if (curFd == -1)
	{
		return -1;
	}

	// try open file
	fileOpened = filesys_open(file_name);
	if (fileOpened == NULL)
	{
		return -1;
	}

	// set correct fd
	fds[curFd].fd_file = fileOpened;
	fds[curFd].fd = curFd;

	// disable write if file running
	if (strcmp(file_name, thread_current()->name) == 0)
	{
		file_deny_write(fds[curFd].fd_file);
	}

	return curFd;
}
int S__SEEK(int fd, unsigned int position)
{
	struct FD f = getFileFromFDS(fd);
	if( f.fd_file == NULL || f.fd < 2 || f.fd > MAX_FDS - 1)
	{
		S__EXIT(-1);
	}
	file_seek(f.fd_file, position);
	return 0;



}

int S__TELL(int fd)
{ 
	struct FD f = getFileFromFDS(fd);
	if (f.fd_file == NULL || f.fd < 2 || f.fd > MAX_FDS - 1)
	{
		return -1;
	}
	return file_tell(f.fd_file);
}

int S__WAIT(tid_t pid)
{
	printf("WAITING...\n");
	return (int)process_wait(pid);
}



/********************************************************************
*	helper functions below
********************************************************************/


// verify that ptr is in userspace and address is good
int validPtr(const void *ptr)
{
  if ( ptr > (PHYS_BASE - sizeof(ptr)) )
    return 0;
  else
    return pagedir_get_page( thread_current()->pagedir, ptr ) != NULL;
}

int getPagePtr(const void *v_ptr)
{
    if (validPtr(v_ptr) == 0)
    	S__EXIT(-1);
    void * ptr = pagedir_get_page(thread_current()->pagedir,v_ptr);
    if (ptr == NULL)
    {
     	S__EXIT(-1);
    }
    return (int)ptr;
}

void getArgs(struct intr_frame *f, int * args, int n)
{
    int i;
    int *ptr;
    for (i = 0; i < n ; i++)
    {
     	ptr = (int *) f->esp + i + 1;
        if (validPtr((const void*) ptr) == 0)
        	S__EXIT(-1);
        args[i] = *ptr;
    }
}

void validString ( const void* str)
{
    while ( *( char *) getPagePtr(str) != 0 )
    {
        str = (char * ) str+1;
    }
}

// validates the nth argument on given frame
void validArg(struct intr_frame *f, int n)
{
	if ( !validPtr(&KTH_ARG(f, n, int *)) )
	{
		S__EXIT(-1);
	}
}


struct FD getFileFromFDS(int fd)
{
	int i;
        struct FD f;
	f.fd_file = NULL;
	f.fd = -1;
        for (i = 0; i < MAX_FDS; ++i)
        {
	//	printf("fds[i].fd = %d", fds[i].fd);
                if (fds[i].fd == fd)
                {
                        f = fds[i];
                        break;
                }
        }
	return f;
}

	//}	
	thread_exit();
	  //DEBUG
}

