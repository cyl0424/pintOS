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

### To-do 6. free_page(). (vm/page.\*) <br>
```C
void free_page (void *kaddr){
    struct list_elem *e;
    struct page *page;
    bool find = false;

    lock_acquire(&lru_list_lock);
    for(e = list_begin(&lru_list); e != list_end(&lru_list); e = list_next(e)){
        page = list_entry(e, struct page, lru);
        if(page->kaddr == kaddr){
            find = true;
            break;
        }
    }
    if (find == true && page!= NULL){
        __free_page(page);
    }

    lock_release(&lru_list_lock);
}
```

> **The function frees a page of memory associated with the given kernel virtual address (kaddr).** <br>
> 1. It declares variables, including a list element pointer e, a page pointer page, and a boolean variable find to track if the page is found in the LRU list.
>
> 2. It acquires the lru_list_lock to ensure exclusive access to the LRU list.
>
> 3. It iterates through the LRU list using a for loop, checking each page's kernel virtual address (page->kaddr) against the provided kaddr. If a matching page is found, it sets find to true, breaks out of the loop, and stores the found page in the page variable.
>
> 4. If a matching page is found (find == true) and the page is not NULL, it proceeds to free the page by calling \_\_free_page(page). This step releases the memory occupied by the page, removes it from the LRU list, and performs other cleanup operations.
>
> 5. It releases the lru_list_lock to allow other threads to access the LRU list.

<br>

### To-do 7. \_\_free_page(). (vm/page.\*) <br>
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

### To-do 8. Add swap_init(). (vm/swap.\*) <br>
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


### To-do 9. Add swap_in(). (vm/swap.\*) <br>
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

### To-do 10. Add swap_out(). (vm/swap.\*) <br>
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

### To-do 11. Add free_victim_page(). (vm/page.\*) <br>
```C
void free_victim_page(enum palloc_flags flag){
    struct page *page;
    struct page *victim;

    if(list_empty(&lru_list)){
        lru_clock = NULL;
    }
    else if(lru_clock==NULL || lru_clock==list_end(&lru_list)){
        lru_clock = list_begin(&lru_list);
    }else if (list_next(lru_clock)==list_end(&lru_list)){
        list_begin(&lru_list);
    }    
    else{
        lru_clock = list_next(lru_clock);
    }

    page = list_entry(lru_clock, struct page, lru);

    while(page->vme->pinned_flag || pagedir_is_accessed(page->t->pagedir, page->vme->vaddr)){
        pagedir_is_accessed(page->t->pagedir, page->vme->vaddr);
        if(list_empty(&lru_list)){
        lru_clock = NULL;
        }
        else if(lru_clock==NULL || lru_clock==list_end(&lru_list)){
            lru_clock = list_begin(&lru_list);
        }else{
            lru_clock = list_next(lru_clock);
        }

        page = list_entry(lru_clock, struct page, lru);
    }

    victim = page;
    uint32_t victim_pgd = victim->t->pagedir;
    struct vm_entry *victim_vaddr = victim->vme->vaddr;

    switch (victim
    ->vme->type)
    {
    case VM_BIN:
        if(pagedir_is_dirty(victim_pgd, victim_vaddr)){
            victim->vme->swap_slot = swap_out(victim->kaddr);
            victim->vme->type = VM_ANON;
        }
        break;
    case VM_FILE:
        if(pagedir_is_dirty(victim_pgd, victim_vaddr)){
            file_write_at(victim->vme->file, victim->vme->vaddr, victim->vme->read_bytes, victim->vme->offset);
        }
        break;
    case VM_ANON:
        victim->vme->swap_slot = swap_out(victim->kaddr);
        break;
    default:
        break;
    }
    victim->vme->is_loaded = false;
    __free_page(victim);
}
```
> **Check conditions:**<br>
> - if(list_empty(&lru_list)):<br>
>   lru_clock is set to null<br>
> - else if(lru_clock==NULL || lru_clock==list_end(&lru_list)):<br>
>   reset to the beginning of the list<br>
> - else if (list_next(lru_clock)==list_end(&lru_list)):<br>
>   reset to the beginning of the list<br>
> - else:<br>
>   moved to the next position<br>

> **page = list_entry(lru_clock, struct page, lru)**<br>
> - page pointer is assigned the value of the current page indicated by lru_clock<br>

> **while(page->vme->pinned_flag || pagedir_is_accessed(page->t->pagedir, page->vme->vaddr))**<br>
>   :enters a loop that continues until a non-pinned and non-accessed page is found
>   - If a page is pinned or has been accessed, the clock hand is moved to the next position in the LRU list and the page pointer is updated accordingly.<br>
>   - Once a victim page is selected, its address is stored in the victim pointer, and its associated page directory and virtual address are stored in victim_pgd and victim_vaddr, respectively.<br>

> **switch (victim->vme->type)**<br>
>   :to handle different types of virtual memory entries (vm_entry) associated with the victim page. Depending on the type, different eviction operations are performed
>   - VM_BIN
>     : if the page has been modified (dirty), it is swapped out to a swap slot using swap_out, and the vm_entry type is changed to VM_ANON.<br>
>   - VM_FILE
>     : if the page has been modified (dirty), its content is written back to the file using file_write_at.<br>
>   - VM_ANON
>     :  the page is swapped out to a swap slot using swap_out.

>**victim->vme->is_loaded = false**<br>
>  :the page is no longer in memory

>**Call __free_page(victim)**<br>
>  :the victim page is freed using the \_\_free_page function<br>
<br>

### To-do 12. Replace allocation and deallocation functions. <br>

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

### To-do 13. Modify handle_mm_fault(). (userprog/process.\*)
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
