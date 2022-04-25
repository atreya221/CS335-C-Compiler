.text
.globl printi
printi:
	li $v0, 1
	syscall
	jr $ra
	
.text
.globl scani 
scani:
	li $v0, 5
	syscall
	jr $ra

.text
.globl printc
printc:
	li $v0, 11
	syscall
	jr $ra

.text
.globl scanc
scanc:
	li $v0, 12
	syscall
	move $s0, $v0
	li $v0, 12
	syscall
	move $v0, $s0
	jr $ra
	
.text
.globl prints
prints:
	li $v0, 4
	syscall
	jr $ra
	
.text
.globl scans
scans:
	bltz $a1 scan_err
	li $v0, 8
	syscall
	jr $ra
scan_err:
	la $a0 scan_err_str	
	li $v0, 4
	syscall
	jr $ra
	
	.data 
scan_err_str: .asciiz "Input size cannot be negative!\n"
 	
 	.text
 	.globl sbrk
sbrk:
	li $v0 , 9
	syscall
	jr $ra


 	.text
 	.globl exit
exit:
	li $v0 , 10
	syscall
	jr $ra

