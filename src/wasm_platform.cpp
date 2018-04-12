
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
  umm AllocationSize = RequestedBytes + sizeof(memory_arena);
  u8 *Bytes = (u8*)malloc(AllocationSize);
  if (!Bytes)
  {
    Assert(!"Unknown error allocating virtual memory!");
  }

#if 1
  for (umm ByteIndex = 0;
      ByteIndex < AllocationSize;
      ++ByteIndex)
  {
    Bytes[ByteIndex] = 0;
  }
#endif

  memory_arena *NewArena = (memory_arena*)Bytes;

  NewArena->FirstFreeByte = (u8*)(Bytes) + sizeof(memory_arena);
  NewArena->Remaining = RequestedBytes;
  NewArena->TotalSize = AllocationSize;
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
  Debug("Can we even get cycle counts?");
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
  Info("Creating Context");

  // Context configurations
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.alpha =
  attr.stencil =
  attr.preserveDrawingBuffer =
  attr.preferLowPowerToHighPerformance =
  attr.failIfMajorPerformanceCaveat = 0;

  attr.enableExtensionsByDefault = 1;
  attr.premultipliedAlpha = 0;
  attr.antialias = 1;
  attr.depth = 1;

  // Looks like on my machine we get 2.0 at maximum (FF 59.0)
  attr.majorVersion = 2;
  attr.minorVersion = 0;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context(0, &attr);
  emscripten_webgl_make_context_current(ctx);

  b32 Result = ctx ? True : False;
  Os->Window = Result;

  return Result;
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
  timespec Time;
  clock_gettime(CLOCK_MONOTONIC, &Time);

  r64 SecondsAsMs = (r64)Time.tv_sec*1000.0;
  r64 NsAsMs = Time.tv_nsec/1000000;

  r64 Ms = SecondsAsMs + NsAsMs;
  Assert(Ms);
  return Ms;
}

b32
ProcessOsMessages(os *Os, platform *Plat)
{
  return 0;
}

inline void
BonsaiSwapBuffers(os *Os)
{
  return;
}

inline void
Terminate(os *Os)
{
  NotImplemented();
  return;
}
