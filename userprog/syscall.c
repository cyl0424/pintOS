#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
/* for shutdown_power_off */
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "filesys/file.h"
#include "devices/input.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "vm/swap.h"

struct file_descriptor
{
  int fd_num;
  tid_t owner;
  struct file *file_struct;
  struct list_elem elem;
};

/* a list of open files, represents all the files open by the user process
   through syscalls. */
struct list open_files; 

/* the lock used by syscalls involving file system to ensure only one thread
   at a time is accessing file system */
struct lock fs_lock;


static void syscall_handler (struct intr_frame *);

/* System call functions */
static void halt (void);
static void exit (int);
static pid_t exec (const char *);
static int wait (pid_t);
static bool create (const char*, unsigned);
static bool remove (const char *);
static int open (const char *);
static int filesize (int);
static int read (int, void *, unsigned);
static int write (int, const void *, unsigned);
static void seek (int, unsigned);
static unsigned tell (int);
static void close (int);
/* End of system call functions */

static struct file_descriptor *get_open_file (int);
static void close_open_file (int);
bool is_valid_ptr (const void *);
static int allocate_fd (void);

static struct vm_entry *address_check (void *, void *);
static void *check_buffer (void *, unsigned , void *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  list_init (&open_files);
  lock_init (&fs_lock);
}

static struct vm_entry
*address_check (void *addr, void *esp)
{
  if (!(is_user_vaddr (addr)) || addr <= (void *)0x08048000UL){
    exit (-1);
  }

  if (!find_vme (addr))
    {   
      exit(-1);
    }
  return find_vme(addr);
}

static void
*check_buffer (void *buf, unsigned size, void *esp)
{
  int i;
  for (i=buf; i <= buf + size; i++){
    struct vm_entry *vme;
    vme = address_check(buf, esp);
    if (vme == NULL){
      exit(-1);
    }
    if (vme->writable != true){
      exit(-1);
    }
  }
}

static void
syscall_handler (struct intr_frame *f)
{
  uint32_t *esp;
  esp = f->esp;
  if (!is_valid_ptr (esp) || !is_valid_ptr (esp + 1) ||
      !is_valid_ptr (esp + 2) || !is_valid_ptr (esp + 3))
    {
      exit (-1);
    }
  else
    {
      int syscall_number = *esp;
      switch (syscall_number)
        {
        case SYS_HALT:
          halt ();
          break;
        case SYS_EXIT:
          address_check(esp + 1, esp);
          exit (*(esp + 1));
          break;
        case SYS_EXEC:
          address_check(esp + 1, esp);
          f->eax = exec ((char *) *(esp + 1));
          break;
        case SYS_WAIT:
          address_check(esp + 1, esp);
          f->eax = wait (*(esp + 1));
          break;
        case SYS_CREATE:
          address_check(esp + 1, esp);
          address_check(esp + 2, esp);
          f->eax = create ((char *) *(esp + 1), *(esp + 2));
          break;
        case SYS_REMOVE:
          address_check(esp + 1, esp);
          f->eax = remove ((char *) *(esp + 1));
          break;
        case SYS_OPEN:
          address_check(esp + 1, esp);
          f->eax = open ((char *) *(esp + 1));
          break;
        case SYS_FILESIZE:
          address_check(esp + 1, esp);
	        f->eax = filesize (*(esp + 1));
	        break;
        case SYS_READ:
          address_check(esp + 1, esp);
          address_check(esp + 2, esp);
          address_check(esp + 3, esp);
          check_buffer((void *) *(esp + 2), *(esp + 3), esp);
          f->eax = read (*(esp + 1), (void *) *(esp + 2), *(esp + 3));
          break;
        case SYS_WRITE:
          address_check(esp + 1, esp);
          address_check(esp + 2, esp);
          address_check(esp + 3, esp);
          check_buffer((void *) *(esp + 2), *(esp + 3), esp);
          f->eax = write (*(esp + 1), (void *) *(esp + 2), *(esp + 3));
          break;
        case SYS_SEEK:
          address_check(esp + 1, esp);
          address_check(esp + 2, esp);
          seek (*(esp + 1), *(esp + 2));
          break;
        case SYS_TELL:
          address_check(esp + 1, esp);
          f->eax = tell (*(esp + 1));
          break;
        case SYS_CLOSE:
          address_check(esp + 1, esp);
          close (*(esp + 1));
          break;
        case SYS_MMAP:
          address_check(esp + 1, esp);
          address_check(esp + 2, esp);
          f->eax = mmap(*(esp + 1), (void *) *(esp + 2));
          break;
        case SYS_MUNMAP:
          address_check(esp + 1, esp);
          munmap(*(esp + 1));
          break;
        default:
          break;
        }
    }
}


/* Terminates the current user program, returning status to the kernel.
 */
