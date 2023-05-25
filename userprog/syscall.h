#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

typedef int pid_t;

void syscall_init (void);
void close_file_by_owner (int);

int mmap(int fd, void *addr);
void munmap(int mapid);

#endif /* userprog/syscall.h */
