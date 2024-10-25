::main
call :getabc  ; scans a,b,c to AA, BB, CC respectively
call :square
hlt

;---------------------------------------------------------
:getabc
putcc 97 putcc 58 putcc 32
in
pop AA
putcc 98 putcc 58 putcc 32
in
pop BB
putcc 99 putcc 58 putcc 32
in
pop CC
ret
;---------------------------------------------------------

;---------------------------------------------------------
:square
push 0
push AA
jne :skiplin1
call :linear
ret
:skiplin1
push BB sqr
push AA push CC push 4
mul mul sub
pop DD          ; discriminant

push DD
push 0
jbe :validsqr
call :non
ret

:validsqr

push BB inv
push DD sqrt
sub
push AA push 2 mul div
putcc 120 putcc 49 putcc 58 putcc 32    ; x1:
out

push BB inv
push DD sqrt
add
push AA push 2 mul div
putcc 120 putcc 50 putcc 58 putcc 32    ; x2:
out
ret
;---------------------------------------------------------

;---------------------------------------------------------
:linear
push BB
push 0
jne :skipconst
call :const
ret
:skipconst
push CC inv
push BB div
putcc 120 putcc 58 putcc 32    ; x:
out
ret
;---------------------------------------------------------

;---------------------------------------------------------
:const
push CC
push 0
jne :nosols
call :anyn
ret
:nosols
call :non
ret
;---------------------------------------------------------

;---------------------------------------------------------
:anyn
putcc 97 putcc 110 putcc 121 putcc 10   ; any'\n'
ret
;---------------------------------------------------------

;---------------------------------------------------------
:non
putcc 110 putcc 111 putcc 10            ; no'\n'
ret
;---------------------------------------------------------
