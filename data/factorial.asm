in
call :factorial
out
hlt

;-------------------
:factorial
dub
push 1
jae :factdone
dub push 1 sub
call :factorial
mul
:factdone
ret
;-------------------
