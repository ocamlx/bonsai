
inline bool
AtomicCompareExchange( volatile unsigned int *Source, unsigned int Exchange, unsigned int Comparator )
{
  return 0;
}

#if 0
inline void
PrintSemValue( semaphore *Semaphore )
{
  s32 Value;

  s32 E = sem_getvalue(Semaphore, &Value);
  Assert(E==0);

  printf("Value: %d \n", Value);

  return;
}
#endif

u64
PlatformGetPageSize()
{
  return 4096;
}

u8*
PlatformProtectPage(u8* Mem)
{
  NotImplemented();
  return 0;
}

memory_arena*
PlatformAllocateArena(umm RequestedBytes = Megabytes(1))
{
  u8 *Bytes = (u8*)malloc(RequestedBytes);
  if (!Bytes)
  {
    Assert(!"Unknown error allocating virtual memory!");
  }

  memory_arena *NewArena = (memory_arena*)Bytes;

  NewArena->FirstFreeByte = (u8*)(Bytes);
  NewArena->Remaining = RequestedBytes;
  NewArena->TotalSize = RequestedBytes;
  NewArena->NextBlockSize = RequestedBytes * 2;

  return NewArena;
}

inline void
ThreadSleep( semaphore *Semaphore )
{
  NotImplemented();
  return;
}

semaphore
CreateSemaphore(void)
{
  Warn("WASM Doesn't have Semaphores ..");
  return 0;
}

int
GetLogicalCoreCount()
{
  return 1;
}

thread_id
CreateThread( void* (*ThreadMain)(void*), thread_startup_params *Params)
{
  NotImplemented();
  return 0;
}

__inline__ u64
GetCycleCount()
{
  NotImplemented();
  return 0;
}

void
CloseLibrary(shared_lib Lib)
{
  NotImplemented();
  return;
}

inline int
OpenLibrary(const char *filename)
{
  NotImplemented();
  return 0;
}

b32
OpenAndInitializeWindow( os *Os, platform *Plat)
{
  NotImplemented();
  return 0;
}

inline GameCallback
GetProcFromLib(shared_lib Lib, const char *Name)
{
  NotImplemented();
  return 0;
}

char*
GetCwd()
{
  NotImplemented();
  return 0;
}

b32
IsFilesystemRoot(char *Filepath)
{
  NotImplemented();
  return 0;
}

#include <time.h>

inline r64
GetHighPrecisionClock()
{
  NotImplemented();
  return 0;
}

b32
ProcessOsMessages(os *Os, platform *Plat)
{
  NotImplemented();
  return 0;
}

inline void
BonsaiSwapBuffers(os *Os)
{
  NotImplemented();
  return;
}

inline void
Terminate(os *Os)
{
  NotImplemented();
  return;
}
