section .text

string_length:
    xor rax, rax
.loop:                      ; main loop starts here
    cmp byte [rdi+rax], 0   ; Check if the current symbol is null-terminator.
                            ; We absolutely need that 'byte' modifier since
                            ; the left and the right part of cmp should be
                            ; of the same size. Right operand is immediate
                            ; and holds no information about its size,
                            ; hence we don't know how many bytes should be
                            ; taken from memory and compared to zero
    je .end                 ; Jump if we found null-terminator
    inc rax                 ; Otherwise go to next symbol and increase
                            ; counter
    jmp .loop
.end:
    ret                     ; When we hit 'ret', rax should hold return value


print_string:
    xor r9, r9
    mov r8, rdi            ; save input to ebx
.loop:                      ; main loop starts here
    cmp byte [r8+r9], 0   ; check if the string over
    je .end                 ; and jump to the end

    mov rax, 1              ; 'write' syscall identifier
    mov rdi, 1              ; stdout file descriptor
    lea rsi, [r8+r9]         ; get current character
    mov rdx, 1              ; the amount of bytes to write
    syscall

    inc r9
    jmp .loop               ; iterate over next characters

.end:
    xor rax, rax
    xor rdi, rdi
    xor rsi, rsi
    xor rdx, rdx
    xor r9, r9
    xor r8, r8
    ret                     ; When we hit 'ret', rax should hold return value

; rdi is input
print_char:
    push rdi                ; put argument to the stack
    mov rax, 1              ; 'write' syscall identifier
    mov rdi, 1              ; stdout file descriptor
    mov rsi, rsp            ; get character from stack
    mov rdx, 1              ; the amount of bytes to write
    syscall

    pop rax                 ; clean stack
    xor rax, rax
    ret


print_newline:
    push '\n'               ; put endline to the stack
    mov rax, 1              ; 'write' syscall identifier
    mov rdi, 1              ; stdout file descriptor
    mov rsi, rsp            ; get character from stack
    mov rdx, 1              ; the amount of bytes to write
    syscall

    xor rax, rax
    ret


; rdi -- parameter
print_uint:
    xor rcx, rcx
    mov rbx, 10

    mov rax, rdi ; move parameter to deviden number register

.loop:
    inc rcx
    xor rdx, rdx ; rdx will contains reminder
    
    div rbx
    ; Result: RAX
    ; Remainder: RDX

    add rdx, 0x30 ; Add '0' to rdx to make it printable
    
    ; We can't just push 1 byte to stack to instead
    ; We manually move stack pointer by 1 byte and the
    ; Put lowest byte of rdx (dl) to the stack pointer address
    dec rsp
    mov [rsp], dl      ; Store rdx in stack
    
    test rax, rax
    jnz .loop

.print_stack:
    ; Write stack values
    mov rax, 1              ; 'write' syscall identifier
    mov rdi, 1              ; stdout file descriptor
    mov rsi, rsp            ; get character from stack
    mov rdx, rcx            ; the amount of bytes to write
    push rcx
    syscall
    pop rcx

    add rsp, rcx            ; free stack

.end:
    xor rax, rax
    ret


; rdi -- parameter
print_int:
    mov r8, rdi ; save input to r8
    cmp r8, 0
    jns .print_value ; if positive value skip print_neg

.print_negative_sign:
    mov rdi, 45 ; move '-' to the print char arg
    call print_char
    neg r8

.print_value:
    mov rdi, r8 ; move input to the print_uint input
    call print_uint

.end:
    xor rax, rax
    ret

; rdi, str1
; rsi, str2
string_equals:
    push rcx     ; use rcx as iterator
    push r8      ; use r8 as tmp1 character
    push r9      ; use r9 as tmp2 character
    xor rcx, rcx
    xor r8, r8
    xor r9, r9
    mov rax, 1   ; equals by default

.loop:
    mov r8b, [rdi + rcx]
    mov r9b, [rsi + rcx]
    inc rcx

    cmp r8b, r9b        ; if characteds not equals return 0
    jne .return_not_equals

    cmp r8b, 0          ; if end of string return 1
    je .return_equals

    jmp .loop           ; otherwise repeat

.return_equals:
    mov rax, 1
    jmp .end

.return_not_equals:
    mov rax, 0
    jmp .end

