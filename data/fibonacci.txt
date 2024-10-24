in
pop AX
push 1 dub dub dub
pop [1] pop [2]
pop BX
pop CX
push 3
pop DX

:nextfib
push CX
push CX push BX add
dub pop [DX]
pop CX
pop BX
push DX+1 dub
pop DX
push AX
jae :nextfib

push 1 pop DX
:nextprint
push [DX]
out
push DX+1 dub
pop DX
push AX
jae :nextprint

hlt
