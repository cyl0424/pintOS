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
