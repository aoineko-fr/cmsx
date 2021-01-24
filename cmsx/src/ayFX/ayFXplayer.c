//-----------------------------------------------------------------------------
//  █▀▀ █▀▄▀█ █▀ ▀▄▀
//  █▄▄ █ ▀ █ ▄█ █ █ v0.2
//-----------------------------------------------------------------------------
// ayFX Replayer
//-----------------------------------------------------------------------------
// Based on ayFX REPLAYER v1.31 by SapphiRe
//  - v1.31		Fixed bug on previous version, only PSG channel C worked
//  - v1.3		Fixed volume and Relative volume versions on the same file, conditional compilation
//				Support for dynamic or fixed channel allocation
//  - v1.2f/r	ayFX bank support
//  - v1.11f/r	If a frame volume is zero then no AYREGS update
//  - v1.1f/r	Fixed volume for all ayFX streams
//  - v1.1		Explicit priority (as suggested by AR)
//  - v1.0f		Bug fixed (error when using noise)
//  - v1.0		Initial release
//-----------------------------------------------------------------------------
#include "ayFXplayer.h"

u8  ayFX_Mode;		// 1	ayFX mode
//	x	x	x	x	x	x	SP	SU
//	7	6	5	4	3	2	1	0	
//							│	└── Update channel switch mode (inc channel at each update)
//							└────── Play channel switch mode (inc channel at each play)

u16 ayFX_Bank;		// 2	Current ayFX Bank
u8  ayFX_Priority;	// 1	Current ayFX stream priotity
u16 ayFX_Pointer;	// 2	Pointer to the current ayFX stream
u16 ayFX_Tone;		// 2	Current tone of the ayFX stream
u8  ayFX_Noise;		// 1	Current noise of the ayFX stream
u8  ayFX_Volume;	// 1	Current volume of the ayFX stream
u8  ayFX_Channel;	// 1	PSG channel to play the ayFX stream
#if (AYFX_RELATIVE)
u16 ayFX_VT;		// 2	ayFX relative volume table pointer
#endif

#define AYREGS _PT3_Regs

//-----------------------------------------------------------------------------
///
void ayFX_Init(void* bank) __FASTCALL
{
	__asm
	ayFX_SETUP::
		// ---          ayFX replayer setup          ---
		// --- INPUT: HL -> pointer to the ayFX bank ---
		ld		(_ayFX_Bank), hl	// Current ayFX bank
		xor		a					// a:=0
		ld		(_ayFX_Mode), a		// Initial mode: fixed channel
		inc		a					// Starting channel (=1)
		ld		(_ayFX_Channel), a	// Updated
	ayFX_END::
		// --- End of an ayFX stream ---
		ld		a, #255				// Lowest ayFX priority
		ld		(_ayFX_Priority), a	// Priority saved (not playing ayFX stream)
	__endasm;
}

