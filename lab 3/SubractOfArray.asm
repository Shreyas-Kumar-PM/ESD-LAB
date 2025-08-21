	AREA RESET,DATA,READONLY
	EXPORT __Vectors
__Vectors
	DCD 0x10001000
	DCD Reset_Handler
	ALIGN
	AREA mycode,CODE,READONLY
	ENTRY
	EXPORT Reset_Handler
Reset_Handler
        LDR R0, =N1       
        LDR R1, =N2     
        LDR R2, =Result 

        MOV R3, #4
		
		MOVS R8,#0xFFFFFFFF
		ADCS R8,R8,#1
Loop
        LDR R5, [R0], #4
        LDR R6, [R1], #4
        SBCS R7, R5, R6
        STR R7, [R2], #4
        BNE Loop

STOP
        B STOP
N1      DCD 0xFFFFFFFF, 0x2, 0x3, 0xFFFFFFFF
N2      DCD 0x5, 0x6, 0x7, 0x8
        AREA mydata, DATA, READWRITE
Result  DCD 0, 0, 0, 0
        END