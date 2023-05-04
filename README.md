# Project 2 - System Call
> Add system calls to provide services to users. <br>
> - Process related: halt, exit, exec, wait
> - File related: create, remove, open, filesize, read, write, seek, tell, close

<br>

## Files to modify
pintos/src/threads/thread.* <br>
pintos/src/userprog/syscall.* <br>
pintos/src/userprog/process.* <br>

<br>

## To-do
- **Implement system call handler.** (userprog/syscall.\*) <br>
     : make system call handler call system call <br>
      check validation of the pointers in the parameter <br>
      copy arguments on the user stack to the kernel <br>
      save return value of system call <br>
     
- **Add address_check() function.** (userprog/syscall.\*) <br>
     : detect invalidity of pointers and terminating process without harm to the kernel. <br>

- **Add system call halt().** (userprog/syscall.\*) <br>
     : shutdown pintos. <br>
     
- **Add system call exit().** (userprog/syscall.\*) <br>
     : exit pintos. <br>
     
- **Add system call exec().** (userprog/syscall.\*) <br>
     : create child process and execute program. <br>
     
- **Add system call wait().** (userprog/syscall.\*) <br>
     : wait for termination of child process. <br> 
     
- **Implement file descriptor table.** (userprog/syscall.\*) <br>
     : implement an array of pointers to struct file, used to access to files. <br>
     
- **Add system call create().** (userprog/syscall.\*) <br>
     : create file. <br>
     
- **Add system call remove().** (userprog/syscall.\*) <br>
     : remove file. <br>  

- **Add system call open().** (userprog/syscall.\*) <br>
     : open file. <br>
     
- **Add system call filesize().** (userprog/syscall.\*) <br>
     : return the size, in bytes, of the open file. <br>  

- **Add system call read().** (userprog/syscall.\*) <br>
     : read size bytes from the file open as fd into buffer. <br>
     
- **Add system call write().** (userprog/syscall.\*) <br>
     : write data from buffer to open file. <br>  
     
- **Add system call seek().** (userprog/syscall.\*) <br>
     : changes the next byte to be read or written in open file. <br>
     
- **Add system call tell().** (userprog/syscall.\*) <br>
     : stack the arguments on the user stack. <br> 

- **Add system call close().** (userprog/syscall.\*) <br>
     : stack the arguments on the user stack. <br>
     
- **Modify load() and process_exit().** <br>
     : deny writes to executing files. <br>
       
<br>
<br>

## Project Description

### To-do 1. Implement system call handler. (userprog/syscall.\*) <br>

``` C
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
```
> **Make system call handler call system call. (userprog/syscall.c)** <br>
>   for each case, call corresponding system calls with appropriate addresses <br>
<br>
                                          
> **Check validation of the pointers in the parameter** <br>
> - **address_check(f->esp+n)** : check if user is passing vailid pointer and terminate the process if not <br>

<br>

> **Save return value of system call** <br>
>   save the return value of each system call to f->eax <br>

<br>

### To-do 2. Add address_check() function. (userprog/syscall.c) <br>

``` C
void address_check(void *addr){
  struct thread *cur = thread_current();
  if (addr == NULL || !(is_user_vaddr(addr))){
    exit(-1);
  }
  if(!pagedir_get_page(cur->pagedir, addr)==NULL){
    return -1;
  }
}
```

> **Verify the validity of a user-provided pointer** <br>
>   user can pass invalid pointers through the systemcall, such as a null pointer or pointer to unmapped virtual memory. <br>
>   so, kernel need to detect invalidity of pointers and terminating process without harm to the kernel or other running processes. <br>
> - **check if user passes valid pointers** <br>
>   - **is_user_vaddr(addr)** : returns true if VADDR is a user virtual address. <br>
>    <br>
> - **check if the user virtual address is mapped** <br>
>   - **pagedir_get_page(cur->pagedir, addr)** : returns the kernel virtual address corresponding to that physical address, <br>
>                                                or a null pointer if UADDR is unmapped.  <br>
<br> 

### To-do 3-1. Add system call halt().** (userprog/syscall.\c) <br>
#### - process.c

```C
void halt(void){
  shutdown_power_off();                     
}
```
<br>

> **System call to shutdown pintos** <br>
> - **shutdown_power_off()** <br>
>   : Powers down the machine we're running on, as long as we're running on Bochs or QEMU. <br>

<br>

### To-do 3-2. Add system call exit().** (userprog/syscall.\c) (threads/thread.h) <br>
#### - process.c
``` C
void exit(int status){
  struct thread *t=thread_current();
  printf("%s: exit(%d)\n", thread_name(), status);
  t->exit_status = status;
  thread_exit();
}
```
<br>

> **System call to exit pintos** <br>
> - **print thread name and its status**
> - **int exit_status** <br>
>   save the process's exit status to be the return value of wait() call <br>
> - **terminate the process** <br>
>   - **thread_exit()** : deschedules the current thread and destroys it <br>

#### - thread.h
``` C
struct thread {

  ...

  int exit_status;
  
  ...

}
```
<br>

> **Add int type field named 'exit_status' to thread structure** <br>

<br>

### To-do 3-3. Add system call exec().** (userprog/syscall.\c) <br>
#### - process.c
``` C
pid_t exec(const char *cmd_line){
  tid_t tid = process_execute(cmd_line);
  if(tid != -1){
    struct thread *child_t = get_child_process(tid);
    
    if(child_t!=NULL){
      if(child_t->load_flag == false){
        return -1;
      }
      else{
        return tid;
      }
    }
  }
  return tid;
}
```
<br>

> **System call to create child process and execute program corresponds to cmd_line on it** <br>
> - **call process_execute(cmd_line)** <br>
> - **int exit_status** <br>
>   save the process's exit status to be the return value of wait() call <br>
> - **terminate the process** <br>
>   - **thread_exit()** : deschedules the current thread and destroys it <br>
<br>

``` C
tid_t
process_execute (const char *file_name){

...

  tid = thread_create (token, PRI_DEFAULT, start_process, fn_copy);
  if (tid == TID_ERROR){
    palloc_free_page (fn_copy);
  }
  else{
    struct thread *cur = thread_current ();
    struct list_elem* e = list_pop_back(&cur->child_thread_list);
    struct thread *child_t = list_entry(e, struct thread, child_elem);
		
    sema_down(&child_t->wait_sema);
    if(!child_t->load_flag){
      tid = TID_ERROR;
    } 
    else{
      list_push_back(&cur->child_thread_list, e);
    }
  }
  
...
  return tid;
}
```
<br>

> **Modify process_execute()** <br>
>   parent should wait until it knows the child process has successfully created and the binary file is successfully loaded <br>
> - **If tid is not error, decrement wait_sema of child process by 1** <br>
>   - sema_down(&child_t->wait_sema) <br>
>   - **If child process successes to load, put the process to child_thread_list** <br>
> **Return tid** <br>
<br>

### To-do 3-4. Add system call wait().** (userprog/syscall.\c) <br>
#### - syscall.c

``` C
int wait(pid_t pid){
  process_wait(pid);
}

```
<br>

> **wait for termination of child process* <br>
> - **call process_execute(cmd_line)** <br>
> - **int exit_status** <br>
>   save the process's exit status to be the return value of wait() call <br>
> - **terminate the process** <br>
>   - **thread_exit()** : deschedules the current thread and destroys it <br>
<br>
