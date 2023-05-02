# Project 2 - Argument Passing
> Extend process_excute() to support passing arguments to new processes.

<br>

## Files to modify
- userprog/process.*

<br>

## To-do
- **Modify process_excution() function.** (userprog/process.c) <br>
     : parse the string of file_name and forward the first token as the name of the new process to thread_create() function.

     
- **Modify start_process() function.** (userprog/process.c) <br>
     : parse the string of file_name and save tokens in the user stack by calling argument_user_stack() function.

     
- **Add argument_user_stack() function.** (userprog/process.\*) <br>
     : stack the arguments on the user stack.
     

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
> - char \*token : add a variable to store the actual file name <br>
> - strtok_r(file_name, " ", &save_ptr) : saparate a stiring into tokens by a certain delimeter. <br>
                                          the first time the strtok_r() function is called, it returns a pointer to the first token in string.<br>         <br>
                                          
> **Forward the first token to thread_create() function** <br>
> - thread_create (token, PRI_DEFAULT, start_process, fn_copy) <br>
>         because the new variable '\*token' now has a value of the first token of parsed string, pend it as the name of the new process. <br>

<br>

### To-do 2. Modify start_process() function.** (userprog/process.c)
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
> - call argument_user_stack(argv, cnt, &if_.esp) <br>
> - call hex_dump(if_.esp, if_.esp, PHYS_BASE- if_.esp, true) <br>
<br>
<br>

### To-do 3. Add argument_user_stack() function.** (userprog/process.\*)
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
> **Calculate to change the priority to PRI_MAX - (recent_cpu / 4) - (nice \* 2)** <br>
> - a and c are int type variables.
> - recent_cpu is float type and -4 is int type. So, div_mixed() function is used to calculate.
> - b is float type and d is int type. So, add_mixed() function is used to calculate.
> - priority should be int type. So, fixed_point_to_int() is used to convert float type to int type.
<br>

#### - process.h

``` C
...

void argument_user_stack(char **agrv,int argc,void **esp);

...
```
<br>

> **Declare the argument_user_stack() function in process.h.** <br>

