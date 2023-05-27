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
     : initialize hash table when process starts. <br>
 
- **Modify process_exit().** (userprog/process.\*) <br>
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
> - **void *vaddr** <br>
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
> **if(vm != NULL && vm_entry != NULL && pg_ofs(vm_entry->vaddr)==0){ }**<br>
> - if the vm pointer is not NULL, indicating that the hash table is valid and exists.<br>
> - if the vm_entry pointer is not NULL, indicating that the entry to be inserted is valid and exists.<br>
> - if the ‘vaddr’ field of vm_entry is aligned to a page boundary by using the pg_ofs macro and checking if it equals 0. This condition ensures that the ‘vaddr’ is properly aligned.<br>

> **bool res = (hash_insert(vm, \&vm_entry->elem) == NULL)**<br>
> - If all three conditions are satisfied, the code proceeds to call the hash_insert function, passing the vm hash table and the elem member of vm_entry as arguments. The hash_insert function attempts to insert the element into the hash table.<br>
> - The result of the hash_insert function is then compared to NULL using the equality (==) operator. If the result is NULL, it indicates that the insertion was successful, and the function returns true. Otherwise, if the result is not NULL, it means that there was already an element with the same key in the hash table, and the function returns false.<br>

<br>

### To-do 6. Add delete_vme(). (vm/page.\*) <br>
```C
bool delete_vme (struct hash *vm, struct vm_entry *vm_entry){
    if(vm != NULL && vm_entry != NULL){
        if(!hash_delete(vm, &vm_entry->elem)){
            return false;
        };

        free_page(pagedir_get_page(thread_current()->pagedir, vm_entry->vaddr));
        free(vm_entry);
        return true;
    }
    return false;
}
```
> **if(vm != NULL && vm_entry != NULL){ }**<br>
> - If both vm and vm_entry are not NULL, the code proceeds to call hash_delete with the vm hash table and &vm_entry->elem. &vm_entry->elem is the address of the hash_elem member within the vm_entry structure. It serves as the key for locating the element to be deleted in the hash table.<br>
> - The hash_delete function attempts to remove the element with a matching key from the hash table. If the deletion is successful, hash_delete returns true; otherwise, it returns false.<br>

<br>

### To-do 7. Add find_vme(). (vm/page.\*) <br>
```C
struct vm_entry *find_vme (void *vaddr){
    struct vm_entry vme;
    struct hash_elem *e;

    if (vaddr != NULL){
        vme.vaddr = pg_round_down(vaddr);
        struct thread *cur = thread_current();

        if (pg_ofs(vme.vaddr) == 0){
            e = hash_find(&cur->vm, &vme.elem);  
        }
    }

    if (e != NULL){
        struct vm_entry *res = hash_entry(e, struct vm_entry, elem);
        return res;
    }
    else {
        return NULL;
    }
}
```
> **vme.vaddr = pg_round_down (vaddr)**<br>
> - vme.addr is page number of vme.<br>
> - The vme.vaddr member is assigned the rounded-down value of ‘vaddr’ using pg_round_down(). This ensures that the virtual address is aligned to the page boundary.<br>
> - The function checks if the offset of vme.vaddr is 0 using pg_ofs(vme.vaddr). This condition verifies if the vme.vaddr represents the starting address of a page.
If the offset is 0, indicating that vme.vaddr is a valid starting address of a page, the function calls hash_find() with the hash table &cur->vm and the address of vme.elem as the key. This searches for an entry in the hash table with a matching key.<br>

> **if (e != NULL){ }**<br>
> - If the search is successful (e is not NULL), the code retrieves the corresponding vm_entry structure using hash_entry(). It passes e as the hash element pointer, struct vm_entry as the structure type, and elem as the name of the hash_elem member within struct vm_entry. This gives the correct offset to access the vm_entry structure from the hash_elem.<br>
> - Finally, if a matching vm_entry structure is found, it is returned. Otherwise, NULL is returned to indicate that the vm_entry could not be found in the hash table.<br>

<br>

### To-do 8. Add vm_destroy(). (vm/page.\*) <br>
**Implement vm_destroy_func()**<br>
```C
void vm_destroy_func (struct hash_elem *e, void *aux){
    if (e != NULL){
        struct vm_entry *vme = hash_entry(e, struct vm_entry, elem);
        free_page(pagedir_get_page(thread_current()->pagedir, vme->vaddr));
        free(vme);
    }
}
```
> - After the assertion, use hash_entry to obtain a pointer to the parent structure containing e. In this case, it retrieves a pointer to the vm_entry structure associated with e.<br>
> - Then proceed to free the page associated with the vm_entry using free_page. It calls pagedir_get_page to retrieve the kernel virtual address corresponding to the user virtual address stored in vme->vaddr. This allows accessing the physical page associated with the user virtual address. The free_page function frees the physical page.<br>
> - Finally, call free() to release the memory occupied by the vm_entry structure itself. <br>

