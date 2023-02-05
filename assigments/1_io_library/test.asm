
            section .data
        arg1: db 'ashdb asdhabs dahb', 0
        arg2: times 9 db  66,0
        section .text
        %include "lib.inc"
        global _start 
        _start:
        
mov rdi, -1
mov rsi, -1
mov rax, -1
mov rcx, -1
mov rdx, -1
mov r8, -1
mov r9, -1
mov r10, -1
mov r11, -1

        mov rdi, arg1
        mov rsi, arg2
        mov rdx, 9
        call string_copy
        test rax, rax
        jnz .good
        .good:
        

section .data
err_calling_convention: db "You did not respect the calling convention! Check that you handled caller-saved and callee-saved registers correctly", 10
.end:
section .text
continue:

        mov rdi, arg2 
        call print_string
        _exit:
        mov rax, 60
        xor rdi, rdi
        syscall