//-----------------------------------------------------------------------------
///
void ayFX_Play(u16 snd_prio) __FASTCALL __naked
{
	__asm
	ayFX_INIT::
		// Fastcall init
		ld		a, l
		ld		c, h
		// ---     INIT A NEW ayFX STREAM     ---
		// --- INPUT: A -> sound to be played ---
		// ---        C -> sound priority     ---
		push	bc					// Store bc in stack
		push	de					// Store de in stack
		push	hl					// Store hl in stack
		// --- Check if the index is in the bank ---
		ld		b, a				// b:=a (new ayFX stream index)
		ld		hl, (_ayFX_Bank)	// Current ayFX BANK
		ld		a, (hl)				// Number of samples in the bank
		or		a					// If zero (means 256 samples)...
		jr		z, CHECK_PRI		// ...goto CHECK_PRI
		// The bank has less than 256 samples
		ld		a, b				// a:=b (new ayFX stream index)
		cp		(hl)				// If new index is not in the bank...
		ld		a, #2				// a:=2 (error 2: Sample not in the bank)
		jr		nc, INIT_END		// ...we can't init it
	CHECK_PRI:	
		// --- Check if the new priority is lower than the current one ---
		// ---   Remember: 0 = highest priority, 15 = lowest priority  ---
		ld		a, b				// a:=b (new ayFX stream index)
		ld		a, (_ayFX_Priority)	// a:=Current ayFX stream priority
		cp		c					// If new ayFX stream priority is lower than current one...
		ld		a, #1				// a:=1 (error 1: A sample with higher priority is being played)
		jr		c, INIT_END			// ...we don't start the new ayFX stream
		// --- Set new priority ---
		ld		a, c				// a:=New priority
		and		#0x0F				// We mask the priority
		ld		(_ayFX_Priority), a	// new ayFX stream priority saved in RAM

	#if (AYFX_RELATIVE)
		// --- Volume adjust using PT3 volume table ---
		ld		c, a				// c:=New priority (fixed)
		ld		a, #15				// a:=15
		sub		c					// a:=15-New priority = relative volume
		jr		z, INIT_NOSOUND		// If priority is 15 -> no sound output (volume is zero)
		add		a, a				// a:=a*2
		add		a, a				// a:=a*4
		add		a, a				// a:=a*8
		add		a, a				// a:=a*16
		ld		e, a				// e:=a
		ld		d, #0				// de:=a
		ld		hl, VT_				// hl:=PT3 volume table
		add		hl, de				// hl is a pointer to the relative volume table
		ld		(_ayFX_VT), hl		// Save pointer
	#endif

		// --- Calculate the pointer to the new ayFX stream ---
		ld		de, (_ayFX_Bank)	// de:=Current ayFX bank
		inc		de					// de points to the increments table of the bank
		ld		l, b				// l:=b (new ayFX stream index)
		ld		h, #0				// hl:=b (new ayFX stream index)
		add		hl, hl				// hl:=hl*2
		add		hl, de				// hl:=hl+de (hl points to the correct increment)
		ld		e, (hl)				// e:=lower byte of the increment
		inc		hl					// hl points to the higher byte of the correct increment
		ld		d, (hl)				// de:=increment
		add		hl, de				// hl:=hl+de (hl points to the new ayFX stream)
		ld		(_ayFX_Pointer), hl	// Pointer saved in RAM
		xor		a					// a:=0 (no errors)
	INIT_END:	
		pop		hl					// Retrieve hl from stack
		pop		de					// Retrieve de from stack
		pop		bc					// Retrieve bc from stack
		ret							// Return

	#if (AYFX_RELATIVE)
	INIT_NOSOUND:	// --- Init a sample with relative volume zero -> no sound output ---
		ld		a, #255				// Lowest ayFX priority
		ld		(_ayFX_Priority), a	// Priority saved (not playing ayFX stream)
		jr		INIT_END			// Jumps to INIT_END
	#endif
	__endasm;
}

