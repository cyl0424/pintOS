# Project 3 - Virtual Memory
> Demand paging <br>
> Stack growth <br>
> Memory Mapped Files <br>

<br>

## Files to modify
pintos/src/userprog/process.* <br>
pintos/src/userprog/exception.* <br>

<br>

## To-do - Stack Growth
- **Add expand_stack().** (userprog/process.\*) <br>
     : initialize hash table. <br>  
     
- **Modify page_fault()** (userprog/exception.c) <br>
     : contain the page table index and page offset. <br>
     
<br>
<br>

## Project Description


### To-do 1. Add expand_stack(). (userprog/process.\*) <br>

```C
bool
expand_stack (void *addr)
{
  struct page *kpage = alloc_page (PAL_USER | PAL_ZERO);

  struct vm_entry *vme = malloc(sizeof(struct vm_entry));
  if (vme == NULL){
    return false;
  }

  if (kpage != NULL)
    {
      kpage->vme = vme;
      void *upage = pg_round_down(addr);

      memset(kpage->vme, 0, sizeof (struct vm_entry));

      kpage->vme->type = VM_ANON;
      kpage->vme->vaddr = upage;
      kpage->vme->writable = true;
      kpage->vme->is_loaded = true;

      insert_vme(&thread_current()->vm, kpage->vme);

      if (!install_page(vme->vaddr, kpage->kaddr, true)){
        free_page(kpage->kaddr);
        free(vme);
        return false;
      }

      add_page_to_lru_list(kpage);
      return true;

    }
}
```
> **The expand_stack function expands the stack region by allocating a new page, creating a corresponding virtual memory entry, inserting it into the virtual memory map, and installing the page in the page table.** <br>
> 1. It allocates a new page of memory for the stack using the alloc_page function, with the USER_PAL flag indicating it is a user page and the PAL_ZERO flag to initialize the page with zeroes.
>
> 2. It allocates memory for a new vm_entry struct using malloc. If the allocation fails and the vme is NULL, the function returns false.
>
> 3. If the page allocation (kpage) is successful, the function proceeds with the following actions:

>   - Associates the vme with the allocated page by assigning it to the kpage->vme field.
>   - Rounds down the provided addr to the nearest page boundary to obtain the user page address (upage).
>   - Initializes the vme with relevant information such as the type (VM_ANON), virtual address (upage), writability, and loaded status.
>   - Inserts the vme into the virtual memory map of the current thread using insert_vme.
>   - Attempts to install the page in the page table using install_page. If the installation fails, it frees the allocated page and vme, and returns false.
>
> 4. If the page installation is successful, the function adds the allocated page (kpage) to the LRU (Least Recently Used) list using add_page_to_lru_list.
>
> 5. Finally, the function returns true to indicate successful expansion of the stack.

<br>

### To-do 2. Modify page_fault() (userprog/exception.c) <br>

``` C
...
  if (vme == NULL){
    printf("no vme\n");
    if (!verify_stack((int32_t) fault_addr, f->esp)){
      printf("no stack\n");
      exit(-1);
    }
    expand_stack(fault_addr);
    return ;
  }
...
```
> **If the stack does not contain the corresponding part of the address, use expand_stack() to include it.** <br>                
    
<br>
