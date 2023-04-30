# Project 2 - Argument Passing
> Extend process_excute() to support passing arguments to new processes.

<br>

## Files to modify
- userprog/process.*

<br>

## To-do
- **Add fixed-point.h File.** (threads/fixed-point.h) <br>
     : Adds a file with a function defined for floating point operations within pintOS.

     
- **Modify thread structure.** (threads/thread.h) <br>
     : Add new int type fields for advanced scheduling.

     
- **Add mlfqs_update_priority() function.** (threads/thread.\*) <br>
     : Calculate to change the priority to PRI_MAX - (recent_cpu / 4) - (nice * 2).
     

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
