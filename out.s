start:
  call Smain
  ret
  jmp $
Sfib:
  mov ax,[vars+4]
  mov word[vars+0],ax
  mov ax,[vars+8]
  mov word[vars+4],ax
  mov ax,[vars+0]
  mov bx,[vars+4]
  add ax,bx
  mov word[vars+8],ax
  ret
  jmp $
Smain:
M0:
  mov ax,1
  mov bx,1
  cmp ax,bx
  jnz M1
  mov ax,[vars+0]
  mov bx,1
  add ax,bx
  mov word[vars+0],ax
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
