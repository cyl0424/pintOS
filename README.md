# Project 1 - Advanced Scheduler
> Implement 4.4 BSD scheduler MLFQ like scheduler without the queues.

<br>

## Files to modify
- threads/thread.*
- threads/synch.c
- devices/timer.c

<br>

## File to add
- threads/fixed-point.h

<br>

## To-do
- **Add fixed-point.h File.** (threads/fixed-point.h) <br>
     : Adds a file with a function defined for floating point operations within pintOS.

     
- **Modify thread structure.** (threads/thread.h) <br>
     : Add new int type fields for advanced scheduling.

     
- **Add mlfqs_update_priority() function.** (threads/thread.\*) <br>
     : Calculate to change the priority to PRI_MAX - (recent_cpu / 4) - (nice * 2).

     
- **Add mlfqs_update_recent_cpu() function.** (threads/thread.\*) <br>
     : Calculate to change the recent_cpu to (2*load_avg)/(2*load_avg + 1) * recent_cpu + nice.

     
- **Add mlfqs_update_load_avg() function.**  (threads/thread.\*) <br>
     : Calculate to change load_avg to (59/60)*load_avg + (1/60)*ready_threads.

     
- **Add mlfqs_incre_recent_cpu() function.** (threads/thread.\*) <br>
     : Calculate to add 1 to the recent_cpu.
     
     
- **Add mlfqs_update_priority_all() function.** (threads/thread.\*) <br>
     : Run mlfqs_update_priority() for updating all_list's priority.


- **Add mlfqs_update_priority_recent_cpu_all() function.** (threads/thread.\*) <br>
     : Run mlfqs_update_priority() for updating all_list's recent_cpu.
     
     
- **Modify thread_start() function.** (threads/thread.c) <br>
     : Declare new int type global variable 'load_avg' in thread.c and initialize it in thread_start().
     
     
- **Modify init_thread() function.** (threads/thread.c) <br>
     : Initialize the nice and recent_cpu.
     
     
- **Modify thread_set_priority() function.** (threads/thread.c) <br>
     : Modify the function in case it runs in mfqs.
     
     
- **Modify thread_set_nice() function.** (threads/thread.c) <br>
     : Implement functions for setting nice.
     
     
- **Modify thread_get_nice() function.** (threads/thread.c) <br>
     : Implement functions for getting nice.
    
    
- **Modify thread_get_load_avg() function.** (threads/thread.c) <br>
     : Implement functions for getting load_avg.
     
    
- **Modify thread_get_recent_cpu() function.** (threads/thread.c) <br>
     : Implement functions for getting recent_cpu.
     
     
- **Modify lock_aquire() function.** (threads/synch.c) <br>
     : Modify the function not to run donate_priority when mlfqs.
     
    
- **Modify lock_release() function.** (threads/synch.c) <br>
     : Modify the function not to run donate_priority when mlfqs.
     
     
- **Modify timer_interrupt() function.** (devices/timer.c) <br>
     : Calculate recent_cpu, priority, load_avg as tick grows.
     

<br>
<br>

## Project Description

### To-do 1. Add fixed-point.h File. (threads/fixed-point.h, threads/thread.c)
#### - fixed-point.h

