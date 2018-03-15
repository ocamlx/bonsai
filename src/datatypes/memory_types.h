#include <sys/mman.h>

#define MEMPROTECT_UNDERFLOW (0)
#define MEMPROTECT_OVERFLOW (1)
#define MEMPROTECT (MEMPROTECT_OVERFLOW || MEMPROTECT_UNDERFLOW)

#if MEMPROTECT_UNDERFLOW && MEMPROTECT_OVERFLOW
#error "Unfortunately, Underflow and Overflow protection at the same time is impossible"
#endif

#if MEMPROTECT && (!BONSAI_INTERNAL)
#error "Memory protection only available in internal mode."
#endif

#define registered_memory_arena(Arena) \
  memory_arena *Arena = PlatformAllocateArena(); \
  DEBUG_REGISTER_ARENA(Arena);

inline u64
Kilobytes(u32 Bytes)
{
  u64 Result = Bytes * 1024;
  return Result;
}

inline u64
Megabytes(u32 Number)
{
  u64 Result = Number*Kilobytes(1024);
  return Result;
}

inline u64
Gigabytes(u32 Number)
{
  u64 Result = Number*Megabytes(1024);
  return Result;
}

inline u64
Terabytes(u32 Number)
{
  u64 Result = Number*Gigabytes(1024);
  return Result;
}


struct memory_arena
{
  u8* FirstFreeByte;
  umm Remaining;
  umm TotalSize;
  umm NextBlockSize;

  memory_arena *Prev;

#if BONSAI_INTERNAL
  umm Pushes;

#if MEMPROTECT
  b32 MemProtect = 1;
#endif

#endif
};

#define PUSH_STRUCT_CHECKED(Type, Arena, Number) \
  (Type*)PushStructChecked_( Arena, sizeof(Type)*Number, #Type, __LINE__, __FILE__ );

#if BONSAI_INTERNAL
#define DEBUG_REGISTER_ARENA(Arena) \
  DebugRegisterArena(#Arena, Arena)
#else
#define DEBUG_REGISTER_ARENA(...)
#endif

#if 0
#define SubArena(Src, Dest, Size) \
  SubArena_(Src, Dest, Size); \
  DEBUG_REGISTER_ARENA(#Dest, Dest)

#define AllocateAndInitializeArena(Arena, Size) \
  AllocateAndInitializeArena_(Arena, Size); \
  DEBUG_REGISTER_ARENA(#Arena, Arena)

inline void
SubArena_( memory_arena *Src, memory_arena *Dest, umm Size)
{
  Dest->FirstFreeByte = (u8*)PushSize(Src, Size);
  Dest->Remaining = Size;
  Dest->TotalSize = Size;

  Assert(Dest->FirstFreeByte);

  return;
}

inline void
AllocateAndInitializeArena_(memory_arena *Arena, umm Size)
{
  Arena->Remaining = Size;
  Arena->TotalSize = Size;

  Arena->FirstFreeByte = Allocate(Arena->Remaining);
  Assert(Arena->FirstFreeByte);
  return;
}
#endif
