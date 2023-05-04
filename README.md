# Project 2 - System Call
> Add system calls to provide services to users. <br>
> - Process related: halt, exit, exec, wait
> - File related: create, remove, open, filesize, read, write, seek, tell, close

<br>

## Files to modify
pintos/src/threads/thread.* <br>
pintos/src/userprog/syscall.* <br>
pintos/src/userprog/process.* <br>

<br>

## To-do
- **Implement system call handler.** (userprog/syscall.\*) <br>
     : make system call handler call system call <br>
       check validation of the pointers in the parameter <br>
       copy arguments on the user stack to the kernel <br>
       save return value of system call <br>
     
- **Add address_check() function.** (userprog/syscall.\*) <br>
     : detect invalidity of pointers and terminating process without harm to the kernel. <br>

- **Add system call halt().** (userprog/syscall.\*) <br>
     : shutdown pintos. <br>
     
- **Add system call exit().** (userprog/syscall.\*) <br>
     : exit pintos. <br>
     
- **Add system call exec().** (userprog/syscall.\*) <br>
     : create child process and execute program. <br>
     
- **Add system call wait().** (userprog/syscall.\*) <br>
     : wait for termination of child process. <br> 
     
- **Implement file descriptor table.** (userprog/syscall.\*) <br>
     : implement an array of pointers to struct file, used to access to files. <br>
     
- **Add system call create().** (userprog/syscall.\*) <br>
     : create file. <br>
     
- **Add system call remove().** (userprog/syscall.\*) <br>
     : remove file. <br>  

- **Add system call open().** (userprog/syscall.\*) <br>
     : open file. <br>
     
- **Add system call filesize().** (userprog/syscall.\*) <br>
     : return the size, in bytes, of the open file. <br>  

- **Add system call read().** (userprog/syscall.\*) <br>
     : read size bytes from the file open as fd into buffer. <br>
     
- **Add system call write().** (userprog/syscall.\*) <br>
     : write data from buffer to open file. <br>  
     
- **Add system call seek().** (userprog/syscall.\*) <br>
     : changes the next byte to be read or written in open file. <br>
     
- **Add system call tell().** (userprog/syscall.\*) <br>
     : stack the arguments on the user stack. <br> 

- **Add system call close().** (userprog/syscall.\*) <br>
     : stack the arguments on the user stack. <br>
     
- **Modify load() and process_exit().** <br>
     : deny writes to executing files. <br>
       
<br>
<br>

## Project Description

### To-do 1. Modify process_execution() function. (userprog/process.c) <br>

``` C
tid_t process_execute (const char *file_name) 
{
  char *fn_copy;
  tid_t tid;

  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
  if (fn_copy == NULL)
    return TID_ERROR;
  strlcpy (fn_copy, file_name, PGSIZE);
  
  /* Create a new thread to execute FILE_NAME. */
  /* Project 2 - Argument Passing */
  char *save_ptr;
  char *token = strtok_r(file_name, " ", &save_ptr);

  tid = thread_create (token, PRI_DEFAULT, start_process, fn_copy);
  if (tid == TID_ERROR)
    palloc_free_page (fn_copy);
  return tid;
}
```
> **Parse the string of 'file_name'** <br>
> - **char \*token** <br>
>   add a variable to store the actual file name, and initialize as the result of strtok_r() <br>
> - **strtok_r(file_name, " ", &save_ptr)** <br>
>   saparate a stiring into tokens by a certain delimeter <br>
    the first time the strtok_r() function is called, it returns a pointer to the first token in string.<br>         
<br>
                                          
> **Forward the first token to thread_create() function** <br>
> - **thread_create (token, PRI_DEFAULT, start_process, fn_copy)** <br>
>   because the new variable '\*token' now has a value of the first token of parsed string, pend it as the name of the new process. <br>

<br>

### To-do 2. Modify start_process() function. (userprog/process.c) <br>
``` C
static void
start_process (void *file_name_)
{
  char *file_name = file_name_;
  struct intr_frame if_;
  bool success;

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;

  char *argv[128];
  char *save_ptr, *slicing;
  char *tmp = file_name;
  int cnt = 0;

  while(true){
    slicing = strtok_r(tmp, " ", &save_ptr);
    argv[cnt] = slicing;
    tmp = strtok_r(NULL, " ", &save_ptr);
    cnt++;

    if (tmp == NULL){
      break;
    }
  }

  char *file = argv[0];
  success = load (file, &if_.eip, &if_.esp);

  if(success){
    argument_user_stack(argv, cnt, &if_.esp);
    hex_dump(if_.esp, if_.esp, PHYS_BASE- if_.esp, true);
  }
  
  /* If load failed, quit. */
  palloc_free_page (file_name);
  if (!success) 
    thread_exit ();

  /* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}
```
> **Parse the string of file_name** <br>
> - **char \*argv[128]** <br>
>   add an array to store the sliced tokens, that is, the arguments <br>
> - **char \*slicing** <br>
>   add a variable to store the actual file name <br>
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

