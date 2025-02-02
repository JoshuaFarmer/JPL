        bits 32
        db "OSAX"
        db 0xAA
        dd __start
        dd 0x1
__start:
        call start
        mov ebx,0
        mov eax,1
        push eax
        push ebx
        int 0x80
        jmp $