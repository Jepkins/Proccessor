;------------------------------------------------------------------------------------------
::main
push 50 pop CX CY ; center coords
push RR+1 pop RR  ; radius
call :drawcirc

draw
slpdif 100000
push 50 RR
ja :end
call ::main
:end
hlt
;------------------------------------------------------------------------------------------

;------------------------------------------------------------------------------------------
:drawcirc
push 0               ; for
pop AX AY
:loop0

call :ifdraw         ; checks AX, AY and ocasionally calls :setpnt

push AY+1 dub pop AY ; j++
push 100
ja :loop0            ; j < 100
mov AY 0
mov AX AX+1          ; i++
push AX 100
ja :loop0            ; i < 100
ret
;------------------------------------------------------------------------------------------

;------------------------------------------------------------------------------------------
:ifdraw
push AX AY CY
sub
sqr pop BY     ; (y-yc)^2 -> BY
push CX sub
sqr push BY    ; BY, (x-xc)^2 -> stack
add
push RR sqr
jb :ifdrawskip
call :setpnt
:ifdrawskip
ret
;------------------------------------------------------------------------------------------

;------------------------------------------------------------------------------------------
:setpnt
push AX 100 mul
push AY
add
pop II
push -16711936
pop [II]
ret
;------------------------------------------------------------------------------------------
