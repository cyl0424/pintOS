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
> **Make system call handler call system call. (userprog/syscall.c) ** <br>
> - **char \*token** <br>
>   add a variable to store the actual file name, and initialize as the result of strtok_r() <br>
> - **strtok_r(file_name, " ", &save_ptr)** <br>
>   saparate a stiring into tokens by a certain delimeter <br>
    the first time the strtok_r() function is called, it returns a pointer to the first token in string.<br>         
<br>
                                          
> **Check validation of the pointers in the parameter** <br>
> - **thread_create (token, PRI_DEFAULT, start_process, fn_copy)** <br>
>   because the new variable '\*token' now has a value of the first token of parsed string, pend it as the name of the new process. <br>

<br>

> **Copy arguments on the user stack to the kernel** <br>
> - **thread_create (token, PRI_DEFAULT, start_process, fn_copy)** <br>
>   because the new variable '\*token' now has a value of the first token of parsed string, pend it as the name of the new process. <br>

<br>

> **Save return value of system call** <br>
> - **thread_create (token, PRI_DEFAULT, start_process, fn_copy)** <br>
>   because the new variable '\*token' now has a value of the first token of parsed string, pend it as the name of the new process. <br>

<br>

### To-do 2. Add address_check() function. (userprog/syscall.c) <br>
``` C
void address_check(void *addr){
  struct thread *cur = thread_current();
  if (addr == NULL || !(is_user_vaddr(addr))){
    exit(-1);
  }
  if(!pagedir_get_page(cur->pagedir, addr)==NULL){
    exit(-1);
  }
}
```
> **Verify the validity of a user-provided pointer** <br>
>   user can pass invalid pointers through the systemcall, such as a null pointer or pointer to unmapped virtual memory. <br>
>   so, kernel need to detect invalidity of pointers and terminating process without harm to the kernel or other running processes. <br>
>   <br>
> - **check if user passes valid pointers** <br>
>   - **is_user_vaddr(addr)** : returns true if VADDR is a user virtual address. <br>
>    <br>
> - **check if the user virtual address is mapped** <br>
>   - **pagedir_get_page(cur->pagedir, addr)** : returns the kernel virtual address corresponding to that physical address, <br>
>                                                or a null pointer if UADDR is unmapped.  <br>
<br> 

### To-do 3. Add argument_user_stack() function. (userprog/process.\*) <br>
#### - process.h

``` C
...

void argument_user_stack(char **agrv,int argc,void **esp);

...
```
<br>

> **Declare the argument_user_stack() function in process.h** <br>


#### - process.c
```C
void argument_user_stack(char **argv,int argc,void **esp){
  char *argv_address[argc];
  int length = 0;

  int i;

  for (i = argc -1; i >= 0; i--){
    int instruction_size = strlen(argv[i])+1;
    *esp -= instruction_size;
    memcpy(*esp, argv[i], instruction_size);
    length += instruction_size;
    argv_address[i]=*esp;
  }

  if (length % 4 != 0){
    for (i = (4 - (length % 4)); i > 0; i--){
      *esp -= 1;
      **(char **)esp = 0;
    }
  }

  *esp = *esp - 4;
  **(char **)esp = 0;

  for (i = argc -1; i >= 0; i--){
    *esp -= 4;
    memcpy(*esp, &argv_address[i], strlen(&argv_address[i]));
  }

  *esp = *esp - 4;
  *(char **)(*esp) = (*esp+4);

  *esp = *esp - 4;
  **(char **)esp = argc; 

  *esp = *esp - 4;
  **(char **)esp = 0;                         
}
```
> **Stack the arguments on the user stack** <br>
> - **char \*argv_address[argc]** <br>
>   add an array to store the address of argv[] <br>
> - **int length** <br>
>   add a variable whose value is the total length of the instruction <br>
> - **stack arguments(String)** <br>
>   : save each argument from the top of the stack to the bottom
>   - for (i = argc -1; i >= 0; i--), <br>
>     - decrement the esp by the size of the argument <br>
>     - copy the memory content of argv to esp <- stack up the argument on user stack <br>
>     - set address_argv[i] to be the esp at that time when argv[i] is loaded on to the user stack <br>
> - **word align** <br>
>   : for the performance, add padding after finishing saving arguments <br>
>   - if (length % 4 != 0), <br>
>     - fill 0 to stack until the total length of the block becomes multiple of 4 <br>
> - **stack arguments' addresses(char \*)** <br>
>   : stack the address of each argument saved the userstack <br>
>   because user register is 4 byte units based, down the esp by 4 every for each iteration <br>
>   use argv_address[i] to get the address of each argument in user stack <br>
> - **main(int argc, char \*\*argv)** <br>
>   : stack the address of the address of the first argument, and argc <br>
> - **return address** <br>
>   : stack 0 as the fake address
<br>

### To-do 4. Modify setup_stack() function. (userprog/process.\*) <br>
#### - process.h
``` C
...

static bool setup_stack (void **esp);

...
```
<br>

> **Modify the declaration of setup_stack() function in process.h** <br>

#### - process.c

``` C
...

static bool
setup_stack (void **esp) 
{
  uint8_t *kpage;
  bool success = false;

  kpage = palloc_get_page (PAL_USER | PAL_ZERO);
  if (kpage != NULL) 
    {
      success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true);
      if (success)
        *esp = PHYS_BASE;
      else
        palloc_free_page (kpage);
    }
  return success;
} 

...
```
<br>

> **Make setup_stack() compatible with argument_user_stack()** <br>
