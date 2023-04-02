# Project 1 - Priority Scheduling
> PintOS uses FIFO sceduling now. So, we modified pintOS scheduler for priority scheduling.

<br>

## 1. Priority Scheduling
### 1) Files to modify
- threads/thread.*

<br>

### 2) To-do
- **Type** (threads/thread.h) <br>
     : desc
     
 
<br>

### 3) Project Description

#### - thread.h

``` C
struct thread {

  ...

  
  
  ...

}
```
> Add 

<br>

<br>

## 2. Priority Synchronization
### 1) Files to modify
- threads/thread.*
- threads/synch.*

<br>

### 2) To-do
- **Type** (threads/thread.h) <br>
     : desc


<br>

### 3) Project Description
``` C
struct thread {

  ...

  
  
  ...

}
```
> Add 

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
``` C
struct thread {

  ...

  
  
  ...

}
```
> Add 

<br>