``` C
#include <stdint.h>

#define F (1 << 14)
#define INT_MAX ((1 << 31) - 1)
#define INT_MIN (-(1 << 31))

int int_to_fixed_point(int n);
int fixed_point_to_int(int x);
int fixed_point_to_int_round(int x);

int add_fixed_point(int x, int y);
int add_mixed(int x, int n);

int sub_fixed_point(int x, int y);
int sub_mixed(int x, int n);

int mult_fixed_point(int x, int y);
int mult_mixed(int x, int y);

int div_fixed_point(int x, int y);
int div_mixed(int x, int n);


int int_to_fixed_point(int n) {
    return n * F;
}

int fixed_point_to_int(int x) {
    return x / F;
}

int fixed_point_to_int_round(int x) {
    if (x >= 0) {
        return (x + F / 2) / F;
    } else {
        return (x - F / 2) / F;
    }
}

int add_fixed_point(int x, int y) {
    return x + y;
}

int add_mixed(int x, int n) {
    return x + n * F;
}

int sub_fixed_point(int x, int y) {
    return x - y;
}

int sub_mixed(int x, int n) {
    return x - n * F;
}

int mult_fixed_point(int x, int y) {
    return ((int64_t)x) * y / F;
}

int mult_mixed(int x, int n) {
    return x * n;
}

int div_fixed_point(int x, int y) {
    return ((int64_t)x) * F / y;
}

int div_mixed(int x, int n) {
    return x / n;
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

#### To-do 4. Add mlfqs_update_recent_cpu() function.  (threads/thread.c)
#### - thread.c
```C
void mlfqs_update_recent_cpu (struct thread *t){
// recent_cpu = (2*load_avg)/(2*load_avg + 1) * recent_cpu + nice
  if (t != idle_thread){
    int a = mult_mixed(load_avg, 2);
    int b = add_mixed(a, 1);
    int a_b = div_fixed_point(a, b);
    int c = mult_fixed_point(a_b, t->recent_cpu);
    
    int result = add_mixed(c, t->nice);

    if ((result >> 31) == (-1) >> 31){
      result = 0;
    }

    t->recent_cpu = result;
  }
}
```
> **Calculate to change the recent_cpu to (2load_avg)/(2load_avg + 1) * recent_cpu + nice.** <br>
> - load_avg is float type and 2 is int type. So mult_mixed() is used to calculate.
> - a is float type and 1 is int type. So, add_mixed() function is used to calculate.
> - a and b are float type variables. So, div_fixed_point() function is used to calculate.
> - a_b and recent_cpu are float type variables. So. mult_fixed_point is used to calculate.
> - c is float type and nice is int type. So, add_mixed() is used to calculate.
> - recent_cpu should not be negative. So, set the value of result to 0, if result is negative.

<br>

#### To-do 5. Add mlfqs_update_load_avg() function.  (threads/thread.c)
#### - thread.c
```C
void mlfqs_update_load_avg (void){
// load_avg = (59/60)*load_avg + (1/60)*ready_threads
  int ready_threads = (int)list_size(&ready_list);
  if (thread_current() != idle_thread){
    ready_threads++;
  }

  int a = div_fixed_point(int_to_fixed_point(59), int_to_fixed_point(60));
  int b = mult_fixed_point(a, load_avg);
  int c = div_fixed_point(int_to_fixed_point(1), int_to_fixed_point(60));
  int d = mult_mixed(c, ready_threads);
  int result = add_fixed_point(b, d);

  load_avg = result;
}
```
> **Calculate to change load_avg to (59/60)\*load_avg + (1/60)\*ready_threads.** <br>
> - ready_threads are the size of ready_list. But, if thread_current is not idle_thread 1 should be added. 
> - To calculate 59/60 and 1/60, convert the value type to float type and use div_fixed_point() function.
> - a and load_avg are float type variables. So, mult_fixed_point() function to calculate.
> - c is float type and ready_threads is int type. So, mult_mixed() function to calculate.
> - b and d are float type variables. So, add_fixed_point() function to calculate.

<br>

#### To-do 6. Add mlfqs_incre_recent_cpu() function.  (threads/thread.c)
#### - thread.c
``` C
void mlfqs_incre_recent_cpu (void){
  struct thread *cur = thread_current();

  if (cur != idle_thread){
    cur->recent_cpu = add_mixed(cur->recent_cpu, 1);
  }
}
```
> **Calculate to add 1 to the recent_cpu.** <br>
> - If thread_current is not idle_thread, recent_cpu increase by 1.
> - recent_cpu is float type and 1 is int type. So, add_mixed is used to calculate.

<br>

### To-do 7. Add mlfqs_update_priority_all() function.  (threads/thread.c)
#### - thread.c
```C
void mlfqs_update_priority_all (void){
  struct list_elem *e;

  for (e = list_begin (&all_list); e != list_end (&all_list); e = list_next (e))
    {
      mlfqs_update_priority(list_entry(e, struct thread, allelem));
    }
}
```
> **mlfqs_update_priority() function is for updating all_list's priority.** <br>

<br>

### To-do 8. Add mlfqs_update_recent_cpu_all() function.  (threads/thread.c)
#### - thread.c
```C
void mlfqs_update_recent_cpu_all (void){
  struct list_elem *e;

  for (e = list_begin (&all_list); e != list_end (&all_list); e = list_next (e))
    {
      mlfqs_update_recent_cpu(list_entry(e, struct thread, allelem));
    }
}
```
> **mlfqs_update_recent_priority() function is for updating all_list's recent_cpu.** <br>

#### - thread.h
```C
...

void mlfqs_update_priority (struct thread *);
void mlfqs_update_recent_cpu (struct thread *);
void mlfqs_update_load_avg (void);
void mlfqs_incre_recent_cpu (void);
void mlfqs_update_recent_cpu_all (void);
void mlfqs_update_priority_all (void);