**Implementaion vm_destroy() using vm_destroy_func()**<br>
```C
void vm_destroy (struct hash *vm){
    if(vm != NULL){
        hash_destroy(vm, vm_destroy_func);
    }
}
```
> **hash_destroy(vm, vm_destroy_func)**<br>
> - Use the hash_destroy() function to remove bucket lists and vm_entries from the hash table.<br>

<br>

### To-do 9. Add vm hash table structure in thread structure and add code to initialize hash table. (threads/thread.\* , userprog/process.\*) <br>
#### thread.h <br>
```C
struct thread
{

...
struct hash vm;
...

}
```
<br>

#### process.c <br>

```C
static void
start_process (void *file_name_)
{
...

  vm_init(&cur->vm);

...
  }
```
> - Initialize by adding the vm_init() function.<br>

<br>

### To-do 10. Modify process_exit(). (userprog/process.\*) <br>
```C
void
process_exit (void)
{
...

  vm_destroy(&cur->vm);

...
}
```
> - Use the hash_destroy() function to remove bucket lists and vm_entries from the hash table.<br>

<br>

### To-do 11. Modify load_segment(). (userprog/procecss.\*) <br>
```C
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{

...

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0) 
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      struct vm_entry *vm_ent = (struct vm_entry *)malloc(sizeof(struct vm_entry));
      if (vm_ent == NULL){
        return false;
      }

      vm_ent -> type = VM_BIN;
      vm_ent -> vaddr = upage;
      vm_ent -> writable = writable;
      vm_ent -> file = file;
      vm_ent -> offset = ofs;
      vm_ent -> read_bytes = read_bytes;
      vm_ent -> zero_bytes = zero_bytes;

      insert_vme(&thread_current()->vm, vm_ent);

      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      ofs += page_read_bytes;
      upage += PGSIZE;
    }
  return true;
}
```
> **load_segment(){ }**<br>
> - The load_segment() function reads the data and code segments and the setup_stack() function assigns a physical page to the stack.<br>
> - Raising all segments of a disk image causes waste of physical memory, so in the process of initializing the virtual address space of the modified pintos, instead of allocating physical memory, only information to be loaded through vm_entry per virtual page.<br>

> Remove the part that mounts memory in the process virtual address space<br>

> **struct vm_entry \*vm_ent = (struct vm_entry \*)malloc(sizeof(struct vm_entry))**<br>
> - Create a vm_entry using the malloc function that allocates memory.<br>

> Add allocation of vm_entry structure, field value initialization, and hash table insertion using insert_vme().<br>

<br>

### To-do 12. Modify setup_stack(). (userprog/process.\*) <br>
```C
static bool
setup_stack (void **esp) 
{

...
  if (kpage != NULL) 
    {

          memset (vm_ent, 0, sizeof(struct vm_entry));
          vm_ent -> type = VM_ANON;
          vm_ent -> writable = true;
          vm_ent -> is_loaded = true;
          vm_ent -> vaddr = ((uint8_t *) PHYS_BASE) - PGSIZE;

          insert_vme(&thread_current()->vm, kpage->vme);
      }
...
}
```
> **setup_stack(){ }**<br>
> - The existing setup_stack() function that initializes the stack allocates a single page, sets the page table, and sets the stack pointer (esp).<br>
> - The setup_stack() function creates a vm_entry of a 4kb stack, initializes the field value of the created vm_entry, and modifies it to insert into the vm hash table.<br>

<br>

### To-do 13. Add address_check(). (userprog/syscall.\*) <br>
```C
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
```
> **Verify the validity of a user-provided pointer** <br>
> - **check if call is_user_vaddr(addr) is False** <br>
>   - addr == NULL : <br>
>   - is_user_vaddr(addr) : check if <br>

> - Use vm_entry to perform validation operations and modify them to return vm_entry.<br>

> **find_vme(addr)**<br>  
> - return the address using find_vme().<br>

<br>

### To-do 14. Add check_buffer(). (userprog/syscall.\*) <br>
```C
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
```
>**for (i=buf\; i <= buf + size\; i++){ }**<br>
> - It applies to vm_entries included in addresses up to buffer + size.<br>

> **vme = address_check(buf, esp);**<br>
> - The check_valid_buffer() function checks the user area of the address through the check_address() function and receives the vm_entry structure because the size from buffer to buffer + size entered as a factor may exceed the size of a page.<br>

> **if (vme->writable != true){ }**<br>
> - Checks whether vm_entry exists for that address and if the writable member of the vm_entry is true.<br>

<br>

### To-do 15. Modify syscall_handler(). (userprog/syscall.\*) <br>
```C
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
```
> Add address_check() and check_buffer approprietly before calling system call.<br>