void
exit (int status)
{
  /* later on, we need to determine if there is process waiting for it */
  /* process_exit (); */
  struct child_status *child;
  struct thread *cur = thread_current ();
  struct thread *parent = NULL;
  printf ("%s: exit(%d)\n", cur->name, status);
  parent = thread_get_by_id (cur->parent_id);
  if (parent != NULL) 
    {
      struct list_elem *e = list_tail(&parent->children);
      while ((e = list_prev (e)) != list_head (&parent->children))
        {
          child = list_entry (e, struct child_status, elem_child_status);
          if (child->child_id == cur->tid)
          {
            lock_acquire (&parent->lock_child);
            child->is_exit_called = true;
            child->child_exit_status = status;
            lock_release (&parent->lock_child);
          }
        }
    }
  thread_exit ();
}

void
halt (void)
{
  shutdown_power_off ();
}

pid_t
exec (const char *cmd_line)
{
  /* a thread's id. When there is a user process within a kernel thread, we
   * use one-to-one mapping from tid to pid, which means pid = tid
   */
  tid_t tid;
  struct thread *cur;
  /* check if the user pinter is valid */
  if (!is_valid_ptr (cmd_line))
    {
      exit (-1);
    }

  cur = thread_current ();

  cur->child_load_status = 0;
  tid = process_execute (cmd_line);
  lock_acquire(&cur->lock_child);
  while (cur->child_load_status == 0)
    cond_wait(&cur->cond_child, &cur->lock_child);
  if (cur->child_load_status == -1)
    tid = -1;
  lock_release(&cur->lock_child);
  return tid;
}

int 
wait (pid_t pid)
{ 
  return process_wait(pid);
}

bool
create (const char *file_name, unsigned size)
{
  bool status;

  if (!is_valid_ptr (file_name))
    exit (-1);

  lock_acquire (&fs_lock);
  status = filesys_create(file_name, size);  
  lock_release (&fs_lock);
  return status;
}

bool 
remove (const char *file_name)
{
  bool status;

  if (!is_valid_ptr (file_name))
    exit (-1);

  lock_acquire (&fs_lock);  
  status = filesys_remove (file_name);
  lock_release (&fs_lock);
  return status;
}

int
open (const char *file_name)
{
  struct file *f;
  struct file_descriptor *fd;
  int status = -1;

  if (!is_valid_ptr (file_name))
    exit (-1);

  lock_acquire (&fs_lock); 

  f = filesys_open (file_name);
  if (f != NULL)
    {
      fd = calloc (1, sizeof *fd);
      fd->fd_num = allocate_fd ();
      fd->owner = thread_current ()->tid;
      fd->file_struct = f;
      list_push_back (&open_files, &fd->elem);
      status = fd->fd_num;
    }
  lock_release (&fs_lock);
  return status;
}

int
filesize (int fd)
{
  struct file_descriptor *fd_struct;
  int status = -1;
  lock_acquire (&fs_lock); 
  fd_struct = get_open_file (fd);
  if (fd_struct != NULL)
    status = file_length (fd_struct->file_struct);
  lock_release (&fs_lock);
  return status;
}

int
read (int fd, void *buffer, unsigned size)
{
  struct file_descriptor *fd_struct;
  int status = 0; 

  if (!is_valid_ptr (buffer) || !is_valid_ptr (buffer + size - 1))
    exit (-1);

  lock_acquire (&fs_lock); 

  if (fd == STDOUT_FILENO)
    {
      lock_release (&fs_lock);
      return -1;
    }

  if (fd == STDIN_FILENO)
    {
      uint8_t c;
      unsigned counter = size;
      uint8_t *buf = buffer;
      while (counter > 1 && (c = input_getc()) != 0)
        {
          *buf = c;
          buffer++;
          counter--; 
        }
      *buf = 0;
      lock_release (&fs_lock);
      return (size - counter);
    } 

  fd_struct = get_open_file (fd);
  if (fd_struct != NULL)
    status = file_read (fd_struct->file_struct, buffer, size);

  lock_release (&fs_lock);
  return status;
}

int
write (int fd, const void *buffer, unsigned size)
{
  struct file_descriptor *fd_struct;  
  int status = 0;

  if (!is_valid_ptr (buffer) || !is_valid_ptr (buffer + size - 1))
    exit (-1);

  lock_acquire (&fs_lock); 

  if (fd == STDIN_FILENO)
    {
      lock_release(&fs_lock);
      return -1;
    }

  if (fd == STDOUT_FILENO)
    {
      putbuf (buffer, size);
      lock_release(&fs_lock);
      return size;
    }

  fd_struct = get_open_file (fd);
  if (fd_struct != NULL)
    status = file_write (fd_struct->file_struct, buffer, size);
  lock_release (&fs_lock);
  return status;
}


void 
seek (int fd, unsigned position)
{
  struct file_descriptor *fd_struct;
  lock_acquire (&fs_lock); 
  fd_struct = get_open_file (fd);
  if (fd_struct != NULL)
    file_seek (fd_struct->file_struct, position);
  lock_release (&fs_lock);
  return ;
}

