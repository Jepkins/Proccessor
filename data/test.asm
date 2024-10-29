mst [2] 666 5
mov AX 0

:next
push [AX] out
mov AX AX+1
push AX 10
ja :next

hlt
