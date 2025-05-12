.section .text
.global _start
_start:
    movl $42, %eax      # Assign value 42 to EAX register
    
    # Exit system call
    movl $1, %eax       # System call number for exit is 1
    movl $0, %ebx       # Return code 0 (success)
    int $0x80           # Invoke the system call