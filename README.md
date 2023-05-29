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

## To-do - Swap In/Out
- **Add page struct.** (vm/page.\*) <br>
     : structure indicating one physical page allocated to a user. <br>
     
- **Add lru_list_init().** (vm/frame.\*) <br>
     : initialize lru list. <br>  

- **Add add_page_to_lru_list().** (vm/frame.\*) <br>
     : add user page at the end of the lru list. <br>  

- **Add del_page_from_lru_list().** (vm/frame.\*) <br>
     : delete user page from the lru list. <br>  

- **Add struct page \*alloc_page().** (vm/page.\*) <br>
     : allocate page. <br>  

- **Add free_page().** (vm/page.\*) <br>
     : call \_\_free_page(). <br> 

- **Add __free_page.** (vm/page.\*) <br>
     : eliminate lru list & deallocate memory space allocated to the page structure. <br>

- **Add swap_init()** (vm/swap.\*) <br>
     : initialize hash table when process starts. <br>

- **Add swap_in()** (vm/swap.\*) <br>
     : copy data stored in swap slot to virtual address kaddr. <br>

- **Add swap_out()** (vm/swap.\*) <br>
     : store the page kaddr is pointing. <br>
     
- **Add free_victim_page().** (vm/frame.\*) <br>
     : free up memory when there is no free physical page using clock algorithm. <br>
     
- **Add alloc_page().** (vm/page.\*) <br>
     : function to allocate physical address space. <br>

- **Replace allocation and deallocation functions.** <br>
     : replace palloc_get_page()->alloc_page(). <br>
       replace palloc_free_page()->free_page(). <br>

- **Modify handle_mm_fault().** (userprog/process.\*) <br>
     : Modify handle_mm_fault() to support swapping. <br>

<br>
<br>

## Project Description

### To-do 1. Add page struct. (vm/page.h) <br>

``` C
struct page {
    void *kaddr;
    struct vm_entry *vme;
    struct thread *t;
    struct list_elem lru;
};
```
> **void \*kaddr:** This is a pointer to the kernel virtual address associated with the page. It points to the starting address of the page in the kernel's virtual memory space.
>
> **struct vm_entry \*vme:** This is a pointer to the corresponding virtual memory entry (VME) for the page. It represents the mapping and properties of the page within the virtual memory space of a specific process.
>
> **struct thread \*t:** This is a pointer to the thread that owns or is associated with the page. It represents the thread of execution within the operating system.
>
> **struct list_elem lru:** This is a list element used to link the page into an LRU (Least Recently Used) list. It allows the page to be efficiently tracked and ordered based on its usage in memory management algorithms.

<br>

### To-do 2. Add lru_list_init(). (vm/frame.\*) <br>

```C
void lru_list_init(void){
    list_init(&lru_list);
    lock_init(&lru_list_lock);
    lru_clock = NULL;
}
```
> **The function initializes the LRU (Least Recently Used) list.**<br>
> 1. It initializes an empty linked list called lru_list using the list_init function. This list is used to maintain the order of pages based on their usage.
>
> 2. It initializes a lock called lru_list_lock using the lock_init function. This lock is used to synchronize access to the LRU list.
> 
> 3. It sets the lru_clock pointer to NULL. This pointer is used as a reference to the current position in the LRU list during clock sweep algorithms.

<br>


### To-do 3. Add add_page_to_lru_list(). (vm/frame.\*) <br>
```C
void add_page_to_lru_list(struct page *page){
    lock_acquire(&lru_list_lock);
    list_push_back(&lru_list, &page->lru);
    lock_release(&lru_list_lock);
}
```
> **The function adds a page to the LRU (Least Recently Used) list.**<br>
> 1. It acquires the lru_list_lock to ensure exclusive access to the LRU list.
>
> 2. It pushes the given page to the back of the lru_list using the list_push_back function. This action inserts the page at the end of the list, indicating that it is the most recently used page.
>
> 3. It releases the lru_list_lock to allow other threads to access the LRU list.

<br>


