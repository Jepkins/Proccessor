::main
call :getabc  ; scans a,b,c to AA, BB, CC respectively
call :square
hlt

;---------------------------------------------------------
:getabc
putcc 97 58 32
in
pop AA
putcc 98 58 32
in
pop BB
putcc 99 58 32
in
pop CC
ret
;---------------------------------------------------------

;---------------------------------------------------------
:square
push 0 AA
jne :skiplin1
call :linear
ret
:skiplin1
push BB sqr
push AA CC 4
mul mul sub
pop DD          ; discriminant

push DD 0
jbe :validsqr
call :non
ret

:validsqr

push BB inv
push DD sqrt
sub
push AA 2 mul div
putcc 120 49 58 32    ; x1:
out

push BB inv
push DD sqrt
add
push AA 2 mul div
putcc 120 50 58 32    ; x2:
out
ret
;---------------------------------------------------------

;---------------------------------------------------------
:linear
push BB 0
jne :skipconst
call :const
ret
:skipconst
push CC inv
push BB div
putcc 120 58 32    ; x:
out
ret
;---------------------------------------------------------

;---------------------------------------------------------
:const
push CC 0
jne :nosols
call :anyn
ret
:nosols
call :non
ret
;---------------------------------------------------------

;---------------------------------------------------------
:anyn
putcc 97 110 121 10   ; any'\n'
ret
;---------------------------------------------------------

;---------------------------------------------------------
:non
putcc 110 111 10            ; no'\n'
ret
;---------------------------------------------------------
