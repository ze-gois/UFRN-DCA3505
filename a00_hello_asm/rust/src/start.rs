#[unsafe(no_mangle)]
#[naked]
pub unsafe extern "C" fn start() -> ! {
    // In naked functions, only a single asm block is allowed
    unsafe {
        core::arch::naked_asm!("mov rsp, rdi", "mov rdx, rsi", "call entry", "hlt");
    }
}