### To-do 4. Add del_page_from_lru_list(). (vm/frame.\*) <br>
```C
void del_page_from_lru_list (struct page *page){
    if(&page->lru == lru_clock){
        lru_clock = list_next(lru_clock);
    }
    list_remove(&page->lru);
}
```
> **The function removes a page from the LRU (Least Recently Used) list.**<br>
> 1. It checks if the page's lru pointer is equal to the lru_clock. This comparison is used to determine if the page being removed is currently pointed to by the lru_clock.
>
> 2. If the condition is true (i.e., the page being removed is the one pointed to by the lru_clock), the lru_clock is updated to point to the next element in the list using list_next. This step ensures that the lru_clock continues to track the correct position in the LRU list.
>
> 3. Regardless of the condition, the function removes the page from the LRU list using list_remove. This action detaches the page from the list without freeing the memory associated with it.

<br>

### To-do 5. Add struct page \*alloc_page(). (vm/page.\*) <br>
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

### To-do 9. Add vm hash table structure in thread structure. (threads/thread.\* , userprog/process.\*) <br>
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
> - Add hash struct 'vm' to structure 'thread'.<br>

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
### To-do 13. Replace allocation and deallocation functions <br>

```C
static bool
setup_stack (void **esp) 
{
  struct page *kpage;
  bool success = false;

  struct vm_entry *vm_ent;
  vm_ent = (struct vm_entry *)malloc(sizeof(struct vm_entry));
  if (vm_ent == NULL){
    return success;
  }
  
  kpage = alloc_page(PAL_USER | PAL_ZERO);
  if (kpage != NULL) 
    {
      kpage->vme = vm_ent;
      add_page_to_lru_list(kpage);

      success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage->kaddr, true);

      if (success){
        *esp = PHYS_BASE;

          memset (vm_ent, 0, sizeof(struct vm_entry));
          vm_ent -> type = VM_ANON;
          vm_ent -> writable = true;
          vm_ent -> is_loaded = true;
          vm_ent -> vaddr = ((uint8_t *) PHYS_BASE) - PGSIZE;

          insert_vme(&thread_current()->vm, kpage->vme);
      }
      else
        free_page (kpage->kaddr);
        free(vm_ent);
    }

  return success;
}
```

```C
void do_munmap(int mapid){
  struct list_elem *e;
  struct list_elem *e2;
  struct list_elem *next_e2;

  for (e = list_begin(&thread_current()->mmap_list); e != list_end(&thread_current()->mmap_list); e = list_next(e)) {
    struct mmap_file *f = list_entry(e, struct mmap_file, elem);
    if (f->mapid == mapid){

      for (e2 = list_begin(&f->vme_list); e2 != list_end(&f->vme_list);) {
        struct thread *cur = thread_current();
        struct vm_entry *vme = list_entry(e2, struct vm_entry, mmap_elem);

        if (vme->is_loaded && pagedir_is_dirty(&cur->pagedir, vme->vaddr)) {
          file_write_at(vme->file, vme->vaddr, vme->read_bytes, vme->offset);
          free_page(vme->vaddr);
        }

        vme->is_loaded = false;
        e2 = list_remove(e2);
        delete_vme(&cur->vm, vme);
      }

      list_remove(&f->elem);
      free(f);

    }
    
  }
}
```

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
> **setup_stack()**<br>
> - replace palloc_get_page() with alloc_page(). <br>
> - replace palloc_free_page() with free_page(). <br>
> **do_munmap()**<br>
> - replace palloc_get_page() with alloc_page(). <br>
> - replace palloc_free_page() with free_page(). <br>
> **handle_mm_fault()**<br>
> - replace palloc_get_page() with alloc_page(). <br>
> - replace palloc_free_page() with free_page(). <br>
<br>

### To-do 14. Modify handle_mm_fault(). (userprog/process.\*)
```C
bool handle_mm_fault(struct vm_entry *vme){ 
...
  case VM_ANON:
      swap_in(vme->swap_slot, pg->kaddr);
      break;
...
}
```
> **Modify handle_mm_fault() to support swapping**<br>
> - swap_in(vme->swap_slot, pg->kaddr)<br>
>   : If a vm entry's type is VM_ANON, call swap_in(). <br>

<br>
