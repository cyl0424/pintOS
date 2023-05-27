# Project 3 - Virtual Memory
> Demand paging <br>
> Stack growth <br>
> Memory Mapped Files <br>

<br>

## Files to modify
pintos/src/threads/thread.* <br>
pintos/src/userprog/syscall.* <br>
pintos/src/userprog/process.* <br>
pintos/src/userprog/exception.* <br>

## Files to add
pintos/src/vm/page.* <br>
pintos/src/vm/frame.* <br>
pintos/src/vm/swap.* <br>

<br>

## To-do - Demand paging
- **Define virtual memory entry structure.** (vm/page.\*) <br>
     : contain the page table index and page offset. <br>
     
- **Add vm_init().** (vm/page.\*) <br>
     : initialize hash table. <br>  

- **Add vm_hash_func().** (vm/page.\*) <br>
     : return a hash value. <br>  
 
 - **Add insert_vme().** (vm/page.\*) <br>
     : Insert vm_entry into hash table. <br>  

 - **Add delete_vme().** (vm/page.\*) <br>
     : Remove vm_entry from hash table. <br> 

 - **Add find_vme().** (vm/page.\*) <br>
     : Search and return vm_entry corresponding to vaddr entered as a factor. <br>

 - **Add vm_destroy().** (vm/page.\*) <br>
     : Remove the bucket list and vm_entry from the hash table. <br>

- **Add vm hash table structure in thread structure and add code to initialize hash table.** (threads/thread.\*) <br>
     : initialize hash table when process creates. <br>
 
 - **Modify process_exit().** (vm/page.\*) <br>
     : Add vm_destory() to remove vm_entries at the end of the process. <br>

- **Modify load_segment().** (userprog/procecss.\*) <br>
     : Adds ability to initialize process virtual memory related data structures. <br>

- **Modify setup_stack().** (userprog/process.\*) <br>
     : Create a vm_entry and set the field value of the created vm_entry to initialize and insert into the vm hash table.. <br>

- **Add address_check().** (userprog/syscall.\*) <br>
     : Use vm_entry to perform validation and return vm_entry. <br>
     
- **Add check_buffer().** (userprog/syscall.\*) <br>
     : Check if the address of the buffer in the read() system call is a valid virtual address. <br> 

- **Modify syscall_handler().** (userprog/syscall.\*) <br>
     : Add check_buffer() to validate with or without buffers. <br>  

- **Add load_file().** (vm/page.\*) <br>
     : Load pages that exist on disk into physical memory. <br>
     
- **Add install_page().** (vm/page.\*) <br>
     : Map physical and virtual addresses to page tables. <br>  

- **Add handle_mm_fault().** (userprog/process.\*) <br>
     : Assign a physical page when a page fault occurs.<br> 
     : Load files on disk into physical pages by calling load_file().<br> 
     : Map virtual and physical addresses to page tables when loading into physical memory is complete. <br>  
     
- **Modify page_fault().** (userprog/exeption.\*) <br>
     : Validates fault_addr and calls handle_mm_fault(). <br>
       
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
    return -1;
  }
}
```
> **Verify the validity of a user-provided pointer** <br>
> - **check if call is_user_vaddr(addr) is False** <br>
>   - addr == NULL : <br>
>   - is_user_vaddr(addr) : check if <br>
> - **check if the user virtual address is mapped** <br>
>   - pagedir_get_page(cur->pagedir, addr) : returns the kernel virtual address corresponding to that physical address, <br>
>                                            or a null pointer if UADDR is unmapped.  <br>
>   
> - **int cnt** <br>
>   add a variable to count the number of the tokens, that is, argc <br>
> - **strtok_r(file_name, " ", &save_ptr)** <br>
>   saparate a stiring into tokens by a certain delimeter. <br>
>   <br>
> - **while** tmp is not NULL(=there is any argument left), <br>
>   - save each token to argv[cnt] <br>
>   - increment cnt by 1 <br>
<br> 

> **Save tokens in user stack** <br>
> - **call argument_user_stack()** <br>
>   argument_user_stack(argv, cnt, &if_.esp) : stack up arguments in *argv* with the number of *cnt* on user stack. newly created function, described below. <br>
> - **call hex_dump()** <br>
>   hex_dump(if_.esp, if_.esp, PHYS_BASE- if_.esp, true) : debugging tool to show the contents of the stack.
<br>
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
