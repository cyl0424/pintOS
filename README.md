# Project 1 - Priority Scheduling
> PintOS uses FIFO sceduling now. So, we modified pintOS scheduler for priority scheduling.

<br>

## 1. Priority Scheduling
### 1) Files to modify
- threads/thread.*

<br>

### 2) To-do
- **Add cmp_priority() function.** (threads/thread.c) <br>
     : Compare the priority between two thread.
     

- **Add check_max_priority() function.** (threads/thread.c) <br>
     :  Compare the priority and run the 'thread_yield ()' depending on the condition.

  
<br>

### 3) Project Description

#### - thread.h
#### To-do 1. Add cmp_priority() and check_max_priority() function. (threads/thread.h)
``` C
...

bool cmp_priority(const struct list_elem *max_pri, const struct list_elem *current_pri, void *aux UNUSED);
void check_max_priority(void);

...
```
> **Declare cmp_priority(), check_max_priority() functions in 'thread.h' we newly created in 'thread.c'. Will be described below**

<br>

``` C
bool
cmp_priority(const struct list_elem *max_pri, const struct list_elem *current_pri, void *aux UNUSED){
  return list_entry (max_pri, struct thread, elem) -> priority > list_entry (current_pri, struct thread, elem)-> priority;
}
```
> **Create a function 'cmp_priority()'**
> - **bool cmp_priority(const struct list_elem \*max_pri, const struct list_elem \*current_pri, void \*aux UNUSED)** : This function returns a True only if the priority of the first thread on the ready_list is greater than the priority of the current running thread
>	- **list_entry (max_pri, struct thread, elem) -> priority** <br>
>		to get the priority of thread located at the first of the ready_list <br>
> 	- **list_entry(current_pri, struct thread, elem) -> priority** <br>
>		to get the priority of running thread <br>

<br>

``` C
void
check_max_priority(void){
  if (!list_empty(&ready_list)){
    struct thread *cur = thread_current();
    struct thread *next = list_entry(list_begin(&ready_list), struct thread, elem);
    if (next->priority > cur->priority){
      thread_yield();
    }
  }
}
```
> **Create a function 'check_max_priority(void)'**
> - **check_max_priority(void)** : This function execute 'thread_yield()' only if the priority of the first thread on the ready_list is greater than the priority of the current running thread.
> 	- **if (!list_empty(&ready_list))**: if the ready list is not empty, <br>
>		- cur: currently running thread
>		- next: thread located at the first of the ready_list <br>
>	- **if (next->priority > cur->priority)**: if the priority of the first thread on the ready_list is greater than the priority of the current running thread <br>
> 		- execute 'thread_yield()': This function changes the state of the running thread to 'THREAD_READY' and inserts the running thread into ready list.

<br>

#### - thread.c
#### To-do 2. Modify thread_create() function. (threads/thread.c)
``` C
tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux) 
{
  ...

  check_max_priority(); 

  return tid;

  ...
}
```
> **Call 'check_max_priority()' function** <br>
> 	- To compare with the priority of new thread and running thread when creating a new thread.

##### To-do 3. Modify thread_set_priority() function. (threads/thread.c)

``` C
void
thread_set_priority (int new_priority) 
{
  thread_current ()->priority = new_priority;
  check_max_priority();
}
```
> **Call 'check_max_priority()' function** <br>
> 	- To compare with the priority of new thread and running thread when changing a new thread.

<br>

#### To-do 4. Modify thread_unblock() function. (threads/thread.c)
``` C
void
thread_unblock (struct thread *t) 
{
  enum intr_level old_level;
  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);
  // list_push_back (&ready_list, &t->elem);
  list_insert_ordered(&ready_list, &t -> elem, cmp_priority, NULL);
  t->status = THREAD_READY;
  intr_set_level (old_level);
}
```
> **Remove 'list_push_back ()' function** <br>
> 	- to consider the priority.
> **Add 'list_insert_ordered ()' function** <br>
> 	- **list_insert_ordered(&read_list, &t -> elem, cmp_priority, NULL)**: This function sorts ready_list in descending order through the cmp_priority() function.
<br>

#### To-do 5. Modify 'thread_yield ()' function. (threads/thread.c)

