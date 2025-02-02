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

start:
  call Smain
  ret
  jmp $
Sfib:
  mov eax,[vars+4]
  mov dword[vars+0],eax
  mov eax,[vars+8]
  mov dword[vars+4],eax
  mov eax,[vars+0]
  mov ebx,[vars+4]
  add eax,ebx
  mov dword[vars+8],eax
  ret
  jmp $
Smain:
M0:
  mov eax,1
  mov ebx,1
  cmp eax,ebx
  setz al
  jnz M1
  mov eax,[vars+0]
  mov ebx,1
  add eax,ebx
  mov dword[vars+0],eax
  jmp M0
M1:
  ret
  jmp $
vars:
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
  dd 0
