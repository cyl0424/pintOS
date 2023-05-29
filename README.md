# Project 3 - Virtual Memory
> Demand paging <br>
> Stack growth <br>
> Memory Mapped Files <br>

<br>

## Files to modify
pintos/src/userprog/syscall.* <br>
pintos/src/userprog/process.* <br>
pintos/src/vm/page.* <br>

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

<br>
<br>

## Project Description

### To-do 1.Define mmap_file structure. (vm/page.\*) <br>

``` C
struct mmap_file {
    int mapid;
    struct file *file;
    struct list_elem elem;
    struct list vme_list;
};
```
> **mapid:** An integer storing the mapping's ID. <br>
> **file:** A pointer to a struct representing the associated file. <br>
> **elem:** A struct used for linking "mmap_file" instances in a list. <br>
> **vme_list:** A list holding "vme" structs representing virtual memory entries. <br>

<br>

### To-do 2. Add mmap(). (userprog/syscall.\*) <br>

```C
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
```
> **The `mmap` function creates a memory mapping for a file, associating it with a unique `mapid` and setting up the necessary virtual memory entries.** <br>
> 1. It checks if a virtual memory entry already exists at the specified address `addr`. If so, it returns -1 to indicate an error.
>
> 2. It verifies that the address is valid by checking if it is properly aligned, non-null, and within the user's address space. If any of these conditions fail, it returns -1.
>
> 3. It allocates memory for a new `mmap_file` struct and checks if the allocation was successful. If not, it returns -1.
>
> 4. It initializes the `vme_list` within the `mmap_file` struct and retrieves the current thread.
>
> 5. It obtains the file associated with the provided file descriptor `fd` using the `get_open_file` function. If the file retrieval fails, it returns -1.
>
> 6. It reopens the file to ensure it remains accessible even if the original file is closed.
>
> 7. It assigns a unique `mapid` to the `mmap_file` struct based on the next available `mapid` value in the current thread's context.
>
> 8. It adds the `mmap_file` struct to the `mmap_list` of the current thread.
>
> 9. It determines the length of the file and proceeds with memory mapping in page-sized chunks until the entire file is mapped.
>
> 10. For each page-sized chunk, it checks if a virtual memory entry already exists at the specified address `addr`. If so, it returns -1 to indicate an error.
>
> 11. It allocates memory for a new `vm_entry` struct and initializes it with relevant information such as the address, file properties, offset, and size.
>
> 12. It adds the `vm_entry` to the `vme_list` of the `mmap_file` struct and inserts it into the virtual memory map of the current thread.
>
> 13. It updates the remaining length, address, and offset for the next iteration.
>
> 14. Once all pages are mapped, it returns the `mapid` associated with the `mmap_file` struct.


<br>

### To-do 3. Add do_munmap(). (userprog/process.\*) <br>

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
> **The function takes an mapid as a parameter and is responsible for unmapping the memory associated with that mapid.** <br>
> 1. It iterates through the mmap_list of the current thread to find the mmap_file with the matching mapid.
>
> 2. If a matching mmap_file is found, it enters a nested loop to iterate through the vme_list of that mmap_file.
>
> 3. For each vm_entry in the vme_list, it performs the following actions:
> 
> - Retrieves the current thread and the vm_entry.
> - Checks if the vm_entry is loaded into physical memory and if its corresponding page in the page directory is dirty.
> - If the conditions are met, it writes the contents of the page back to the associated file using file_write_at.
> - It frees the page using free_page.
> - Marks the vm_entry as not loaded.
> - Removes the vm_entry from the vme_list and deletes it from the virtual memory map of the current thread using delete_vme.
> 4. After processing all vm_entry elements in the vme_list, it removes the mmap_file from the mmap_list of the current thread using list_remove.
>
> 5. Finally, it frees the memory occupied by the mmap_file using free.


<br>

### To-do 4. Add munmap(). (userprog/syscall.\*) <br>

```C
void munmap (int mapid){
  do_munmap(mapid);
}
```
> **A function for handling a Munmap system call, using the do_munmap function** <br>

<br>


### To-do 5. Modify handle_mm_fault(). (userprog/process.\*) <br>

```C
void vm_init (struct hash *vm){
    if (vm != NULL){
        hash_init(vm, vm_hash_func, vm_less_func, NULL);
    }
}
```
> **Reasons for using hash** <br>
> - vm_entries should be managed in a bundle so that they can be navigated.<br>

<br>

### To-do 6. Modify process_exit(). (userprog/process.\*) <br>

```C
void process_exit (void){
     ...
     int mapid;
     for (mapid = 1; mapid < cur->next_mapid; mapid++){
          do_munmap(mapid);
     }
     ...
}
```
> **Clean up and deallocate all vm entries when a process exits** <br>
>   - for(mapid = 1; mapid < cur->next_mapid; mapid++) <br>
>     : to handle all the memory mappings created by the process <br>
>      - do_munmap() <br>
>        : responsible for unmapping a memory mapping associated with the given mapid <br>
>          performs necessary cleanup and deallocation of resources associated with the mapping <br>
