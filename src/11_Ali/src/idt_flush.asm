[GLOBAL idt_flush]

idt_flush:
    mov eax, [esp+4] ; load address of IDT pointer
    lidt [eax]       ; load the IDT
    ret
