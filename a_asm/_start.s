.section .data
    hello_msg: .string "Hello, ASM World!\n"

.section .text
    .global _start

_start:
    # Write the message to stdout (file descriptor 1)
    movq $1, %rax           # System call number for write
    movq $1, %rdi           # File descriptor 1 (stdout)
    leaq hello_msg(%rip), %rsi  # Address of string
    movq $16, %rdx          # Length of string
    syscall

    # Exit the program
    movq $60, %rax          # System call number for exit
    movq $0, %rdi           # Exit status 0
    syscall
