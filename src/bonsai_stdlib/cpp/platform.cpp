#ifdef _WIN32
#include <win32_platform.cpp>
#else
#include <unix_platform.cpp>
#endif


u64
GetWorkerThreadCount()
{
  u64 LogicalCoreCount = GetLogicalCoreCount();
  u64 ThreadCount = LogicalCoreCount - 1 - DEBUG_THREAD_COUNT_BIAS; // -1 because we already have a main thread
  return ThreadCount;
}

u32
GetTotalThreadCount()
{
  u32 Result = (u32)GetWorkerThreadCount() + 1;
  return Result;
}

