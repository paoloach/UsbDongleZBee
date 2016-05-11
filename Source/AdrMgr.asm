
BANKED_ENTER_XDATA (A-->data,
0x0461	ADD A.XSP(L)
		XCH A,DPL
		PUSH A
		MOV A,XSP(H)
		ADDC A,#0xFF
		XCH A,DPH
		PUSH A
		MOV A,XSP(L)
		CLR C
		SUBB A,DPL
		ADD A,#0xF8
		JBC L1
		MOV XSP(H),DPH
		MOV XSP(L),DPL
		SJMP L2
L1:
		MOV XSP(H),DPH
		MOB XSP(L),DPL
		SETB 0xAF
L2:
		XCH A,R6
		MOVX @DPTR,A
		INC DPTR
		MOV A,VB
		MOVX @DPTR,A
		MOV R0,#0x08
L3
		MOV A,@R0
		INC R0
		MOVX @DPTR,A
		INC DPTR
		DJNZ R6,L3
		MOV A,R7
		MOVX @DPTR,A
		INC DPTR

		MOV A,SP
		ADD A,#0xFA
		MOV RO,A
		MOV A,@R0
		INC R0
		MOV @DPTR,A
		INC DPTR

		MOV A,@R0
		INC R0
		MOVX @DPTR,A
		INC DPTR

		MOV A,@R0
		INC R0
		MOVX @DPTR,A
		INC DPTR

		INC R0
		INC R0
		MOV A,@R0
		INC R0
		MOVX @DPTR,A
		INC DPTR

		MOV A,@R0
		INC R0
		MOVX @DPTR,A
		INC DPTR

		DEC SP
		DEC SP
		POP A
		MOV R6,A
		POP A
		MOV R0,A
		DEC SP
		DEC SP
		DEC SP
		MOV A,R0
		PUSH A
		MOV A,R6
		PUSH A
		RET
ADD_XSTACK_DISP0_8
0x092C:
		ADD		A,XSP(L)
		MOV		DPL,A
		JBC		0xAF,0x93B
		MOV		XSP(L),A
		JC		0x943
		DEC		XSP(H)
		SJMP	0x0943
0x093B	MOV		XSP(L),A
		JC		0x0941
		DEC		XSP(H)
0x0941	SETB	0xAF
0x0943	MOV		DPH,XSP(H)
0x0946	RET
		
PUSH_XSTACK_I_TWO:	
0x096A
		MOV		A,2
		SJMP	PUSH_XSTACK_I
PUSH_XSTACK_I_ONE
		MOV		A,1
PUSH_XSTACK_I
0x0970:
		PUSH	A
		CPL		A
		INC		A
		LCALL	ADD_XSTACK_DISPO0_8
0x0977	POP		A		
		CALL	MOVE_LONG8_XDATA_IDATA
0x097C	RET

MOVE_LONG8_XDATA_IDATA
0x0B9F:
		XCH		A,R2
		PUSH	A
0x0BA2:	MOV		A,@R0
		MOV		@DPTR,A
		INC		DPTR
		INC		R0
		DJNZ	R2,0x0BA2
		POP		A
		MOV		R2,A
0x0BAB	RET

0x0BDE:	XSTACK_DISP0_8 (LOAD DPTR with XSP + A)
		ADD		A,XSP(L)
		MOV		DPL,A
		CLR		A
		ADDC	A,XSP(H)
		MOV		DPH,A
		RET
		
0x090C: BRET_FF
		POP		CBANK
		RET
0x090F: BDISPATCH_FF
		POP		DPH
		POP		DPL
		PUSH	CBANK
		CLR		A
		MOVC	A,@A+DPTR
		PUSH	A
		MOV		A,1
		MOVC	A,@A+DPTR
		PUSH	A
		MOV		A,2
		MOVC	A,@A+DPTR
		MOV		CBANK,A
		RET
AddrMgrEntryAdd (R3:R2 --> AddrMgrEntry_t)
0x2A45:	MOV		A,-12
		LCALL	BANKED_ENTER_XDATA (DPTR = SPX, SPX = SPX-12)
		CALL	0x3125 (put the field index of r3:r2 to 0xFFFE and r7:r6 = r3:r2)
		MOV		R1,0
		MOV		DPL,R6
		MOV		DPH,R7
		MOV		A,@DPTR
		JZ		0x2AD3	(if the user of r3:r2 is dafault, jump)
0x2A56	MOV		V2,R1
		MOV		V3,R1
		SJMP	0x2ACB


0x2ACB	JMP	0x50CB
AddrMgrEntryUpdateEx: (R3:R2 --> AddrMgrEntry_t)
0x2Ad6:	MOV		A,-11
		LCALL	BANKED_ENTER_XDATA (DPTR=SPX, SPX = SPX-8)
		MOV		A,R2
		MOV		R6,A
		MOV		A,R3
		MOV		R7,A
		r7:r6 == r3:r2 == AddrMgrEntry_t passato come argomento
		
		MOV		V2,1
		MOV		A,#11
		LCALL	XSTACK_DISP0_8
		LCALL	0x3A32 (mette in v1:v0 in ccntenuto di dptr, ovvero il contenuto di SPX all'inizio della funzione)
0x2AEA	MOV		A,R4
		ORL		A,R5
		JZ		0x2B03
		MOV		A,R4
		LCALL	0x2B8E
		ADCC	A,R5
		LCALL	0x3133
		MOV		R1,#1
		LCALL	AddrMgrEntrySet
		MOV		A,V0
		ORL		A,V1
		JZ		0x2B4D
0x2B01	SJMP	0x2B26
0x2B03	MOV		A,V0
		ORL		A,V1
		JZ		0x2B4D
		LCALL	0x2B8C
		ADDC	A,V1
		LCALL	0x3133
		LCALL	0x2B82
		LCALL	AddrMgrExtAddrValid
		MOV		A,R1
		XRL		A,#1
		JNZ		0x2B37
		MOV		A,R6
		MOV		R2,A
		MOV		A,R7
		MOV		R3,A
0x2B20	LCALL	AddrMgrEntryAdd
		MOV		A,R1
		MOV		V2,A
		MOV		DPL,V0
		MOV		DPH,V1
		LCALL	0x2FDA
		MOV		R1,#2
		MOV		R2,V0
		MOV		R3,V1
0x2B35	SJMP	0x2B48
		LCALL	0x3063
		LCALL	AddrMgrExtAddrValid
		MOV		A,R1
		XRL		A,#1
		JNZ		0x2B76
		MOV		R1,#3
		MOV		A,R6
		MOV		R2,A
		MOV		A,R7
		MOV		R3,A
		LCALL	AddrMgrEntrySet
		SJMP	0x2b76
		// Controlla se network id dell'argomento passato è 0xFFFE
0x2B4D: MOV		DPL,r6
		MOV		DPH,R7
		INC		DPTR
		MOVX	A,@DPTR
		XRL		A,#0xFE
		JNZ		0x2B5A
		INC		DPTR
		MOV		A,@DPTR
		CPL		A
0x2B5a  JNZ		0x2B67
		// il netkwork id dell'argomento è 0xFFFE
		LCALL	0x3063
		LCALL	AddrMgrExtAddrValid
		MOV		A,R1
		XRL		A,#1
		JNZ		0x2B73
0x2B67: // il netkwork id dell'argomento è diverso da 0xFFFE	
		// mette in R3:R2 l'indirizzo della struttura AddrMgrEntry_t passata come argomento
		MOV		A,R6
		MOV		R2,A
		MOV		A,R7
		MOV		R3,A
0x2B6B	CALL	AddrMgrEntryAdd
		MOV		A,R1
		MOV		V2,A
		SJMP	0x2B76
0x2B73	MOV 	V2,A
0X2B76	LJMP	0X359D
0X2B79	MOV		A,V2
		
0x2B84: ADD		A,3 (V1:A pointer, R3:R2 pointer +3) 
		MOV		R2,A
		CLR		A
		ADDC	A,V1
		MOV		R3,A
0x2B8B	RET
		
		
0x2D4B: (verifica se R7:r6 vale quanto il numero massimo di elementi di AddrMgr)
		MOV		DPTR,0x33C (numero di element di AddrMgr)
		LCALL	0x4A6C (load r1:r0 il valore puntato da DPTR)
		CLR		C
		MOV		A,R6
		SUBB	A,R0
		MOV		A,R7
		SUBB	A,R1
		RET
		
0x2D4F:	AddrMgrExtAddrSet(r2:r3 -->src,r4:r5 -->dst) 
		PUSH	DPL
		PUSH	DPH
		MOV		A,R4
		ORL		A,R5
		JZ		L1
		LCALL	sAddrExtCpy
		SJMP	L2
L1:
		MOV		R4,8
		MOV		R5,0
		MOV		R1,0
		LCALL	osal_memset
L2
0x2D65	LJMP	0x45F2
0x2D68: AddrMgrExtAddrValid (r3:r2 indirizzo da verificare)
verifica che gli 8 byte puntatu da r3:r2 siano diversi da 0 (r1=1). Se sono tutti zeri R1=0
		MOV		A,#-10
		LCALL	BANKED_ENTER_XDATA
		MOV		R1,#0
		MOV		A,R2
		ORL		A,R3
		JZ		L1
		MOV		R0,#0
L3
0x2D75	MOV		A,R2
		ADD		A,R0
		MOV		DPL,A
		CLR		A
		CALL	5e84 (carica in A il valore di R3:DPL)
		JZ		L2
		MOV		R1,#1
		MOV		R0,#8
L2:
		INC		R0
		MOV		A,R0
		CLR		C
		SUBB	A,#8
		JC		L3
L3:
0x2D8A	JMP 	0x34A8


0X2E4B	MOV		A,-9
		CALL	BANKED_ENTER_XDATA
		MOV		A,R2
		MOV		R6,A
		MOV		A,R3
		MOV		R7,A
		MOV		V0,#0
		DPL		R6
		DPH		R7
		MOVX	A,@DPTR
		JZ		0X2E88
		LCALL	AddrMgrEntryValidate
		MOV		A,R1
		MOV		V0,A
		MOV		A,#1
		XRL		A,V0
		JNZ		0X2E88
		LCALL	0X2E8B
		MOV		R2,A
		MOV		DPL,R0
		MOV		DPH,R1
		MOVX	A,@DPTR
		ANL		A,R2
		JNZ		0X2E88
		MOV		DPL,R6
		MOV		DPH,R7
		MOVX	A,@DPTR
		ORL		A,#0X80
		MOV		R2,A
		MOV		DPL,R0
		MOV		DPH,R1
		MOVX	A,@DPTR
		ORL		A,R2
		MOVX	@DPTR,A
		LCALL	AddrMgrWriteNVRequest
0X2E88	LJMP	0X41D7
		
		
		
		
		
		
		
		
0x2F46  AddrMgrEntryLookupExt(r3:r2 -->AddrMgrEntry_t* entry)
		MOV		A,#-13
		LCALL	BANKED_ENTER_XDATA
		MOV		V0,r2
		MOV		V1,R3
		MOV		DPL,R2
		MOV		DPH,R3
0x2F53	LCALL	0x2FD2 (Set the nkwaddress of entry as -2 )
		MOV		A,R2
		LCALL	0x2FFC  (assegna ad index=-2)
		MOV		V4,0
		MOV		A,R2
		CALL	0x2B7C (R2:R3 punta a extAddr)
		CALL	AddrMgrExtAddrValid
		MOV		A,R1
		XRL		A,#1
		JNZ 	0X2FD3
		MOV		R6,#0
		MOV		R7,#0
		SJMP	0x2FC6
		
		r7:r6 --> counter 2 bytes
0x2F67:
		MOV		A,R6
		MOV		RO,A
		MOV		A,R7
		MOV		R1,A
0x2F6b:	MOV		A,R0
		MOV		B,#11
		MUL		AB
		MOV		R0,A
		MOV		R2,B
		MOV		B,#11
		MOV		A,R1
		MUL		AB
		MOV		R1,A
		R2:R1 --> r7:r6 * 11
		
		
		MOV		DPTR,@0x33E
		CALL	0x73e9 (mette in dptr il valore puntato da dptr + r2:r1)
		MOVX	A,@DPTR
		ANL		A,#0x0F
		JZ		0x2FC9
0x2F85	MOV		A,r6
		
		
0x2FD2: INC		DPTR
		MOV		A,#0xFE
		MOVX	@DPTR,A
		INC		DPTR
		INC		A
		MOVX	@DPTR,A
0x02Fd9	RET
		
		
0x2FC9:
		INC		R6
		MOV		A,R6
		JNZ		0x2FCE
		INC		R7
0x2FC6: LCALL	0x2D4B ((verifica se R7:r6 vale quanto il numero massimo di elementi di AddrMgr))		
		JC		0x2F6F
		
		// testati tutti gli elementi
0X2FD3  MOV		R1,V4
		MOV 	R7,5
		LJMP	BANKED_LEAVE_XDATA
0x2FDA:	INC		DPTR
		MOV		A,-2
		MOVX	@DPTR,A
		INC		DPTR
		INC		A
		MOVX	@DPTR,A
		RET
		
0x2FFC: LCALL	0x3006 (mette in DPTR l'indirizzo di index)
		MOV		A,-2
		MOVX	@DPTR,A
		INC		DPTR
		INC		A
		MOV		XDPTR,A
0x3005	RET
0x3006: LCALL	0x657E (DPTR punta a index)
		ADDC	A,V1
		MOV		DPH,A
0x300D	RET


AddrMgrEntryUpdate (R3:R2 -> AddrMgrEntry_t src)
0x3073	MOV A, #-14
		call BANKED_ENTER_XDATA
		MOV A, #0xE6
		call ALLOC_STACK
		call	0X3125 (R2:R3 --> pointer dst.index=-2, R7:R6 = R3:R2)
		MOV V4,1
		mov V0,0
		mov v1,0
		mov v2,0
		mov v3,0
		
		MOV		DPL,XSP(L)
		MOV		DPH,XSP(H)
		
		CLR		A
		MOVX	@DPTR,A// user=0
		
		MOV		A,R6
		ADD		A,3
		MOV		R4,A
		CLR		A
		ADDC	A,R7
		MOV		R5,A   R4:R5 --> extAddr

// LOAD r2:r3 with extAddr
		MOV 	A,3
		CALL	XSTACK_DISP0_8
0x3098 	MOV		R2,DPL		
		MOV		R3,DPH
0x30A2	LCALL	AddrMgrExtAddrSet
		
		MOV		R2,XSP(L)
		MOV		R3,XSP(H)
		LCALL	AddrMgrEntryLookupExt
		
		MOV		A,R1
		XRL		A,1
		JNZ		0x30D5
0x30B6	MOV		A,#0x0B







0x30D5:	MOV		A,#0x0D
		LCALL	XSTACK_DISP0_8
		CALL	0x45B4 (*dptr=0, dptr <-r7:r6)
		INC		DPTR
		LCALL	0X45BB	(r1:r0 =*dptr, r1:r0 contiene il network address della struttra AddrMgrEntry_t passata come argomento)
		MOV		A,#0x0E
		LCALL	0x4A4F (*dptr = r1:r0)
		MOV		A,0x0D
		CALL	XSTACK_DISTP101_8
		LCALL	AddrMgrEntryLookupNwk
0x30F1	MOV		A,R1
		XRL		A,1
		JNZ		0x30FF
		MOV		A,#0x0D
		CALL	XSTACK_DISP100_8
		MOV		V2,R0
		MOV		V3,R1
0x30FF:	MOV		R0,#0x0A
		CALL	PUSH_XSTACK_I_TWO (alloca 2 byte nello stack e ci mette i dati della locazione 0A e 0B)
		MOV		R4,V0
		MOV		R5,V1
		MOV		A,R6
		MOV		R2,A
		MOV		A,R7
		MOV		R3,A
		LCALL	AddrMgrEntryUpdateEx
		MOV		A,#2
		LCALL	DEALLOC_XSTACK
		MOV		A,R1
		MOV		V4,A
		MOV		A,R6
		MOV		R2,A
		MOV		A,R7
		MOV		R3,A
		LCALL	AddrMgrEntryAddRef (add entry ref if user != 0)
0X311E	MOV		R1,V4
		MOV		A,#0X1A
		LJMP	0X5DC7
		// input: R3:R2 pointer to AddrMgrEntry_t
		// output: the field index of the argument is put to 0xFFFE
0x3125:	MOV		A,R2
		MOV		R6,A
		MOV		A,R3
		MOV		R7,A   // r7:r7 pointer to AddrMgrEntry_t
		LCALL	0x4C1C // DPTR point to field index of AddrMgrEntry_t passed as argumento (r3:r2)
		MOV		A,0xFE
		MOVX	@DPTR,A
		INC		DPTR
		INC		A
		MOV		@DPTR,A
0x3132	RET


0X359D	MOV		R1,V2
		MOV		R7,3
		LJMP	BANKED_LEAVE_XDATA
		
0x3A32: MOV		A,@DPTR
		MOV		V0,A
		INC		@DPTR
		MOV		A,@DPTR
		MOV		V1,@DPTR
0x3A39	RET

0X41D7	MOV		R1,V0
		LJMP	0X4A85
		
0x4538:
		MOVX	@DPTR,A
		INC		DPTR
		MOV		A,R1
		MOVX	@DPTR,A
0x453C	RET

0x45B4: CLR		A
		MOVX	@DPTR,A
		MOV		DPL,R6
		MOV		DPH,R7
0X45BA	RET
0X45BB	CALL	0X4A6C
0X45BE	RET

0x45F2:
		POP		DPH
		POP		DPL
		LJMP	BRET_FF
		
0x4A4F:
		MOV		A,R0
		LCALL	0x4538
		RET
		
0x4A6C:
		LCALL	0x4A70
		RET
0x4A70:
		MOVX	A,@DPTR
		MOV		R0,A
		INC		DPTR
		MOV		A,@DPTR
		MOVX	R1,A
0x4A75	RET

0X4A85:	MOV		R7,#1
		LJMP	BANKED_LEAVE_XDATA
		

0x4C1C: // DPTR = r7:r6 + 11
		MOV		A,R6
		CALL	0x657E
		ADC		A,R7
		MOV		DPH,A
		RET

0x50CB	MOV		R7,4
		LJMP	BANKED_LEAVE_XDATA
		
0x5DC7	LCALL	DEALLOC_XSTACK8
		MOV		R7,#6
		òJMP	BANKED_LEAVE_XDATA
		
0x5E84:
		ADDC	A,R3
		MOV		DPH,A
		MOVX	A,@DPTR
		RET
		
0x657E:
		ADD		A,#0x0B
		MOV		DPL,A
		CLR		A
		RET
	
	
0x73e9:
		MOVX	A,@DPTR (dptr = *dptr+r2:r1,mette in dptr il valore puntato da dptr + r2:r1)
		ADD		A,R0
		MOV		R2,A
		INC		DPTR
		MOVX	A,@DPTR
		ADDC	A,R1
		MOV		DPL,R2
		MOV		DPH,A
		RET
	
	
0xD5FF:
		MOV		A,R6
		MOV		R2,A
		MOV		A,R7
		MOV		R3,A
		LCALL	AddrMgrExtAddrLookup
		SJMP	0xD65E
		
0x1D5FE	MOV		DPTR,0x2969 (gMAX_NEIGHBOR_ENTRIES)
		CLR		A
		MOVC	A,@A+DPTR
		MOV		R0,A
		MOV		A,V2
		CLR		C
		SUBB	A,R0
		RET
		
nwkNeighborUpdateNwkAddr
	(r5:r4 -> extende address, r3:r2 ->network address
0x1E233	MOV		A,-11
		LCALL	BANKED_ENTER_XDATA
		MOV		V0,R2
		MOV		V1,R3
		MOV		A,R4
		MOV		R6,A
		MOV		A,R5
		MOV		R7,A
		(V1:v2 -> network address, r7:r6 -> extended address)
		MOV		V2,0
		SJMP	0x1E247
0x1E245 INC		V2
0x1E247	LCALL	0xD5FE
		JNC		0x1E26E
		MOV		A,R6
		MOV		R4,A
		MOV		A,R7
		MOV		R5,A
		MOV		A,V2
		LCALL	0x1E357 (r3:r2 = A*17+neighborTable +1)
		LCALL	sAddrExtCmp (ccmp r3:r2 with r5:r4 -> R1==0 se sono diversi)
		MOV		A,R1
0x1E259	JZ		0x1E245
		// FOUND
		MOV		A,V2
		MOV		B,#0x17
		MUL		AB
		MOV		R0,A
		MOV		R1,B
		MOV		A,#0x4D  (0xC4D = neighborTable)
		MOV		A,R0
		MOV		DPL,A
		MOV		A,x0C
		LCALL	0x1EA0E
0x1E26E	JMP		0x1E939

		
0x1E357 MOV		B,0x17
		MUL		AB
		MOV		R0,A
		MOV		R1,B
		MOV		A,#4F
		ADD		A,R0
		MOV		R2,A
		MOV		A,#C
		ADC		A,R1
		MOV		R3,A
		RET

0x1E939 MOV		R7,0x03
		LJMP	BANKED_LEAVE_XDATA

ALLOC_XSTACH
	ADD A,xsp(L)
	JBC	0xAF,L1
	MOV	XSP(L),A
	JC	L2
	DEC	XSP(H)
	SJMP L2
L1:
	MOV	XSP(L),A
	JC	L3
	DEC	XSP(H)
L3:
	SETB	0xAF
L2:
	RET





