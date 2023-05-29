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
code file
```
> **Type of vm_entry** <br>

<br>

### To-do 2. Add mmap(). (userprog/syscall.\*) <br>

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

### To-do 3. Add do_munmap(). (userprog/process.\*) <br>

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

### To-do 4. Add munmap(). (userprog/syscall.\*) <br>

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

### To-do 5. Modify process_exit(). (userprog/process.\*) <br>

```C
void vm_init (struct hash *vm){
    if (vm != NULL){
        hash_init(vm, vm_hash_func, vm_less_func, NULL);
    }
}
```
> **Reasons for using hash** <br>
> - vm_entries should be managed in a bundle so that they can be navigated.<br>
