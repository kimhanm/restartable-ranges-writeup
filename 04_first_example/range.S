.text
.globl _myrange_start, _myrange_end, _myrange_recovery
_myrange_start:
    nop 
loop:       /* does anyone remember that 5s tent scene from The Endless (2017)? */
    b loop
_myrange_end:
_myrange_recovery:
    ret
