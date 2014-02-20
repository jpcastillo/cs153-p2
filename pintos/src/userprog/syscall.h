#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

int S__WRITE(unsigned int, const void *, unsigned int);
int S__READ(int, void *, unsigned );
#endif /* userprog/syscall.h */
