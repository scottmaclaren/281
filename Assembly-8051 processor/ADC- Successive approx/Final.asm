

$MODDE2

org 0000H
ljmp myprogram


delay100us: 
mov R1, #10
aaa: mov R0, #111
bbb: djnz R0, bbb ; 111*30ns*3=10us 
djnz R1, aaa ; 10*10us=100us, approximately 
ret

myprogram:

mov SP, #7FH ; Set the stack pointer
mov LEDRA, #0 ; Turn off all LEDs
mov LEDRB, #0
mov LEDRC, #0
mov LEDG, #0
mov P1MOD, #11111111B ; Configure 1.0 to 1.7 as outputs 
mov p2mod, #00000000B ; use pin set two as input- need one pin
mov P3MOD, #11111111B ; move pin set three to be output

mov hex2, #00001000B
mov hex1, #01000000B
mov hex0, #01000110B


loop:
lcall ADC1
lcall ADC2
sjmp loop

ADC1:
mov p3, #0
lcall delay100us

setb p3.7 
lcall delay100us
jnb p2.0, L1 ; this see's if the comparison is not true
clr p3.7

L1:

setb p3.6 
lcall delay100us
jnb p2.0, L2
clr p3.6

L2:

setb p3.5 ;next highest bit
lcall delay100us
jnb p2.0, L3
clr p3.5

L3:

setb p3.4 ;next highest bit
lcall delay100us
jnb p2.0, L4
clr p3.4

L4:

setb p3.3 ;next highest bit
lcall delay100us
jnb p2.0, L5
clr p3.3

L5:

setb p3.2 ;next highest bit
jnb p2.0, L6
clr p3.2

L6:

setb p3.1 ;next highest bit
lcall delay100us
jnb p2.0, L7
clr p3.1

L7:

setb p3.0 ;next highest bit

jnb p2.0, L8
clr p3.0

L8:

mov LEDG, p3

ret

ADC2:
mov p3, #0
lcall delay100us

setb p3.7 
lcall delay100us
jnb p2.1, M1 ; this see's if the comparison is not true
clr p3.7

M1:

setb p3.6 
lcall delay100us
jnb p2.1, M2
clr p3.6

M2:

setb p3.5 ;next highest bit
lcall delay100us
jnb p2.1, M3
clr p3.5

M3:

setb p3.4 ;next highest bit
lcall delay100us
jnb p2.1, M4
clr p3.4

M4:

setb p3.3 ;next highest bit
lcall delay100us
jnb p2.1, M5
clr p3.3

M5:

setb p3.2 ;next highest bit
jnb p2.1, M6
clr p3.2

M6:

setb p3.1 ;next highest bit
lcall delay100us
jnb p2.1, M7
clr p3.1

M7:

setb p3.0 ;next highest bit

jnb p2.1, M8
clr p3.0

M8:

mov LEDRA, p3

ret

end

