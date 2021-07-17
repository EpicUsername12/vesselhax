#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "imports.h"

void  (*OSShutdown)(int hi_there);
void  (*OSSleepTicks)(unsigned long long int number_of_ticks);
void  (*__os_snprintf)(char *new, int len, char *format, ...);

void* (*MEMAllocFromDefaultHeapEx)(int size, int align);
void  (*MEMFreeToDefaultHeap)(void *ptr);
void* (*OSAllocFromSystem)(int size, int align);
void  (*OSFreeToSystem)(void *ptr);
void  (*CopyMemory)(void *dest, void *source, unsigned int size);
void  (*SetMemory)(void *dest, unsigned int value, unsigned int size);
void* (*OSEffectiveToPhysical)(void* addr);
void  (*DCFlushRange)(void *buffer, uint32_t length);
void  (*ICInvalidateRange)(void *buffer, uint32_t length);

int  (*IM_Open)();
int  (*IM_Close)(int fd);
int  (*IM_SetDeviceState)(int fd, void *mem, int state, int a, int b);

bool  (*OSCreateThread)(void *thread, void *entry, int argc, void *args, uint32_t stack, uint32_t stack_size, int priority, uint16_t attr);
int   (*OSResumeThread)(void *thread);
int   (*OSSuspendThread)(void *thread);
void  (*OSExitThread)(int c);
void  (*OSJoinThread)(void *thread, int *ret_val);
int  (*OSIsThreadTerminated)(void *thread);
void (*OSYieldThread)(void);
int (*OSRunThread)(OSThread *thread, void *entry, int argc, char **argv);
OSThread* (*OSGetDefaultThread)(int coreId);

void (*OSSavesDone_ReadyToRelease)(void);
void (*OSReleaseForeground)(void);

int (*OSScreenPutFontEx)(int bufferNum, unsigned int posX, unsigned int line, const char* buffer);
int (*OSScreenClearBufferEx)(int bufferNum, uint32_t val);
void (*OSScreenInit)(void);
int (*OSScreenEnableEx)(int n, int a);
int (*OSScreenGetBufferSizeEx)(int bufferNum);
int (*OSScreenSetBufferEx)(int bufferNum, void* addr);
int (*OSScreenFlipBuffersEx)(int bufferNum);

void (*OSExit)(int code);
void (*OSFatal)(const char* txt);
int (*VPADRead)(int controller, VPADData *buffer, unsigned int num, int *error);

void (*GX2Init)(void *arg);
void (*GX2Shutdown)(void);
void (*GX2SetSemaphore)(uint64_t *sem, int action);
void (*GX2Flush)(void);
int  (*GX2DirectCallDisplayList)(void* dl_list, uint32_t size);
int  (*GX2GetMainCoreId)(void);

void(*OSRestartGame)();
void (*OSSendAppSwitchRequest)(int rampid, int a, int b);

int (*_SYSLaunchTitleWithStdArgsInNoSplash)(uint64_t tid, void *ptr);
uint64_t (*_SYSGetSystemApplicationTitleId)(int sysApp);
void (*SYSLaunchMiiStudio)(void);
void (*SYSLaunchMenu)(void);

int (*const OSDynLoad_Acquire)(const char* lib_name, int* out_addr) = (void*)COREINIT(0x0200DFB4);
int (*const OSDynLoad_FindExport)(int lib_handle, int flags, const char* name, void* out_addr) = (void*)COREINIT(0x0200F428);

int (*OSSendMessage)(OSMessageQueue *queue, OSMessage *message, OSMessageFlags flags);
int (*IOS_Open)(const char* device, int mode);
int (*IOS_Ioctl)(int fd, int reqId, void* in_buf, size_t in_len, void* io_buf, size_t io_len);
int (*IOS_Close)(int fd);
void (*OSReport)(const char *fmt, ...);
void*(*OSPhysicalToEffective)(void* addr);

