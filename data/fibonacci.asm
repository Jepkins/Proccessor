in
pop AX
push 1
pop [1] [2] BX CX
mov DX 3

:nextfib
push CX
push CX push BX add
pop [DX] CX
pop BX
push DX+1
mov DX DX+1
push AX
jae :nextfib

push 1 pop DX
:nextprint
push DX+1 dub
pop DX
push AX
jae :nextprint

push DX 1 sub
pop DX
push [DX]
out

hlt
