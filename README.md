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

- **Add alloc_page().** (vm/page.\*) <br>
     : function to allocate physical address space. <br>

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
struct page *alloc_page (enum palloc_flags flag){
    lock_acquire(&lru_list_lock);
    int *kpage;
    kpage = palloc_get_page(flag);
    while (kpage == NULL){
        free_victim_page(flag);
        kpage = palloc_get_page(flag);
    }

    struct page *page;
    page = (struct page *)malloc(sizeof(struct page));
    page->kaddr = kpage;
    page->t = thread_current();

    add_page_to_lru_list(page);
    lock_release(&lru_list_lock);

    return page;
}
```
> **The function allocates a page of memory and returns a pointer to the allocated page struct.** <br>
> 1. It acquires the lru_list_lock to ensure exclusive access to the LRU list and other relevant data structures.
>
> 2. It declares an integer pointer kpage to hold the kernel virtual address of the allocated page.
>
> 3. It calls palloc_get_page with the specified flag to attempt to allocate a page. If the allocation is unsuccessful (i.e., kpage is NULL), it proceeds with the following actions:
>
> a. It calls free_victim_page to free up a page from memory, considering the specified allocation flag.
>
> b. It attempts to allocate a page again by calling palloc_get_page once more and assigns the result to kpage.
>
> 4. It declares a page pointer page and allocates memory for the page struct using malloc. The size of the allocation is determined by sizeof(struct page).
>
> 5. It assigns the kpage value to page->kaddr, representing the kernel virtual address of the allocated page.
> 
> 6. It assigns the current thread (thread_current()) to page->t, indicating the thread that owns or is associated with the allocated page.
> 
> 7. It adds the page to the LRU list by calling add_page_to_lru_list(page). This ensures proper tracking and ordering of the page based on its usage.
>
> 8. It releases the lru_list_lock to allow other threads to access the LRU list and related data structures.
>
> 9. Finally, it returns the page pointer, providing the caller with a reference to the allocated page.

<br>

### To-do 6. __free_page(). (vm/page.\*) <br>
```C
void __free_page (struct page *p){
    del_page_from_lru_list(p);
    pagedir_clear_page(p->t->pagedir, pg_round_down(p->vme->vaddr));
    palloc_free_page(p->kaddr);
    free(p);
}
```

> **The function frees a page of memory and performs cleanup operations.** <br>
> 1. It removes the page from the LRU (Least Recently Used) list by calling del_page_from_lru_list(p). This action detaches the page from the LRU order.
>
> 2. It clears the page entry in the page directory (pagedir_clear_page) associated with the thread (p->t) that owns or is associated with the page. This step removes the mapping between the virtual address (p->vme->vaddr) and the physical frame of the page.
>
> 3. It frees the physical frame of the page by calling palloc_free_page(p->frame). This action releases the allocated memory of the page.
> 
> 4. It frees the memory occupied by the page struct itself by calling free(p). This step ensures that the memory used by the struct is deallocated and can be reused.

<br>

### To-do 7. Add swap_init(). (vm/swap.\*) <br>
```C
struct lock swap_lock;
struct bitmap *swap_bitmap;

void swap_init(size_t size){
    lock_init(&swap_lock);
    swap_bitmap = bitmap_create(size);
}
```
> **The swap_init function initializes the swap space by initializing a lock and creating a swap bitmap to manage the swap slots.**<br>
> 1. It initializes a lock called swap_lock using lock_init. This lock is likely used to synchronize access to the swap space.
>
> 2. It creates a swap bitmap using bitmap_create with the specified size. The swap bitmap is used to track the availability of swap slots in the swap space.

<br>


### To-do 8. Add swap_in(). (vm/swap.\*) <br>
```C
void swap_in(size_t index, void *kaddr){
    lock_acquire(&swap_lock);
    struct block *swap_block = block_get_role(BLOCK_SWAP);
    if(bitmap_test(swap_bitmap, index)){
        int i;
        for(i=0; i<PGSIZE/BLOCK_SECTOR_SIZE; i++){
            block_read(swap_block, PGSIZE/BLOCK_SECTOR_SIZE*index + i, BLOCK_SECTOR_SIZE*i+kaddr);
        }
        bitmap_reset(swap_bitmap, index);
    }
    lock_release(&swap_lock);
}
```
> **The function swaps in a page from the swap space to the provided kernel virtual address (kaddr).**
> 1. It acquires the swap_lock to ensure exclusive access to the swap space.
>
> 2. It retrieves the swap_block associated with the swap space.
> 
> 3. If the specified index in the swap_bitmap indicates that the swap slot is occupied (i.e., the bit is set), it proceeds with the following actions:
>
> a. It iterates through each sector within the page size (PGSIZE) and reads the corresponding data from the swap_block into the kernel virtual address (kaddr).
>
> b. It resets the bit for the index in the swap_bitmap, indicating that the swap slot is now available.
>
> 4. Finally, it releases the swap_lock to allow other threads to access the swap space.

<br>

### To-do 9. Add swap_out(). (vm/swap.\*) <br>
```C
size_t swap_out(void *kaddr){
    lock_acquire(&swap_lock);
    struct block *swap_block = block_get_role(BLOCK_SWAP);

    size_t swap_index = bitmap_scan(swap_bitmap, 0, 1, false);

    if(BITMAP_ERROR != swap_index){
        int i;
        for(i=0; i<PGSIZE/BLOCK_SECTOR_SIZE; i++){
            block_write(swap_block, PGSIZE/BLOCK_SECTOR_SIZE*swap_index + i, BLOCK_SECTOR_SIZE*i+kaddr);
        }
        bitmap_set(swap_bitmap, swap_index, true);
    }
    lock_release(&swap_lock);
    return swap_index;
}
```
> **The function swaps out a page from the provided kernel virtual address (kaddr) to the swap space.**
> 1. It acquires the swap_lock to ensure exclusive access to the swap space.
>
> 2. It retrieves the swap_block associated with the swap space.
>
> 3. It scans the swap_bitmap to find an available swap slot index using bitmap_scan. The index is stored in the swap_index variable.
> 
> 4. If a valid swap_index is found (not equal to BITMAP_ERROR), it proceeds with the following actions:
>
> a. It iterates through each sector within the page size (PGSIZE) and writes the corresponding data from the kernel virtual address (kaddr) to the respective sectors in the swap_block.
>
> b. It sets the bit for the swap_index in the swap_bitmap, indicating that the swap slot is now occupied.
>
> 5. Finally, it releases the swap_lock, allowing other threads to access the swap space, and returns the swap_index.

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
### To-do 13. Replace allocation and deallocation functions. <br>

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
