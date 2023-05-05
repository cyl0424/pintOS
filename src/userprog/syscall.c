#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* system calls */
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "threads/synch.h"

static void syscall_handler (struct intr_frame *);
struct lock filesys_lock;

void
syscall_init (void) 
{
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  switch(*(int32_t*)(f->esp)){
    case SYS_HALT: 
      halt();
      break;
    case SYS_EXIT: 
      address_check(f->esp+4);
      exit(*(int*)(f->esp+4));
      break;
    case SYS_EXEC:
      address_check(f->esp+4);
      f->eax=exec((char*)*(uint32_t*)(f->esp+4));
      break;
    case SYS_WAIT:
      address_check(f->esp+4);
      f->eax = wait(*(uint32_t*)(f->esp+4));
      break;
    case SYS_CREATE:
      address_check(f->esp+4);
      address_check(f->esp+8);
      f->eax = create((char*)*(uint32_t*)(f->esp+4), *(uint32_t*)(f->esp+8));
      break;
    case SYS_REMOVE: 
      address_check(f->esp+4);
      f->eax = remove((char*)*(uint32_t*)(f->esp+4));
      break;
    case SYS_OPEN:
      address_check(f->esp+4);
      f->eax = open((char*)*(uint32_t*)(f->esp+4));
      break;
    case SYS_FILESIZE:
      address_check(f->esp+4);
      f->eax = filesize(*(uint32_t*)(f->esp+4));
      break;
    case SYS_READ:
      address_check(f->esp+4);
      address_check(f->esp+8);
      address_check(f->esp+12);
      f->eax = read((int)*(uint32_t*)(f->esp+4), (void*)*(uint32_t*)(f->esp+8),
            (unsigned)*(uint32_t*)(f->esp+12));
      break;
    case SYS_WRITE:
      address_check(f->esp+4);
      address_check(f->esp+8);
      address_check(f->esp+12);
      f->eax = write((int)*(uint32_t*)(f->esp+4), (const void*)*(uint32_t*)(f->esp+8),
            (unsigned)*(uint32_t*)(f->esp+12));
      break;
    case SYS_SEEK:
      address_check(f->esp+4);
      address_check(f->esp+8);
      seek((int)*(uint32_t*)(f->esp+4), (unsigned)*(uint32_t*)(f->esp+8));
      break;
    case SYS_TELL:
      address_check(f->esp+4);
      f->eax = tell((int)*(uint32_t*)(f->esp+4));
      break;
    case SYS_CLOSE:
      address_check(f->esp+4);
      close(*(uint32_t*)(f->esp+4));
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
  struct thread *t=thread_current();
  printf("%s: exit(%d)\n", thread_name(), status);
  t->exit_status = status;
  thread_exit();
}


pid_t exec(const char *cmd_line){
  if(cmd_line == NULL){
    exit(-1);
  }

  tid_t tid = process_execute(cmd_line);
  if (tid == TID_ERROR)
    return -1;

  struct thread *child_t = get_child_process(tid);
  if (child_t == NULL)
    return -1;

  sema_down(&child_t->wait_sema);
  if (child_t->load_flag == false)
    return -1;

  return tid;
}


int wait(pid_t pid){
  if (pid <= 0){
    return-1;
  }
  process_wait(pid);
}


bool create (const char *file, unsigned initial_size){
  if (file == NULL){
    exit(-1);
  }
  return filesys_create(file, initial_size);
}


bool remove (const char *file){
  if(file == NULL){
    return -1;
  }
  return filesys_remove(file);
}


int open (const char *file){
  if(file == NULL){
    return -1;
  }

  lock_acquire(&filesys_lock);
  struct file *open_file = filesys_open(file);

  if (open_file == NULL){
    lock_release(&filesys_lock);
    return -1;
  }

  int fd = process_add_file(open_file);

  if(fd == -1 || fd == 128){
    file_close(open_file);
    return -1;
  }

  lock_release(&filesys_lock);
  return fd;
}


int filesize (int fd){
  struct file *f = process_get_file(fd);
  if (f==NULL){
    return -1;
  }
  return file_length(f);
}


int read (int fd, void *buffer, unsigned length){
  unsigned char *buf = buffer;
  int read_length;

  address_check(buffer);
  
  if (fd < 0 || fd == 1 || fd >= 128){
    return -1;
  }
  
  lock_acquire(&filesys_lock);
  
  if (fd == 0){
    for(read_length = 0; read_length < length; read_length++){
      char c = input_getc();
      if (!c){
        break;
      }
      *buf++ = c;
    }
  }
  else{
    struct file *f = process_get_file(fd);
    if (f == NULL){
      lock_release(&filesys_lock);
      return -1;
    }
    read_length = file_read(f, buffer, length);
  }
  lock_release(&filesys_lock);
  return read_length;
}


int write (int fd, const void *buffer, unsigned length){
  unsigned char *buf = buffer;
  int write_length;

  address_check(buffer);

  if (fd <=0 || fd >= 128){
    return -1;
  }
  
  lock_acquire(&filesys_lock);
  
  if (fd == 1){
    putbuf(buf, length);
  }
  else{
    struct file *f = process_get_file(fd);
    if (f == NULL){
      lock_release(&filesys_lock);
      return -1;
    }
    write_length = file_write(f, buffer, length);
  }
  lock_release(&filesys_lock);
  return write_length;
}

void seek (int fd, unsigned position){
  struct file *f = process_get_file(fd);
  if (f != NULL){
    file_seek(f, position);
  }
  else{
    exit(-1);
  }
}

unsigned tell (int fd){
  struct file *f = process_get_file(fd);
  if (f == NULL){
    exit(-1);
  }
  return file_tell(f);
}

void close (int fd){
  if(fd <= 2 || fd >= 128){
    return;
  }

  struct file *f = process_get_file(fd);
  if (f == NULL){
    return;
  }

  process_close_file(fd);
  file_close(f);

}

void address_check(void *addr) {
  struct thread *cur = thread_current();
  if (addr == NULL || !is_user_vaddr(addr) || pagedir_get_page(cur->pagedir, addr) == NULL) {
    exit(-1);
  }
}

int process_add_file(struct file *f){
  struct thread *cur = thread_current();
  int i;

  for (i = 3 ; i < 128; i++){
    if (cur->fd_table[i] == NULL){
      cur->fd_table[i] = f;
      cur->fd_index = i;
      cur->running_file = f;
      return cur->fd_index;
    }
  }

  cur->fd_index = 128;
  return -1;
}

void process_close_file(int fd_idx){
  struct thread *cur = thread_current();
  if (fd_idx < 3 || fd_idx >= 128){
    return NULL;
  }
  if(cur -> fd_table[fd_idx] != NULL){
    cur-> fd_table[fd_idx] = NULL;
    file_close(cur -> fd_table[fd_idx]);
  }
}

struct file *process_get_file(int fd_idx){
  if (fd_idx < 3 || fd_idx >= 128){
    return NULL;
  }
  return thread_current() -> fd_table[fd_idx];;
}