.section .data
message:
    .ascii "Grato por tudo.\n"
    .set message_length, . - message

.section .text
.global _start
_start:
    # syscall write(int fd, const void *buf, size_t count)
    # rax=1 (syscall number for write)
    # rdi=1 (file descriptor: stdout)
    # rsi=pointer to message
    # rdx=message length
    mov $1, %rax          # syscall number for write
    mov $1, %rdi          # file descriptor: stdout (1)
    lea message(%rip), %rsi  # pointer to the message
    mov $message_length, %rdx  # message length
    syscall               # call kernel

    # syscall exit(int status)
    # rax=60 (syscall number for exit)
    # rdi=0 (exit status: 0)
    mov $60, %rax         # syscall number for exit
    mov $0, %rdi          # exit status 0
    syscall               # call kernel
