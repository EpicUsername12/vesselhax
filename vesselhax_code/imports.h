#ifndef IMPORTS_H
#define IMPORTS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/* stolen from dynamic_libs, thanks Maschell lmaooo */
#define BUS_SPEED                       248625000
#define SECS_TO_TICKS(sec)              (((unsigned long long)(sec)) * (BUS_SPEED/4))
#define MILLISECS_TO_TICKS(msec)        (SECS_TO_TICKS(msec) / 1000)
#define MICROSECS_TO_TICKS(usec)        (SECS_TO_TICKS(usec) / 1000000)
#define os_usleep(usecs)                OSSleepTicks(MICROSECS_TO_TICKS(usecs))
#define os_sleep(secs)                  OSSleepTicks(SECS_TO_TICKS(secs))

#define COREINIT(x) (x - 0xFE3C00) // From IDA

typedef struct OSThread OSThread;
typedef struct OSThreadLink OSThreadLink;
typedef struct OSThreadQueue OSThreadQueue;
typedef struct OSThreadSimpleQueue OSThreadSimpleQueue;

struct OSThreadLink
{
	OSThread *next;
	OSThread *prev;
};

struct OSThreadQueue
{
	OSThread *head;
	OSThread *tail;
	void *parent;
	uint32_t unk;
};

struct OSThreadSimpleQueue
{
	OSThread *head;
	OSThread *tail;
};

extern int   (*OSSuspendThread)(void *thread);

typedef struct OSFastMutex OSFastMutex;
typedef struct OSFastMutexQueue OSFastMutexQueue;
typedef struct OSMutex OSMutex;
typedef struct OSMutexQueue OSMutexQueue;
typedef struct OSThread OSThread;

typedef uint8_t OSThreadState;
typedef uint32_t OSThreadRequest;
typedef uint8_t OSThreadAttributes;

typedef int (*OSThreadEntryPointFn)(int argc, const char **argv);
typedef void (*OSThreadCleanupCallbackFn)(OSThread *thread, void *stack);
typedef void (*OSThreadDeallocatorFn)(OSThread *thread, void *stack);

enum OS_THREAD_STATE
{
	OS_THREAD_STATE_NONE             = 0,
	OS_THREAD_STATE_READY            = 1 << 0,
	OS_THREAD_STATE_RUNNING          = 1 << 1,
	OS_THREAD_STATE_WAITING          = 1 << 2,
	OS_THREAD_STATE_MORIBUND         = 1 << 3,
};

enum OS_THREAD_REQUEST
{
	OS_THREAD_REQUEST_NONE           = 0,
	OS_THREAD_REQUEST_SUSPEND        = 1,
	OS_THREAD_REQUEST_CANCEL         = 2,
};

enum OS_THREAD_ATTRIB
{
	OS_THREAD_ATTRIB_AFFINITY_CPU0   = 1 << 0,
	OS_THREAD_ATTRIB_AFFINITY_CPU1   = 1 << 1,
	OS_THREAD_ATTRIB_AFFINITY_CPU2   = 1 << 2,
	OS_THREAD_ATTRIB_AFFINITY_ANY    = ((1 << 0) | (1 << 1) | (1 << 2)),
	OS_THREAD_ATTRIB_DETACHED        = 1 << 3,
	OS_THREAD_ATTRIB_STACK_USAGE     = 1 << 5
};

struct OSMutexQueue
{
	OSMutex *head;
	OSMutex *tail;
	void *parent;
	uint32_t unk;
};

struct OSFastMutexQueue
{
	OSFastMutex *head;
	OSFastMutex *tail;
};

typedef struct OSContext {

	uint64_t tag;
	uint32_t gpr[32];
	uint32_t cr;
	uint32_t lr;
	uint32_t ctr;
	uint32_t xer;
	uint32_t srr0;
	uint32_t srr1;
	uint32_t dsisr;
	uint32_t dar;
	char a[12];
	uint32_t fpscr;
	double fpr[32];
	uint16_t spinLockCount;
	uint16_t state;
	uint32_t gqr[8];
	char b[4];
	double psf[32];
	uint64_t coretime[3];
	uint64_t starttime;
	uint32_t error;
	char c[4];
	uint32_t pmc1;
	uint32_t pmc2;
	uint32_t pmc3;
	uint32_t pmc4;
	uint32_t mmcr0;
	uint32_t mmcr1;

} OSContext;

