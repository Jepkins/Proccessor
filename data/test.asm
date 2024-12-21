call :main
out
hlt

:main
push [BX+0]
push 0
je :endif#0x55589f347440
push [BX+0]
log
push 4
push [BX+1]
push 3
pow
mul
add
pop [BX+2]
push [BX+2]
push [BX+1]
div
pop AX
ret
:endif#0x55589f347440
push [BX+0]
push [BX+1]
add
pop [BX+2]
push [BX+2]
pop AX
ret
