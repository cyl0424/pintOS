#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* system calls */
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  char *fn_copy;
	int size;
	
	switch (f->eax)
	{
    case SYS_HALT:
      halt();
      break;
    case SYS_EXIT:
      exit(f->edi);
      break;
    case SYS_EXEC:
      break;
    case SYS_WAIT:
      break;
    case SYS_CREATE:
      break;
    case SYS_REMOVE:
      break;
    case SYS_OPEN:
      break;
    case SYS_FILESIZE:
      break;
    case SYS_READ:
      break;
    case SYS_WRITE:
      break;
    case SYS_SEEK:
      break;
    case SYS_TELL:
      break;
    case SYS_CLOSE:
      break;
    default:
      exit(-1);
      break;
    }
}

void halt(void){
  shutdown_power_off();
}

void exit(int status){
  printf("%s: exit(%d)", thread_name(), status);
  thread_exit();
}

pid_t exec (const char *cmd_line){
  
}

int wait (pid_t pid){

}

bool create (const char *file, unsigned initial_size){
  return filesys_create(file, initial_size);
}

bool remove (const char *file){
  return filesys_remove(file);
}

int open (const char *file){
  struct file *open_file = filesys_open(file);

  if (open_file == NULL){
    return -1;
  }

  struct thread *cur = thread_current();

}

int filesize (int fd){
  return 0;
}

int read (int fd, void *buffer, unsigned length){

}

int write (int fd, const void *buffer, unsigned length){

}

void seek (int fd, unsigned position){

}

unsigned tell (int fd){

}

void close (int fd){

}

static struct file *find_file(int fd){
  
}