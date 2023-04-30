# Project 2 - Argument Passing
> Extend process_excute() to support passing arguments to new processes.

<br>

## Files to modify
- userprog/process.*

<br>

## To-do
- **Modify process_excution() function.** (userprog/process.c) <br>
     : 

     
- **Modify start_process() function.** (userprog/process.c) <br>
     : 

     
- **Add argument_user_stack() function.** (userprog/process.\*) <br>
     : 
     

<br>
<br>

## Project Description

### To-do 1. Modify process_execution() function. (userprog/process.c) <br>

``` C
process_execute (const char *file_name) 
{
  char *fn_copy;
  tid_t tid;

  /* Make a copy of FILE_NAME.
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
  if (fn_copy == NULL)
    return TID_ERROR;
  strlcpy (fn_copy, file_name, PGSIZE);

  char *save_ptr;
  char *token = strtok_r(file_name, " ", &save_ptr);

  /* Create a new thread to execute FILE_NAME. */
  tid = thread_create (token, PRI_DEFAULT, start_process, fn_copy);
  if (tid == TID_ERROR)
    palloc_free_page (fn_copy);
  return tid;
}
```
> **Adds a file with a function defined for floating point operations within pintOS.** <br>
> - Created according to the instructions in the following link.
>    https://web.stanford.edu/class/cs140/projects/pintos/pintos_7.html#SEC131

<br>

#### - thread.h
``` C
...

#include "threads/fixed-point.h"

...
```
> **Include the fixed-point.h file in thread.c.** <br>

<br>

### To-do 2. Modify thread structure. (threads/thread.h)
#### - thread.h
``` C
struct thread
{
    ...

    int nice;
    int recent_cpu;

    ...
}
```
> **Add new int type fields for advanced scheduling.** <br>

<br>

### To-do 3. Add mlfqs_update_priority() function.  (threads/thread.\*)
#### - thread.c
```C
void mlfqs_update_priority (struct thread *t){
// priority = PRI_MAX - (recent_cpu / 4) - (nice * 2)
  if (t != idle_thread){
    int a = (int) PRI_MAX;
    int b = div_mixed(t->recent_cpu, -4);
    int c = t->nice * (-2);
    int d = a+c;
    int result = fixed_point_to_int(add_mixed(b, d));

    if (result > PRI_MAX){
      result = PRI_MAX;
    }else if (result < PRI_MIN){
      result = PRI_MIN;
    }

    t->priority = result;
  }
}
```
> **Calculate to change the priority to PRI_MAX - (recent_cpu / 4) - (nice * 2)** <br>
> - a and c are int type variables.
> - recent_cpu is float type and -4 is int type. So, div_mixed() function is used to calculate.
> - b is float type and d is int type. So, add_mixed() function is used to calculate.
> - priority should be int type. So, fixed_point_to_int() is used to convert float type to int type.

<br>