<br>

### To-do 16. Add load_file(). (vm/page.\*) <br>
```C
bool load_file (void *kaddr, struct vm_entry *vm_entry){
    if (kaddr == NULL || vm_entry == NULL || vm_entry -> type == VM_ANON){
        return false;
    }
    if (file_read_at(vm_entry->file, kaddr, vm_entry->read_bytes, vm_entry->offset) != (int) vm_entry->read_bytes){
        return false;
    }

    memset(kaddr + (vm_entry->read_bytes), 0, vm_entry->zero_bytes);
    return true;
}
```
> **load_file (void \*kaddr, struct vm_entry \*vm_entry){ }**<br>
> - load_file() is a function that loads pages existing on disk into physical memory.<br>

> **if (file_read_at(vm_entry->file, kaddr, vm_entry->read_bytes, vm_entry->offset) != (int) vm_entry->read_bytes){ }**<br>
> - It is necessary to implement a function that reads one page as kaddr as a file and offset in vme.<br>
> - Use the file_read_at() function or the file_read() + file_seek() function to read a file entered as a factor.<br>

> **memset(kaddr + (vm_entry->read_bytes), 0, vm_entry->zero_bytes)**<br>
> - If file couldn't be writed all 4KB, fill the rest with zero.<br>

<br>

### To-do 17. Add handle_mm_fault(). (userprog/process.\*) <br>
```C
bool handle_mm_fault(struct vm_entry *vme){
  struct page *pg;
  pg = alloc_page(PAL_USER);
  if(pg == NULL ||vme == NULL){
    return;
  }
  pg->vme = vme;

  bool success;

  switch (vme->type){
    case VM_BIN:
      success = load_file(pg->kaddr, vme);
      if(!success){
        free_page(pg->kaddr);
        return false;
      }
      break;
    case VM_FILE:
      success = load_file(pg->kaddr, vme);
      if(!success){
        free_page(pg->kaddr);
        return false;
      }
          
    case VM_ANON:
      swap_in(vme->swap_slot, pg->kaddr);
      break;
  }

  if(!install_page (vme->vaddr, pg->kaddr, vme->writable)){
    free_page(pg->kaddr);
    return false;
  }
  vme->is_loaded = true;
  add_page_to_lru_list(pg);

  return true;
  
}
```
> **handle_mm_fault(struct vm_entry \*vme){ }**<br>
> - The handle_mm_fault() is a function called for handling when a page fault occurs.<br>
> - Assign a physical page when a page fault occurs.<br>
> - Returns the success or failure of the load in the bool data type.<br>

> **pg = alloc_page(PAL_USER)**<br>
> - Allocate physical memory to page.<br>

> **switch () { }**<br>
> - It is processed according to the type of vm_entry with a switch statement. For VM_BIN binary files, call load_file() and load it into physical memory.<br>

> **load_file()**<br>
> - to load files on disk onto physical pages.<br>

>**install_page()**<br>
> - to complete loading in physical memory, the virtual address and physical address are mapped to a page table.<br>

<br>

### To-do 18. Modify page_fault(). (userprog/exeption.\*) <br>
```C
static void
page_fault (struct intr_frame *f) 
{

 ...
 
  struct vm_entry *vme;
  if (not_present != true){
    exit(-1);
  }
  vme = find_vme(fault_addr);
  if (vme == NULL){
    if (!verify_stack((int32_t) fault_addr, f->esp)){
      exit(-1);
    }
    expand_stack(fault_addr);
    return ;
  }
  if (!handle_mm_fault(vme)){
    exit(-1);
  }
}

...

```
> **page_fault (struct intr_frame \*f) { }**<br>
> - The existing page_fault() process unconditionally generates a "segmentation fault" when an error occurs after permission and address validation, and kills(-1) to terminate it.<br>
> - Delete the command to kill(-1) to terminate.<br>

> **if (!verify_stack((int32_t) fault_addr, f->esp)) { }**<br>
> - validate fault_addr using verify_stack().

> **if (!handle_mm_fault(vme)) { }**<br>
> - Calls the page fault handler function handle_mm_fault().<br>

<br>

## To-do - Memory Mapped File
- **Define mmap_file structure.** (vm/page.\*) <br>
     : Store the information of the mapped file. <br>
- **Add mmap().** (userprog/syscall.\*) <br>
     : Load file data into memory by request paging. <br>
- **Add do_munmap().** (userprog/process.\*) <br>
     : Remove an entry from the page table.<br>
     : Write memory contents to disk.<br>
- **Add munmap().** (userprog/syscall.\*) <br>
     : Remove an entry from the page table. <br>
- **Modify handle_mm_fault().** (userprog/process.\*) <br>
- **Modify process_exit().** (userprog/process.\*) <br>