.end:
    pop r9
    pop r8
    pop rcx
    ret


read_char:
    push 0          ; increase stack by 1
    mov rax, 0      ; call read syscall
    mov rdi, 0      ; from stdin
    mov rdx, 1      ; 1 byte
    mov rsi, rsp    ; put result on the stack
    syscall

    pop rax
    ret

.eof:
    xor rax, rax
    ret

; rdi contains address of word_buf
; rsi contains size of buffer 
read_word:
    push r10            ; save original r10
    xor r10, r10        ; iterator
.loop:
    cmp r10, rsi        ; compare current read position with buffer size
    je .buffer_overflow ; if equals returns 0

    push rdi            ; save rdi
    push rsi            ; save rsi
    call read_char
    pop rsi             ; resore rsi
    pop rdi             ; restore rdi
    
    cmp al, 32         ; if read ' ' -- return buffer
    je .end

    cmp al, 10         ; if read ' ' -- return buffer
    je .end
    cmp al, 13         ; if read ' ' -- return buffer
    je .end
    
    cmp al, 9          ; if read '\t' -- return buffer
    je .end

    cmp al, 0          ; if read EOF -- return buffer
    je .end
    
    mov byte [rdi + r10], al   ; move read character to memory buffer
    inc r10
    jmp .loop           ; jump to loop if bufer is not over

.end:
    mov byte [rdi + r10], 0     ; put '\0' at the end
    mov rax, rdi                ; move address of buffer to result
    mov rdx, r10                ; store word length to rdx
    pop r10                     ; restore r10
    ret

.buffer_overflow:
    xor rax, rax              ; return 0 in case of error
    xor rdx, rdx              ; word length is 0
    pop r10                   ; restore r10
    ret

; rdi points to a string
; returns rax: number, rdx : length
parse_uint:
    push rbx       ; rbx used for termorary result
    push r8        ; r8 used for current digit 
    push r9        ; r9 used for remporary iterator
    xor rax, rax   ; put 0 to result
    xor r9, r9   ; put 0 to the length
    xor rdx, rdx   ; put 0 to the length

.loop:
    xor r8, r8     ; put 0 to current char
    mov r8b, byte[rdi + r9] ; move char to r8 lowest byte

    ; if character is less then '0' return result
    cmp r8b, 48
    jl .end

    ; if character greater then '9' return result
    cmp r8b, 57
    jg .end

    sub r8b, 48              ; substract '0' to get the value

    mov rax, 10              ; multiply by 10
    mul rbx                  ; add one digit
    mov rbx, rax             ; mult result is on rax, move it to rbx
    add rbx, r8              ; add to result current digit

    inc r9                   ; increment length to digit
    jmp .loop 

.end:
    mov rax, rbx   ; store temporary result to rax
    mov rdx, r9   ; store temporary result to rax
    pop r8         ; restore r8
    pop r9         ; restore r9
    pop rbx        ; restore rbx
    ret

; rdi points to a string
; returns rax: number, rdx : length
parse_int:
    push r8        ; r8 used for the first char
    xor r8, r8
    mov r8b, byte [rdi]  ; get first character to r8

    cmp r8, 45      ; check if character is '-'
    je .parse_negative   ; if it is parse negative uint

    ; otherwise parse positive
    call parse_uint
    jmp .end

.parse_negative:
    inc rdi         ; skip sign
    call parse_uint
    
    push rdx       ; save rdx
    mov r8, -1
    mul r8         ; change sign of rax
    
    pop rdx        ; restore rdx
    inc rdx        ; increase the length

.end:
    pop r8
    ret 

; rdi original string
; rsi buffer
; rdx buffer length
string_copy:
    push r8             ; use r8 for tmp char
    push rcx            ; rcx is iterator
    xor rcx, rcx

.loop:
    cmp rcx, rdx        ; buffer is over
    je .return_zero

    mov r8b, [rdi+rcx]
    mov [rsi+rcx], r8b

    cmp r8, 0           ; if string is over
    je .return_dest     ; return destination

    inc rcx
    jmp .loop           ; Otherwise repeat

.return_zero:
    xor rax, rax
    jmp .end

.return_dest:
    mov rax, rsi
    jmp .end

.end:
    pop rcx
    pop r8
    ret