typedef bool (*OSExceptionCallbackFn)(OSContext *context);

struct OSThread
{
	OSContext context;
	uint32_t tag;

	OSThreadState state;
	OSThreadAttributes attr;

	uint16_t id;
	int suspendCounter;
	int priority;
	int basePriority;
	int exitValue;
	char unk4[0x35C - 0x338];

	OSThreadQueue *queue;
	OSThreadLink link;
	OSThreadQueue joinQueue;

	OSMutex *mutex;
	OSMutexQueue mutexQueue;

	OSThread *next;
	OSThread *prev;

	void *stackStart;
	void *stackEnd;
	OSThreadEntryPointFn entryPoint;

	char unk3[0x57c - 0x3a0];
	void *specific[0x10];
	char unk2[0x5c0 - 0x5bc];

	const char *name;
	uint32_t unk1;

	void *userStackPointer;
	OSThreadCleanupCallbackFn cleanupCallback;
	OSThreadDeallocatorFn deallocator;
	bool cancelState;

	OSThreadRequest requestFlag;
	int needSuspend;
	int suspendResult;
	OSThreadQueue suspendQueue;

	char unk0[0x6a0 - 0x5f4];
};

typedef struct OSMutex {

	uint32_t tag;
	const char *name;
	uint32_t unk;
	OSThreadQueue queue;
	OSThread *owner;
	int32_t count;
	void *next;
	void *prev;

} OSMutex;

extern void (*OSShutdown)(int hi_there);
extern void (*OSSleepTicks)(unsigned long long int number_of_ticks);

extern void* (*MEMAllocFromDefaultHeapEx)(int size, int align);
extern void  (*MEMFreeToDefaultHeap)(void *ptr);
extern void*(*OSAllocFromSystem)(int size, int align);
extern void (*OSFreeToSystem)(void *ptr);
extern void (*CopyMemory)(void *dest, void *source, unsigned int size);
extern void (*SetMemory)(void *dest, unsigned int value, unsigned int size);
extern void*(*OSEffectiveToPhysical)(void* addr);
extern void (*DCFlushRange)(void *buffer, uint32_t length);
extern void (*ICInvalidateRange)(void *buffer, uint32_t length);
extern void (*OSSendAppSwitchRequest)(int rampid, int a, int b);
extern int(*IM_Open)();
extern int(*IM_Close)(int fd);
extern int(*IM_SetDeviceState)(int fd, void *mem, int state, int a, int b);

extern bool (*OSCreateThread)(void *thread, void *entry, int argc, void *args, uint32_t stack, uint32_t stack_size, int priority, uint16_t attr);
extern int  (*OSResumeThread)(void *thread);
extern void (*OSExitThread)(int c);
extern void (*OSJoinThread)(void *thread, int *ret_val);
extern int  (*OSIsThreadTerminated)(void *thread);
extern void (*OSYieldThread)(void);

extern void (*OSSavesDone_ReadyToRelease)(void);
extern void (*OSReleaseForeground)(void);

extern int (*_SYSLaunchTitleWithStdArgsInNoSplash)(uint64_t tid, void *ptr);
extern uint64_t (*_SYSGetSystemApplicationTitleId)(int sysApp);
extern void (*SYSLaunchMiiStudio)(void);
extern void (*SYSLaunchMenu)(void);

extern void(*OSRestartGame)(int argc, char* argv);
extern int (*OSScreenPutFontEx)(int bufferNum, unsigned int posX, unsigned int line, const char* buffer);
extern int (*OSScreenClearBufferEx)(int bufferNum, uint32_t val);
extern void (*OSScreenInit)(void);
extern int (*OSScreenEnableEx)(int n, int a);
extern int (*OSScreenGetBufferSizeEx)(int bufferNum);
extern int (*OSScreenSetBufferEx)(int bufferNum, void* addr);
extern int (*OSScreenFlipBuffersEx)(int bufferNum);

extern void(*OSExit)(int code);
extern void (*OSFatal)(const char* txt);

typedef struct OSMessage OSMessage;
typedef struct OSMessageQueue OSMessageQueue;

typedef enum OSMessageFlags
{
	OS_MESSAGE_FLAGS_NONE            = 0,
	OS_MESSAGE_FLAGS_BLOCKING        = 1 << 0,
	OS_MESSAGE_FLAGS_HIGH_PRIORITY   = 1 << 1,
} OSMessageFlags;


