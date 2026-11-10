global set_cr3
global load_idtr

set_cr3:
    mov cr3, rdi
    ret

load_idtr:
    lidt [rdi]
    ret