```C
void
thread_yield (void) 
{
  struct thread *cur = thread_current ();
  enum intr_level old_level;
  
  ASSERT (!intr_context ());

  old_level = intr_disable ();
  if (cur != idle_thread) 
    // list_push_back (&ready_list, &cur->elem);
    list_insert_ordered(&ready_list, &cur -> elem, cmp_priority, NULL);
  cur->status = THREAD_READY;
  schedule ();
  intr_set_level (old_level);
}
```
> **Remove 'list_push_back ()' function** <br>
> 	- To consider the priority. <br>
>
> **Add 'list_insert_ordered ()' function** <br>
> 	- **list_insert_ordered(&read_list, &t -> elem, cmp_priority, NULL)**: This function sorts ready_list in descending order through the cmp_priority() function.
<br>

<br>

## 2. Priority Synchronization
### 1) Files to modify
- threads/synch.*

<br>

### 2) To-do
- **Type** (threads/thread.h) <br>
     : desc


<br>

### 3) Project Description
#### - synch.h

``` C
...

bool cmp_sema_priority(const struct list_elem *a, const struct list_elem *b, void *aux);

...
```
> Declare cmp_sema_priority() function.

<br>

#### - synch.c

``` C
void
sema_down (struct semaohore *sema)
{
  ...

  while (sema->value == 0) 
    {
      // list_push_back (&sema->waiters, &thread_current ()->elem);
      list_insert_ordered(&sema->waiters, &thread_current()->elem, cmp_priority, NULL);
      thread_block ();
    }

  ...
}
```
> priority에 따라 waiters에 추가하도록 list_push_back이 아닌 list_insert_ordered()를 사용함.

<br>

``` C
void
cond_signal (struct condition *cond, struct lock *lock UNUSED) 
{
  ...

  if (!list_empty (&cond->waiters)) {
    list_sort(&cond -> waiters, cmp_sema_priority, NULL);
    sema_up (&list_entry (list_pop_front (&cond->waiters), struct semaphore_elem, elem)->semaphore);
  }
}
```
> cond의 waiters를 priority에 따라 정렬하도록 list_sort() 함수를 사용함.

<br>

``` C
void
cond_wait (struct condition *cond, struct lock *lock) 
{
  ...

  sema_init (&waiter.semaphore, NULL);
  // list_push_back (&cond->waiters, &waiter.elem);
  list_insert_ordered(&cond -> waiters, &waiter.elem, cmp_sema_priority, NULL);
  lock_release (lock);
  sema_down (&waiter.semaphore);
  lock_acquire (lock);
}
```
> cond의 waiters를 priority에 따라 추가하도록 list_push_back이 아닌 list_insert_ordered()를 사용함.

<br>

``` C
bool
cmp_sema_priority(const struct list_elem *l, const struct list_elem *s, void *aux UNUSED)
{
	struct semaphore_elem *l_sema = list_entry (l, struct semaphore_elem, elem);
	struct semaphore_elem *s_sema = list_entry (s, struct semaphore_elem, elem);

	struct list *waiter_l_sema = &(l_sema->semaphore.waiters);
	struct list *waiter_s_sema = &(s_sema->semaphore.waiters);

	return list_entry (list_begin (waiter_l_sema), struct thread, elem)->priority
		 > list_entry (list_begin (waiter_s_sema), struct thread, elem)->priority;
}
```
> 두 waiters의 첫번째 요소가 가진 priority를 비교하는 함수를 추가함.

<br>


<br>

## 3. Priority Donation
### 1) Files to modify
- threads/thread.*
- threads/synch.*

<br>

### 2) To-do
- **Modify thread structure** (threads/thread.h) <br>
     : Donation을 관리하기 위한 새로운 변수와 lock, list, list_elem을 추가해라.
     
     
- **Modify init_thread() function** (threads/thread.c) <br>
     : structure thread에 추가한 내용을 초기화할 코드를 추가해라.
     
     
- **Modify lock_acquire(), lock_release() finction** (threads/synch.c) <br>
     : Donation을 관리하기 위해 코드를 수정해라.


<br>