void InitFuncPtrs() {

	int coreinit_handle, vpad_handle, gx2_handle, sys_handle;
	OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
	OSDynLoad_Acquire("vpad.rpl", &vpad_handle);
	OSDynLoad_Acquire("gx2.rpl", &gx2_handle);
	OSDynLoad_Acquire("sysapp.rpl", &sys_handle);

	OSDynLoad_FindExport(coreinit_handle, 0, "OSShutdown", &OSShutdown);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSSleepTicks", &OSSleepTicks);
	OSDynLoad_FindExport(coreinit_handle, 0, "__os_snprintf", &__os_snprintf);

	unsigned int *functionPointer;

	OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &functionPointer);
	MEMAllocFromDefaultHeapEx = (void*)(*functionPointer);

	OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &functionPointer);
	MEMFreeToDefaultHeap = (void*)(*functionPointer);

	OSDynLoad_FindExport(coreinit_handle, 0, "OSAllocFromSystem", &OSAllocFromSystem);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSFreeToSystem", &OSFreeToSystem);
	OSDynLoad_FindExport(coreinit_handle, 0, "memcpy", &CopyMemory);
	OSDynLoad_FindExport(coreinit_handle, 0, "memset", &SetMemory);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSEffectiveToPhysical", &OSEffectiveToPhysical);
	OSDynLoad_FindExport(coreinit_handle, 0, "DCFlushRange", &DCFlushRange);
	OSDynLoad_FindExport(coreinit_handle, 0, "ICInvalidateRange", &ICInvalidateRange);

	OSDynLoad_FindExport(coreinit_handle, 0, "IM_Open", &IM_Open);
	OSDynLoad_FindExport(coreinit_handle, 0, "IM_Close", &IM_Close);
	OSDynLoad_FindExport(coreinit_handle, 0, "IM_SetDeviceState", &IM_SetDeviceState);

	OSDynLoad_FindExport(coreinit_handle, 0, "OSCreateThread", &OSCreateThread);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSResumeThread", &OSResumeThread);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSExitThread", &OSExitThread);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSJoinThread", &OSJoinThread);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSIsThreadTerminated", &OSIsThreadTerminated);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSYieldThread", &OSYieldThread);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSRunThread", &OSRunThread);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSSuspendThread", &OSSuspendThread);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSGetDefaultThread", &OSGetDefaultThread);

	OSDynLoad_FindExport(coreinit_handle, 0, "OSSavesDone_ReadyToRelease", &OSSavesDone_ReadyToRelease);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSReleaseForeground", &OSReleaseForeground);

	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenPutFontEx", &OSScreenPutFontEx);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenClearBufferEx", &OSScreenClearBufferEx);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenInit", &OSScreenInit);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenEnableEx", &OSScreenEnableEx);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenGetBufferSizeEx", &OSScreenGetBufferSizeEx);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenSetBufferEx", &OSScreenSetBufferEx);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenFlipBuffersEx", &OSScreenFlipBuffersEx);

	OSDynLoad_FindExport(coreinit_handle, 0, "exit", &OSExit);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSFatal", &OSFatal);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSSendAppSwitchRequest", &OSSendAppSwitchRequest);

	OSDynLoad_FindExport(vpad_handle, 0, "VPADRead", &VPADRead);

	OSDynLoad_FindExport(gx2_handle, 0, "GX2Init", &GX2Init);
	OSDynLoad_FindExport(gx2_handle, 0, "GX2Shutdown", &GX2Shutdown);
	OSDynLoad_FindExport(gx2_handle, 0, "GX2SetSemaphore", &GX2SetSemaphore);
	OSDynLoad_FindExport(gx2_handle, 0, "GX2Flush", &GX2Flush);
	OSDynLoad_FindExport(gx2_handle, 0, "GX2DirectCallDisplayList", &GX2DirectCallDisplayList);
	OSDynLoad_FindExport(gx2_handle, 0, "GX2GetMainCoreId", &GX2GetMainCoreId);

	OSDynLoad_FindExport(coreinit_handle, 0, "OSRestartGame", &OSRestartGame);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSSendMessage", &OSSendMessage);

	OSDynLoad_FindExport(coreinit_handle, 0, "IOS_Open", &IOS_Open);
	OSDynLoad_FindExport(coreinit_handle, 0, "IOS_Ioctl", &IOS_Ioctl);
	OSDynLoad_FindExport(coreinit_handle, 0, "IOS_Close", &IOS_Close);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSReport", &OSReport);
	OSDynLoad_FindExport(coreinit_handle, 0, "__OSPhysicalToEffectiveCached", &OSPhysicalToEffective);

	OSDynLoad_FindExport(sys_handle, 0, "_SYSGetSystemApplicationTitleId", &_SYSGetSystemApplicationTitleId);
	OSDynLoad_FindExport(sys_handle, 0, "_SYSLaunchTitleWithStdArgsInNoSplash", &_SYSLaunchTitleWithStdArgsInNoSplash);
	OSDynLoad_FindExport(sys_handle, 0, "SYSLaunchMiiStudio", &SYSLaunchMiiStudio);
	OSDynLoad_FindExport(sys_handle, 0, "SYSLaunchMenu", &SYSLaunchMenu);
}