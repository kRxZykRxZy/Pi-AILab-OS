.section .text
.global vectors
vectors:
    b _start
    b exception_undef
    b exception_swi
    b exception_prefetch
    b exception_data
    b exception_reserved
    b exception_irq
    b exception_fiq

exception_undef:     b exception_undef
exception_swi:       b exception_swi
exception_prefetch:  b exception_prefetch
exception_data:      b exception_data
exception_reserved:  b exception_reserved
exception_irq:       b exception_irq
exception_fiq:       b exception_fiq
