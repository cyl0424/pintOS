#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdbool.h>
#include "threads/thread.h"
#include "filesys/file.h"

typedef int pid_t;

void syscall_init (void);

void halt (void);
void exit (int status);
pid_t exec (const char *file);
int wait (pid_t);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

void address_check(void *addr);

void process_close_file(int fd_idx);
int process_add_file(struct file *f);
struct file *process_get_file(int fd_idx);



#endif /* userprog/syscall.h */
