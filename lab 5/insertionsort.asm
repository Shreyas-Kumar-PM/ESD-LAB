        AREA RESET, DATA, READONLY
        EXPORT __Vectors

__Vectors
        DCD 0x10001000       ; initial stack pointer
        DCD Reset_Handler    ; reset vector
        ALIGN

        AREA mycode, CODE, READONLY
        ENTRY
        EXPORT Reset_Handler

Reset_Handler
        LDR R4, =ARR         ; base address of array
        LDR R5, =SIZE        ; address of size
        LDR R5, [R5]         ; R5 = number of elements
        BL INSERTION_SORT

STOP    B STOP               ; infinite loop

INSERTION_SORT
        PUSH {R0-R7, LR}
        MOV R6, #1           ; i = 1

SORT_OUTER
        CMP R6, R5
        BGE SORT_DONE        ; if i >= size ? done

        ; key = A[i]
        LSL R1, R6, #2       ; offset = i*4
        ADD R0, R4, R1       ; R0 = &A[i]
        LDR R2, [R0]         ; R2 = key

        SUB R3, R6, #1       ; j = i-1

SORT_INNER
        CMP R3, #0xFFFFFFFF  ; while j >= 0
        BLT SORT_INSERT

        LSL R1, R3, #2
        ADD R0, R4, R1       ; R0 = &A[j]
        LDR R1, [R0]         ; R1 = A[j]
        CMP R1, R2
        BLE SORT_INSERT      ; if A[j] <= key ? break

        ADD R7, R0, #4       ; &A[j+1]
        STR R1, [R7]         ; A[j+1] = A[j]

        SUB R3, R3, #1       ; j--
        B SORT_INNER

SORT_INSERT
        ADD R3, R3, #1
        LSL R1, R3, #2
        ADD R0, R4, R1       ; &A[j+1]
        STR R2, [R0]         ; A[j+1] = key

        ADD R6, R6, #1       ; i++
        B SORT_OUTER

SORT_DONE
        POP {R0-R7, LR}
        BX LR
		
        AREA mydata, DATA, READWRITE
ARR     DCD 7, 3, 9, 1, 5, 2   ; unsorted array
SIZE    DCD 6                  ; number of elements

        END