//-----------------------------------------------------------------------------
///
void ayFX_Update()
{
	__asm
	ayFX_PLAY::
		// --- PLAY A FRAME OF AN ayFX STREAM ---
		ld		a, (_ayFX_Priority)	// a:=Current ayFX stream priority
		or		a					// If priority has bit 7 on...
		ret		m					// ...return
		// --- Calculate next ayFX channel (if needed) ---
		ld		a, (_ayFX_Mode)		// ayFX mode
		and		#1					// If bit0=0 (fixed channel)...
		jr		z, TAKECB			// ...skip channel changing
		ld		hl, #_ayFX_Channel	// Old ayFX playing channel
		dec		(hl)				// New ayFX playing channel
		jr		nz, TAKECB			// If not zero jump to TAKECB
		ld		(hl), #3			// If zero -> set channel 3
	TAKECB:	// --- Extract control byte from stream ---
		ld		hl, (_ayFX_Pointer)	// Pointer to the current ayFX stream
		ld		c, (hl)				// c:=Control byte
		inc		hl					// Increment pointer
		// --- Check if there's new tone on stream ---
		bit		5, c				// If bit 5 c is off...
		jr		z, CHECK_NN			// ...jump to CHECK_NN (no new tone)
		// --- Extract new tone from stream ---
		ld		e, (hl)				// e:=lower byte of new tone
		inc		hl					// Increment pointer
		ld		d, (hl)				// d:=higher byte of new tone
		inc		hl					// Increment pointer
		ld		(_ayFX_Tone), de	// ayFX tone updated
	CHECK_NN:	// --- Check if there's new noise on stream ---
		bit		6, c				// if bit 6 c is off...
		jr		z, SETPOINTER		// ...jump to SETPOINTER (no new noise)
		// --- Extract new noise from stream ---
		ld		a, (hl)				// a:=New noise
		inc		hl					// Increment pointer
		cp		#0x20				// If it's an illegal value of noise (used to mark end of stream)...
		jr		z, ayFX_END			// ...jump to ayFX_END
		ld		(_ayFX_Noise), a	// ayFX noise updated
	SETPOINTER:	// --- Update ayFX pointer ---
		ld		(_ayFX_Pointer), hl	// Update ayFX stream pointer
		// --- Extract volume ---
		ld		a, c				// a:=Control byte
		and		#0x0F				// lower nibble

	#if (AYFX_RELATIVE)
		// --- Fix the volume using PT3 Volume Table ---
		ld		hl, (_ayFX_VT)		// hl:=Pointer to relative volume table
		ld		e, a				// e:=a (ayFX volume)
		ld		d, #0				// d:=0
		add		hl, de				// hl:=hl+de (hl points to the relative volume of this frame
		ld		a, (hl)				// a:=ayFX relative volume
		or		a					// If relative volume is zero...
	#endif

		ld		(_ayFX_Volume), a	// ayFX volume updated
		ret		z					// ...return (don't copy ayFX values in to AYREGS)
		// -------------------------------------
		// --- COPY ayFX VALUES IN TO AYREGS ---
		// -------------------------------------
		// --- Set noise channel ---
		bit		7, c				// If noise is off...
		jr		nz, SETMASKS		// ...jump to SETMASKS
		ld		a, (_ayFX_Noise)	// ayFX noise value
		ld		(AYREGS+6), a		// copied in to AYREGS (noise channel)
	SETMASKS:	// --- Set mixer masks ---
		ld		a, c				// a:=Control byte
		and		#0x90				// Only bits 7 and 4 (noise and tone mask for psg reg 7)
		cp		#0x90				// If no noise and no tone...
		ret		z					// ...return (don't copy ayFX values in to AYREGS)
		// --- Copy ayFX values in to ARYREGS ---
		rrca						// Rotate a to the right (1 TIME)
		rrca						// Rotate a to the right (2 TIMES) (OR mask)
		ld		d, #0xDB			// d:=Mask for psg mixer (AND mask)
		// --- Dump to correct channel ---
		ld		hl, #_ayFX_Channel	// Next ayFX playing channel
		ld		b, (hl)				// Channel counter
	CHK1:		// --- Check if playing channel was 1 ---
		djnz	CHK2				// Decrement and jump if channel was not 1
	PLAY_C:	// --- Play ayFX stream on channel C ---
		call	SETMIXER			// Set PSG mixer value (returning a=ayFX volume and hl=ayFX tone)
		ld		(AYREGS+10), a		// Volume copied in to AYREGS (channel C volume)
		bit		2, c				// If tone is off...
		ret		nz					// ...return
		ld		(AYREGS+4), hl		// copied in to AYREGS (channel C tone)
		ret							// Return
	CHK2: // --- Check if playing channel was 2 ---
		rrc		d					// Rotate right AND mask
		rrca						// Rotate right OR mask
		djnz	CHK3				// Decrement and jump if channel was not 2
	PLAY_B:	// --- Play ayFX stream on channel B ---
		call	SETMIXER			// Set PSG mixer value (returning a=ayFX volume and hl=ayFX tone)
		ld		(AYREGS+9), a		// Volume copied in to AYREGS (channel B volume)
		bit		1, c				// If tone is off...
		ret		nz					// ...return
		ld		(AYREGS+2), hl		// copied in to AYREGS (channel B tone)
		ret							// Return
	CHK3: // --- Check if playing channel was 3 ---
		rrc		d					// Rotate right AND mask
		rrca						// Rotate right OR mask
	PLAY_A:	// --- Play ayFX stream on channel A ---
		call	SETMIXER			// Set PSG mixer value (returning a=ayFX volume and hl=ayFX tone)
		ld		(AYREGS+8), a		// Volume copied in to AYREGS (channel A volume)
		bit		0, c				// If tone is off...
		ret		nz					// ...return
		ld		(AYREGS+0), hl		// copied in to AYREGS (channel A tone)
		ret							// Return
	SETMIXER:	// --- Set PSG mixer value ---
		ld		c, a				// c:=OR mask
		ld		a, (AYREGS+7)		// a:=PSG mixer value
		and		d					// AND mask
		or		c					// OR mask
		ld		(AYREGS+7), a		// PSG mixer value updated
		ld		a, (_ayFX_Volume)	// a:=ayFX volume value
		ld		hl, (_ayFX_Tone)	// ayFX tone value
		ret							// Return

#if (AYFX_RELATIVE)
			// --- UNCOMMENT THIS IF YOU DON'T USE THIS REPLAYER WITH PT3 REPLAYER ---
			// VT_:
				// .INCBIN	"VT.BIN"
			// --- UNCOMMENT THIS IF YOU DON'T USE THIS REPLAYER WITH PT3 REPLAYER ---
#endif
__endasm;
}