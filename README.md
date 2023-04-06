# Project 1 - Alarm Clock
> Reimplement code to avoid busy waiting

<br>

## Files to modify
- threads/thread.*
- devices/timer.*

<br>

## To-do
- **Modify thread structure.** (threads/thread.h) <br>
     : Add new int64 type field for local tick.

     
- **Modify thread_init() function.** (threads/thread.c) <br>
     : Add the code to initialize the sleep queue data structure.

     
- **Modify timer_sleep() function.** (devices/timer.c) <br>
     : Call the function that insert thread to the sleep queue.

     
- **Modify timer_interrupt() function.** (devices/timer.c) <br>
     : At every tick, check whether some thread must wake up from sleep queue and call wake up function.

     
- **Add thread_sleep() function.**  (threads/thread.c) <br>
     : Set thread state to blocked and wait after insert it to sleep queue.

     
- **Add thread_wakeup() function.**  (threads/thread.c) <br>
     : Find the thread to wake up from sleep queue and wake up it.

     
- **Add save_mintick function.**  (threads/thread.c) <br>
     : Save the minimum calue of tick that threads have.

     
- **Add return_mintick function.**  (threads/thread.c) <br>
     : Return the minimum value of tick.


<br>
<br>

## Project Description

### - thread.h
#### To-do 1. Modify thread structure. (threads/thread.h)

``` C
struct thread {

  ...

  /* local tick till wake up */
  int64_t wakeup_tick;
  
  ...

}
```
> **Add int64_t type field named 'wakeup_tick'to thread structure** <br>
> - •wakeup_tick : the value of (its timer ticks + system ticks)

<br>

```C
void thread_sleep(int64_t ticks);
void thread_wakeup(int64_t ticks);
void save_mintick(int64_t ticks);
int64_t return_mintick(void);
```
> **Declare the functions in 'thread.h' we newly creadted in 'thread.c'** <br>

<br>

### - thread.c
#### To-do 2. Modify thread_init() function. (threads/thread.c)
``` C
/* project 1 : the list to control blocked thread */
static struct list sleep_list;
static int64_t next_tick_to_wakeup;
```
> **Define a list struct 'sleep_list' and int64 type 'next_tick_to_wakeup'** <br>
> - sleep_list : a queue to store blocked threads until its time to wake them up <br>
> - next_tick_to_wakeup : the minimum value of tick that threads have 

<br>

```C
void
thread_init (void) {
  ...
  
  /* project 1 : init blocked_list and next_to_wakeup */
  list_init (&sleep_list);
  next_tick_to_wakeup = INT64_MAX;
  
  ...
}
```
> **Initialize the sleep queue using list_init** <br>
> **Initialize next_tick_to_wakeup to be INT64_MAX** <br>
>  so that whenever there is any other ticks smaller than current, it can be updated to be the smaller one.

<br>

#### To-do 3. Add thread_sleep() function.  (threads/thread.c)
```C
void
thread_sleep(int64_t ticks){
  struct thread *current;
  enum intr_level old_level;

  old_level = intr_disable();
  current = thread_current();

  ASSERT(current != idle_thread);

  current -> wakeup_tick = ticks;
  save_mintick(ticks);
  
  list_push_back (&sleep_list, &current -> elem);
  thread_block();
  
  intr_set_level(old_level);
}
```
> **Create a function 'thread_sleep()'** <br>
> - thread_sleep(int64_t ticks) : It is called whenever a thread need to sleep, that is, need to be blocked and moved to sleep queue. <br>
>  - <span style="color:orange"> interrupts turn off </span>
>  - **Call ASSERT(current != idle_thread)** <br>
>   - because if an idle thread is blocked, the cpu stops working so any idle thread should not be blocked. <br>
>  - **Add variable 'current' and save local tick** <br>
>   - current : currently running thread <br>
>   - current->wakeup_tick : set to be 'ticks', parameter received from 'thread_sleep()', value of (timer ticks+system tick) <br>
>  - **Call 'save_minticks()'** <br>
>    to update the value of minimum tick that threads have <br>
>    save_minticks() : newly created function. Will be described below <br>
>  - **Call 'list_push_back()'** <br>
>   - to put the current thread into the sleep queue <br>
>  - **Call 'thread_block()'** <br>
>   - to set the status of the current thread to be 'THREAD_BLOCKED'
>  - <span style="color:orange"> interrupts turn on </span>

<br>

```C
void
thread_wakeup(int64_t ticks){
  struct list_elem *e = list_begin(&sleep_list);
  struct thread *t;
  enum intr_level old_level;

  old_level = intr_disable();
  next_tick_to_wakeup = INT64_MAX;

  while(e != list_end(&sleep_list)){
    t = list_entry(e, struct thread, elem);
  
    if (t-> wakeup_tick <= ticks){
      e = list_remove(e);
      thread_unblock(t);
    }
    else{
      e = list_next(e);
      save_mintick(ticks);
    }
  }
  intr_set_level(old_level);
}
```
> - Declare variable *e, and initiate it to be the first element of the sleep list. <br>
> - Declare variable *t. <br>
> - **Turn interrupt off.** <br>
> - Set the value of next_tick_to_wakeup to be INT64_MAX, so that whenever there is any smaller ticks, it can be updated. <br>
>   - Using while clause, traversal sleep list from the first to end element to find a thread to wake up and call thread_unblock() <br>
>   -  Set variable t to be the tread that is being currently traversaled. <br>
>   -  If wakeup_tick(which is the ticks t has to wakeup) is equal or smaller than the parameter ticks(which is current system ticks), unblock t by calling thread_unblock(). <br>
>   - If it is not, move to the next sleeping thread and update minimum ticks by calling save_mintick(ticks) <- newly created function. described below. <br>
> - **Turn interrupts on.**

<br>

``` C
int64_t
return_mintick(void){
  return next_tick_to_wakeup;
}
```
> - thread가 가진 tick 중 가장 작은 tick을 나타내는 next_tick_to_wakeup 변수를 반환함

<br>

```C
void
save_mintick(int64_t ticks){
  if (ticks < next_tick_to_wakeup){
    next_tick_to_wakeup = ticks;
  }
}
```
> - thread가 가진 tick 중 가장 작은 tick을 나타내는 next_tick_to_wakeup 변수를 저장함

<br>

### - timer.c
```C
void
timer_sleep (int64_t ticks) 
{
  int64_t start = timer_ticks ();

  ASSERT (intr_get_level () == INTR_ON);
  thread_sleep(start+ticks);
}
```
> - Busy waiting을 방지하기 위해 기존 while문을 제거하고, thread_sleep() 함수를 호출함

<br>

```C
static void
timer_interrupt (struct intr_frame *args UNUSED)
{
  ticks++;
  thread_tick ();
  
  int64_t minTick = return_mintick();

  if (ticks >= minTick){
    thread_wakeup(ticks);
  }

}
```
> - thread.c에서 정의한 next_tick_to_wakeup 변수를 호출하여 현재 tick과 비교해 깨워야할 thread를 thread_wake() 함수로 ready queue에 넣음