struct OSMessage
{
	void *message;
	uint32_t data0;
	uint32_t data1;
	uint32_t data2;
};

#define OS_MESSAGE_QUEUE_TAG 0x6D536751u

struct OSMessageQueue
{
	uint32_t tag;
	const char *name;
	uint32_t unk;
	OSThreadQueue sendQueue;
	OSThreadQueue recvQueue;
	OSMessage *messages;
	uint32_t size;
	uint32_t first;
	uint32_t used;
};

extern int (*OSSendMessage)(OSMessageQueue *queue, OSMessage *message, OSMessageFlags flags);

extern int OSDriver_Register(char* name, size_t name_size, void* buffer1, void* buffer2);
extern int OSDriver_CopyToSaveArea(char* name, size_t name_size, void* buffer, size_t buffer_size);
extern int KernelMemcpy(void* dst, void* src, size_t len);
extern int SC_KernelCopyData(void* dst, void* src, size_t len);
extern int Syscall_0x26();

/* dynamic_libs/vpad_functions.h */
#define BUTTON_A        0x8000
#define BUTTON_B        0x4000
#define BUTTON_X        0x2000
#define BUTTON_Y        0x1000
#define BUTTON_LEFT     0x0800
#define BUTTON_RIGHT    0x0400
#define BUTTON_UP       0x0200
#define BUTTON_DOWN     0x0100
#define BUTTON_ZL       0x0080
#define BUTTON_ZR       0x0040
#define BUTTON_L        0x0020
#define BUTTON_R        0x0010
#define BUTTON_PLUS     0x0008
#define BUTTON_MINUS    0x0004
#define BUTTON_HOME     0x0002
#define BUTTON_SYNC     0x0001

typedef struct
{
    float x,y;
} Vec2D;

typedef struct
{
    uint16_t x, y;               /* Touch coordinates */
    uint16_t touched;            /* 1 = Touched, 0 = Not touched */
    uint16_t validity;           /* 0 = All valid, 1 = X invalid, 2 = Y invalid, 3 = Both invalid? */
} VPADTPData;
 
typedef struct
{
    uint32_t btn_hold;           /* Held buttons */
    uint32_t btn_trigger;        /* Buttons that are pressed at that instant */
    uint32_t btn_release;        /* Released buttons */
    Vec2D lstick, rstick;        /* Each contains 4-byte X and Y components */
    char unknown1c[0x52 - 0x1c]; /* Contains accelerometer and gyroscope data somewhere */
    VPADTPData tpdata;           /* Normal touchscreen data */
    VPADTPData tpdata1;          /* Modified touchscreen data 1 */
    VPADTPData tpdata2;          /* Modified touchscreen data 2 */
    char unknown6a[0xa0 - 0x6a];
    uint8_t volume;
    uint8_t battery;             /* 0 to 6 */
    uint8_t unk_volume;          /* One less than volume */
    char unknowna4[0xac - 0xa4];
} VPADData;

extern int (*VPADRead)(int controller, VPADData *buffer, unsigned int num, int *error);

extern void (*GX2Init)(void *arg);
extern void (*GX2Shutdown)(void);
extern void (*GX2SetSemaphore)(uint64_t *sem, int action);
extern void (*GX2Flush)(void);
extern int  (*GX2DirectCallDisplayList)(void* dl_list, uint32_t size);
extern int  (*GX2GetMainCoreId)(void);

extern int (*OSRunThread)(OSThread *thread, void *entry, int argc, char **argv);
extern OSThread* (*OSGetDefaultThread)(int coreId);

extern int (*IOS_Open)(const char* device, int mode);
extern int (*IOS_Ioctl)(int fd, int reqId, void* in_buf, size_t in_len, void* io_buf, size_t io_len);
extern int (*IOS_Close)(int fd);
extern void (*OSReport)(const char *fmt, ...);
extern void*(*OSPhysicalToEffective)(void* addr);

void InitFuncPtrs();

extern void(*__os_snprintf)(char *new, int len, char *format, ...);
extern int (*const OSDynLoad_Acquire)(const char* lib_name, int* out_addr);
extern int (*const OSDynLoad_FindExport)(int lib_handle, int flags, const char* name, void* out_addr);

#endif