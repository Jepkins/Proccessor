;---------------------------------------------
::main
push 50 dub pop CX pop CY  ; center coords
push RR push 1 add pop RR  ; radius
call :drawcirc

draw
sleep 50
push 50 push RR
ja :end
call ::main
:end
hlt
;---------------------------------------------

;---------------------------------------------
:drawcirc
push 0 dub           ; for
pop AX pop AY
:loop0

call :ifdraw         ; checks AX, AY and ocasionally calls :setpnt

push AY+1 dub pop AY ; j++
push 100
ja :loop0            ; j < 100
push 0 pop AY
push AX+1 dub pop AX ; i++
push 100
ja :loop0            ; i < 100
ret
;---------------------------------------------

;---------------------------------------------
:ifdraw
push AX push AY
push CY sub
sqr pop BY     ; (y-yc)^2 -> BY
push CX sub
sqr push BY    ; BY, (x-xc)^2 -> stack
add
push RR sqr
jb :ifdrawskip
call :setpnt
:ifdrawskip
ret
;---------------------------------------------

;---------------------------------------------
:setpnt
push AX push 100 mul
push AY
add
pop II
push -16711936
pop [II]
ret
;---------------------------------------------