unsigned 
tell (int fd)
{
  struct file_descriptor *fd_struct;
  int status = 0;
  lock_acquire (&fs_lock); 
  fd_struct = get_open_file (fd);
  if (fd_struct != NULL)
    status = file_tell (fd_struct->file_struct);
  lock_release (&fs_lock);
  return status;
}

void 
close (int fd)
{
  struct file_descriptor *fd_struct;
  lock_acquire (&fs_lock); 
  fd_struct = get_open_file (fd);
  if (fd_struct != NULL && fd_struct->owner == thread_current ()->tid)
    close_open_file (fd);
  lock_release (&fs_lock);
  return ; 
}

struct file_descriptor *
get_open_file (int fd)
{
  struct list_elem *e;
  struct file_descriptor *fd_struct; 
  e = list_tail (&open_files);
  while ((e = list_prev (e)) != list_head (&open_files)) 
    {
      fd_struct = list_entry (e, struct file_descriptor, elem);
      if (fd_struct->fd_num == fd)
	      return fd_struct;
    }
  return NULL;
}

void
close_open_file (int fd)
{
  struct list_elem *e;
  struct list_elem *prev;
  struct file_descriptor *fd_struct; 
  e = list_end (&open_files);
  while (e != list_head (&open_files)) 
    {
      prev = list_prev (e);
      fd_struct = list_entry (e, struct file_descriptor, elem);
      if (fd_struct->fd_num == fd)
	      {
	        list_remove (e);
          file_close (fd_struct->file_struct);
	        free (fd_struct);
	        return ;
	      }
      e = prev;
    }
  return ;
}


/* The kernel must be very careful about doing so, because the user can
 * pass a null pointer, a pointer to unmapped virtual memory, or a pointer
 * to kernel virtual address space (above PHYS_BASE). All of these types of
 * invalid pointers must be rejected without harm to the kernel or other
 * running processes, by terminating the offending process and freeing
 * its resources.
 */
bool
is_valid_ptr (const void *usr_ptr)
{
  struct thread *cur = thread_current ();
  if (usr_ptr != NULL && is_user_vaddr (usr_ptr))
    {
      return (pagedir_get_page (cur->pagedir, usr_ptr)) != NULL;
    }
  return false;
}

int
allocate_fd ()
{
  static int fd_current = 1;
  return ++fd_current;
}

void
close_file_by_owner (tid_t tid)
{
  struct list_elem *e;
  struct list_elem *next;
  struct file_descriptor *fd_struct; 
  e = list_begin (&open_files);
  while (e != list_tail (&open_files)) 
    {
      next = list_next (e);
      fd_struct = list_entry (e, struct file_descriptor, elem);
      if (fd_struct->owner == tid)
        {
	        list_remove (e);
	        file_close (fd_struct->file_struct);
          free (fd_struct);
	      }
      e = next;
    }
}

int
mmap (int fd, void *addr){
  struct mmap_file *mmap_file;
  size_t offset = 0;

  if(find_vme(addr)){
      return -1;
    }

  if (pg_ofs (addr) != 0 || !addr || is_user_vaddr (addr) == false)
    return -1;
  
  mmap_file = malloc (sizeof (struct mmap_file));
  if (mmap_file == NULL){
    return -1;
  }

  memset(mmap_file, 0, sizeof(struct mmap_file));

  list_init (&mmap_file ->vme_list);

  struct thread *cur = thread_current();

  if (!(mmap_file->file = get_open_file(fd))){
    return -1;
  }

  mmap_file->file = file_reopen(mmap_file->file);
  mmap_file->mapid = cur->next_mapid++;

  list_push_back(&cur-> mmap_list, &mmap_file->elem);
  list_init(&mmap_file->vme_list);

  int len = file_length(mmap_file->file);

  while(len > 0){
    if(find_vme(addr)){
      return -1;
    }

    struct vm_entry *vme = malloc (sizeof (struct vm_entry));
    memset(vme, 0, sizeof(struct vm_entry));

    int read_bytes_ = PGSIZE;
    if (len < PGSIZE){
      read_bytes_ = len;
    }

    vme->type = VM_FILE;
    vme->vaddr = addr;
    vme->writable = true;
    vme->is_loaded = false;
    vme->file = mmap_file->file;
    vme->offset = offset;
    vme->read_bytes = read_bytes_;
    vme->zero_bytes = PGSIZE - read_bytes_;

    list_push_back(&mmap_file->vme_list, &vme->mmap_elem); 
    insert_vme(&cur->vm, vme);
    
    len -= read_bytes_;
    addr += PGSIZE;
    offset += read_bytes_;
  }
  return mmap_file->mapid;
}

void munmap (int mapid){
  do_munmap(mapid);
}
