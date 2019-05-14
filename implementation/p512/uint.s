
.intel_syntax noprefix

.section .rodata

.global uint_1
uint_1: .quad 1, 0, 0, 0, 0, 0, 0, 0


.section .text

.global uint_set
uint_set:
    cld
    mov rax, rsi
    stosq
    xor rax, rax
    mov rcx, 7
    rep stosq
    ret


.global uint_bit
uint_bit:
    mov rcx, rsi
    and rcx, 0x3f
    shr rsi, 6
    mov rax, [rdi + 8*rsi]
    shr rax, cl
    and rax, 1
    ret


.global uint_add3
uint_add3:
    mov rax, [rsi +  0]
    add rax, [rdx +  0]
    mov [rdi +  0], rax
    .set k, 1
    .rept 7
        mov rax, [rsi + 8*k]
        adc rax, [rdx + 8*k]
        mov [rdi + 8*k], rax
        .set k, k+1
    .endr
    setc al
    movzx rax, al
    ret

.global uint_sub3
uint_sub3:
    mov rax, [rsi +  0]
    sub rax, [rdx +  0]
    mov [rdi +  0], rax
    .set k, 1
    .rept 7
        mov rax, [rsi + 8*k]
        sbb rax, [rdx + 8*k]
        mov [rdi + 8*k], rax
        .set k, k+1
    .endr
    setc al
    movzx rax, al
    ret


.global uint_mul3_64
uint_mul3_64:

    mulx r10, rax, [rsi +  0]
    mov [rdi +  0], rax

    mulx r11, rax, [rsi +  8]
    add  rax, r10
    mov [rdi +  8], rax

    mulx r10, rax, [rsi + 16]
    adcx rax, r11
    mov [rdi + 16], rax

    mulx r11, rax, [rsi + 24]
    adcx rax, r10
    mov [rdi + 24], rax

    mulx r10, rax, [rsi + 32]
    adcx rax, r11
    mov [rdi + 32],rax

    mulx r11, rax, [rsi + 40]
    adcx rax, r10
    mov [rdi + 40],rax

    mulx r10, rax, [rsi + 48]
    adcx rax, r11
    mov [rdi + 48],rax

    mulx r11, rax, [rsi + 56]
    adcx rax, r10
    mov [rdi + 56],rax

    ret