### 3) Project Description
#### - thread.h
``` C
struct thread {

  ...

  /* priority donation */
    int original_priority;
    struct lock *waiting_lock;
    struct list donation_list;
    struct list_elem donation_elem;
  
  ...

}
```
> - inherit priority에서 원래의 priority로 돌아올 때 사용할 original_priority를 추가함
> - lock을 control하기 위한 waiting_lock을 추가함
> - donation을 위한 donation_list, donation_elem을 추가함

<br>

``` C
...

void donate_priority(void);
void remove_lock(struct lock *lock);
void update_priority(void);

...
```
> Declare donate_priority(), remove_lock(), update_priority() functions.

<br>

#### - thread.c
``` C
static void
init_thread (struct thread *t, const char *name, int priority)
{

  ...

  /* priority donation */
  t->original_priority = priority;
  t->waiting_lock = NULL;
  list_init(&t->donation_list);
  
  ...

}
```
> thread.h에서 새로 정의한 변수와 리스트를 초기화함.

<br>

``` C
void
thread_set_priority (int new_priority) 
{
  thread_current ()->priority = new_priority;
  thread_current ()->original_priority = new_priority;

  update_priority();
  check_max_priority();
}
```
> - original_priority값을 new_priority로 선언함
> - thread의 priority 변수값을 donation_list 첫번째 요소의 priority와 비교하여 update하도록 update_priority() 함수를 사용함.

<br>

```C
void
remove_lock(struct lock *lock){
  struct thread *cur = thread_current();
  struct list_elem *e = list_begin(&cur -> donation_list);

  while(e != list_tail(&cur->donation_list)){
    struct thread *t = list_entry(e, struct thread, donation_elem);

    if(t->waiting_lock == lock){
      e = list_remove(e);
    }else{
      e = list_next(e);
    }
  }
}
```
> 현재 쓰레드의 donation_list를 순회하며 donation_list 요소의 waiting_lock이 삭제하고자 하는 lock과 같다면 제거하는 함수를 추가함.

<br>

```C
void update_priority(void){
  struct thread *cur = thread_current();
  cur->priority = cur->original_priority;

  if (!list_empty(&cur->donation_list)){
      struct thread *t = list_entry(list_begin(&cur->donation_list), struct thread, donation_elem);

      if (&cur->priority < &t->priority){
        cur->priority = t->priority;
      }
  }
}
```
> thread_current()의 priority와 donation_list 첫번째 요소의 priority를 비교하여 priority를 update하는 함수를 추가함.

<br>

```C
void donate_priority(void)
{
  struct thread *holder = thread_current()->waiting_lock->holder;
  int depth = 0;
  while (holder != NULL && depth < 8)
  {
    holder->priority = thread_current()->priority;
    if (holder->waiting_lock == NULL)
      break;
    holder = holder->waiting_lock->holder;
    depth++;
  }
}
```
> donation_priority 설명 

<br>

#### - synch.h
```C
bool cmp_donation_priority(const struct list_elem *max_pri, const struct list_elem *current_pri, void *aux);
```
> Declare cmp_donation_priority() function.

<br>

#### - synch.c
```C
void
lock_acquire (struct lock *lock)
{
  ...
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (!lock_held_by_current_thread (lock));
  
  struct thread *cur = thread_current();

  if (lock->holder != NULL){
    cur->waiting_lock = lock;
    list_insert_ordered(&lock->holder->donation_list, &cur->donation_elem, cmp_donation_priority, NULL);
    donate_priority();
  }

  sema_down (&lock->semaphore);
  cur->waiting_lock = NULL;

  lock->holder = thread_current ();
}
```
> lock_acquire 설명 추가

<br>

```C
void
lock_release (struct lock *lock) 
{
  ASSERT (lock != NULL);
  ASSERT (lock_held_by_current_thread (lock));

  lock->holder = NULL;

  remove_lock(lock);
  update_priority();

  sema_up (&lock->semaphore);
}
```
> waiting_lock에서 lock을 삭제하고 thread_current()의 priority를 update 할 수 있도록 코드를 수정함.

<br>

```C
bool
cmp_donation_priority(const struct list_elem *max_pri, const struct list_elem *current_pri, void *aux UNUSED)
{
  return list_entry (max_pri, struct thread, donation_elem) -> priority > list_entry (current_pri, struct thread, donation_elem)-> priority;
}
```
> donation_elem의 priority를 비교하는 함수를 추가함.

<br>
