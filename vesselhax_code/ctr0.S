.section ".init"
.align 4

.global  _start
.type   _start,@function

.extern _entry
.type _entry, %function


_start:
    lis r1, 0x1ab5
    ori r1, r1, 0xd138
	bl _entry