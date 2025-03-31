#![no_std]

#[unsafe(no_mangle)]
pub extern "C" fn entry() {
    // Your program logic here
    // For example, writing a message to standard output
    const MSG: &[u8] = b"Hello, world!\n";

    unsafe {
        // Linux syscall: write(1, message, len)
        core::arch::asm!(
            "mov rax, 1",   // syscall number for write
            "mov rdi, 1",   // file descriptor: stdout
            "mov rsi, {0}", // message pointer
            "mov rdx, {1}", // message length
            "syscall",
            in(reg) MSG.as_ptr(),
            in(reg) MSG.len(),
            out("rax") _,
            out("rcx") _,
            out("r11") _,
        );

        // Exit syscall: exit(0)
        core::arch::asm!(
            "mov rax, 60",  // syscall number for exit
            "xor rdi, rdi", // exit code 0
            "syscall",
            options(noreturn)
        );
    }
}

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}
