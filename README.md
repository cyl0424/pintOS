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

- **Add vm_less_func().** (vm/page.\*) <br>
     : Compare the vaddr of the two hash_em entered and return true and false. <br>  

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

### To-do 1. Define virtual memory entry structure. (vm/page.\*) <br>

``` C
enum vm_type {
    VM_ANON,
    VM_FILE,
    VM_BIN
};

struct vm_entry {
    uint8_t type;
    void *vaddr;
    bool writable;
    bool is_loaded;
    bool pinned_flag;
    struct file *file;
    struct list_elem mmap_elem;
    size_t offset;
    size_t read_bytes;
    size_t zero_bytes;
    size_t swap_slot;
    struct hash_elem elem;
};
```
> **Type of vm_entry** <br>
> - **VM_BIN** <br>
>   Loads data from binary files. <br>
>  - **VM_FILE** <br>
>   Load data from mapped files. <br>
>  - **VM_ANON** <br>
>   Load data from swap area. <br>

>  **Structure of vm_entry** <br>
> - **uint8_t type** <br>
>    Type that distinguishes VM_BIN, VM_FILE, and VM_ANON
> - **void *vaddr;** <br>
>    virtual page number
> - **bool writable** <br>
>    indicate whether the address is writable
> - **bool is_loaded** <br>
>    indicate whether physical memory is loaded
> - **bool pinned_flag** <br>
>    Type that distinguishes VM_BIN, VM_FILE, and VM_ANON
> - **struct file \*file** <br>
>    indicate which is a file mapped to a virtual address
> - **struct list_elem mmap_elem** <br>
>    the element of the mmap list
> - **size_t offset** <br>
>    offset of the file to be read
> - **size_t read_bytes** <br>
>    the data size written on the virtual page
> - **size_t zero_bytes** <br>
>    the bytes of the remaining page to be filled with 0
> - **size_t swap_slot** <br>
>    swap slot    
> - **struct hash_elem elem** <br>
>    hash table element                     
    
<br>

### To-do 2. Add vm_init(). (vm/page.\*) <br>

```C
void vm_init (struct hash *vm){
    if (vm != NULL){
        hash_init(vm, vm_hash_func, vm_less_func, NULL);
    }
}
```
> **Reasons for using hash** <br>
> - vm_entries should be managed in a bundle so that they can be navigated.<br>
> - Therefore, vm_entry is managed with a hash with fast navigation and hash values are extracted with vaddr.<br>

> **hash_init(vm, vm_hash_func, vm_less_func, NULL)** <br>
> - Initializes the hash table. <br>

<br>

### To-do 3. Add vm_hash_func(). (vm/page.\*) <br>
```C
unsigned vm_hash_func (const struct hash_elem *e, void *aux){
    unsigned int res;
    if (e != NULL){
        int hash_ent = hash_entry(e, struct vm_entry, elem) ->vaddr;
        res = hash_int(hash_ent);
        return res;
    }
    return NULL;
}
```
> **if (e != NULL){}** <br>
> - If the e pointer is not NULL, the code proceeds to extract a field ‘vaddr’ from a structure of type vm_entry using the hash_entry macro.<br>
> - This macro calculates the starting address of the structure that contains the hash_elem element.<br> 
> - It assumes that the hash_elem structure is embedded within another structure, in this case, struct vm_entry.<br>

> **int hash_ent = hash_entry(e, struct vm_entry, elem) ->vaddr** <br>
> - Once the ‘vaddr’ value is obtained, it is passed as an argument to the hash_int function.<br>
> - It takes an integer value and applies a hashing algorithm to produce a hash value. The resulting hash value ‘res’ is then returned by the function.<br>

<br>

### To-do 4. Add vm_less_func(). (vm/page.\*) <br>
```C
bool vm_less_func (const struct hash_elem *e1, const struct hash_elem *e2, void *aux){
    if (e1 != NULL && e2 != NULL){
        int e1_entry = hash_entry(e1, struct vm_entry, elem) ->vaddr;
        int e2_entry = hash_entry(e2, struct vm_entry, elem) ->vaddr;
        return e1_entry < e2_entry;
    }
    return NULL;
}
```
> **if (e1 != NULL && e2 != NULL){ }** <br>
> - If both pointers are not NULL, extract the ‘vaddr’ from the struct vm_entry structures associated with each hash_elem using the hash_entry macro.<br>

> **return e1_entry < e2_entry**<br>
> - Once the ‘vaddr’ values are obtained for both elements, a comparison is performed using the < operator, checking if the ‘vaddr’ of e1 is less than the ‘vaddr’ of e2.<br>
> - The result of the comparison (true or false) is then returned by the function. If e1 has a lower ‘vaddr’ value than e2, the function returns true, indicating that e1 should be placed before e2 in the hash table. Otherwise, it returns false.<br>

<br>

### To-do 5. Add insert_vme(). (vm/page.\*) <br>
```C
bool insert_vme (struct hash *vm, struct vm_entry *vm_entry){
    if(vm != NULL && vm_entry != NULL && pg_ofs(vm_entry->vaddr)==0){
        vm_entry->pinned_flag = false;
        bool res = (hash_insert(vm, &vm_entry->elem) == NULL);
        return res;
    }
    return false;
}
```

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
