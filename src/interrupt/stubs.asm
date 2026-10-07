global page_fault_stub
extern page_fault_handler

section .text

%macro ENTER 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; RSP padding trick
    mov rax, rsp
    and rsp, -16
    sub rsp, 16
    mov [rsp], rax
    ; Now RSP points to the saved old RSP.
%endmacro


%macro LEAVE 0
    mov rsp, [rsp] ; Retrieve the old RSP.

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

page_fault_stub:
    ENTER

    mov rdi, [rax + 120]
    call page_fault_handler

    LEAVE
    add rsp, 8  ; pop error code
    iretq