...
```
> **Declare all the functions above to thread.h.** <br>

<br>

### To-do 9. Modify init_thread() function.  (threads/thread.c)
#### - thread.c
```C
static void
init_thread (struct thread *t, const char *name, int priority)
{
  ...

  t->nice = 0;
  t-> recent_cpu = 0;
}
```
> **Initialize the nice and recent_cpu.**
> - The default values of nice and recent_cpu are 0. 

<br>

### To-do 10. Modify thread_set_priority() function.  (threads/thread.c)
#### - thread.c
```C
void
thread_set_priority (int new_priority) 
{
  if (!thread_mlfqs){
    thread_current ()->priority = new_priority;
    thread_current ()->original_priority = new_priority;
    
    update_priority();
    check_max_priority();
  }
}
```
> **Modify the function in case it runs in mlfqs.**
> - Added if statement because priority cannot be changed arbitrarily in mlfqs scheduler.

<br>

### To-do 11. Modify thread_set_nice() function.  (threads/thread.c)
#### - thread.c
```C
void
thread_set_nice (int nice UNUSED) 
{
  enum intr_level old_level;
  old_level = intr_disable();
  thread_current()->nice = nice;
  mlfqs_update_priority(thread_current());
  check_max_priority();
  intr_set_level (old_level);
}

```
> **Implement functions for setting nice.**
> - The interrupt must be disabled to set the nice of the current thread accurately.
> - Set the current thread's nice value and update the priority by using mlfqs_update_priority().
> - Schedule according to priority.

<br>

### To-do 12. Modify thread_get_nice() function.  (threads/thread.c)
#### - thread.c
```C
int
thread_get_nice (void) 
{
  enum intr_level old_level;
  old_level = intr_disable();
  int result = thread_current()->nice;
  intr_set_level (old_level);
  return result;
}

```
> **Implement functions for getting nice.**
> - The interrupt must be disabled to set the nice of the current thread accurately.
> - Return the current_thread's nice.


<br>

### To-do 13. Modify thread_load_avg() function.  (threads/thread.c)
#### - thread.c
```C
int
thread_get_load_avg (void) 
{
  enum intr_level old_level;
  old_level = intr_disable();
  int result = fixed_point_to_int_round(mult_mixed(load_avg, 100));
  intr_set_level (old_level);
  return result;
}

```
> **Implement functions for getting load_avg.**
> - The interrupt must be disabled to get the load_avg accurately
> - load_avg is float type and 100 is int. So, mult_mixed is using to calculate.
> - Because the function type is int, result should be int. So, fixed_point_to_int_round is used to calculate.

<br>

### To-do 14. Modify thread_get_recent_cpu() function.  (threads/thread.c)
#### - thread.c
```C
int
thread_get_recent_cpu (void) 
{
  enum intr_level old_level;
  old_level = intr_disable();
  int result = fixed_point_to_int_round(mult_mixed(thread_current()->recent_cpu, 100));
  intr_set_level (old_level);
  return result;
}

```
> **Implement functions for getting recent_cpu.**
> - The interrupt must be disabled to get the recent accurately.
> - recent_cpu is float type and 100 is int type. So, mult_mixed() is used to calculate.
> - Because the function type is int, result should be int. So, fixed_point_to_int_round is used to calculate.

<br>

### To-do 15. Modify lock_aquire() function.  (threads/synch.c)
#### - synch.c
```C
void
lock_acquire (struct lock *lock)
{
  ...
    if (lock->holder != NULL){
        cur->waiting_lock = lock;
        list_insert_ordered(&lock->holder->donation_list, &cur->donation_elem, cmp_donation_priority, NULL);
    
        if(!thread_mlfqs){
          donate_priority();
        }
    }
}
```
> **Modify the function not to run donate_priority when mlfqs.**
> - Added if condition statement because donation should be disabled when mlfqs scheduler.

<br>

### To-do 16. Modify lock_release() function.  (threads/synch.c)
#### - synch.c
```C
void
lock_release (struct lock *lock)
{
    ...
    
    if (!thread_mlfqs){
        remove_lock(lock);
        update_priority();
    }
    
    ...
}
```
> **Modify the function not to run donate_priority when mlfqs.**
> - Added if condition statement because donation should be disabled when mlfqs scheduler.

<br>

### To-do 17. Modify timer_interrupt() function.  (devices/timer.c)
#### - timer.c
```C
static void
timer_interrupt (struct intr_frame *args UNUSED)
{
    ...

    if (thread_mlfqs){
        mlfqs_incre_recent_cpu();
        
        if(timer_ticks() % 4 == 0){
            mlfqs_update_priority_all();

            if (timer_ticks() % 100 == 0){
                mlfqs_update_load_avg();
                mlfqs_update_recent_cpu_all();
            }
        }
    }

    ...
}
```
> **Calculate recent_cpu, priority, load_avg as tick grows.**
> - For mlfqs schedulers, the tick should be increased by 1 each time time_interrupt is called.
> - Update all priorities every 4 seconds by using 'mlfqs_update_priority_all()'.
> - Update load_avg by using 'mlfqs_update_load_avg()' when timer_ticks() % TIMER_FREQ == 0.
> - Update all recent_cpu by using 'mlfqs_update_recent_cpu_all()' when timer_ticks() % TIMER_FREQ == 0.


<br>
