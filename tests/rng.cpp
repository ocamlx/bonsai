#define BONSAI_NO_PUSH_METADATA
#define BONSAI_NO_DEBUG_MEMORY_ALLOCATOR

#include <bonsai_types.h>
#include <unix_platform.cpp>
#include <test_utils.cpp>

s32
main()
{
  TestSuiteBegin("RNG");

#if 0
  random_series Entropy = {43215426453};

  const u32 TableSize = 256;
  u32 HitTable[TableSize] = {};
  u32 MaxValue = 0;

  memory_arena* Memory = AllocateArena(Megabytes(8));

  ansi_stream WordStream = AnsiStreamFromFile(CS("tests/fixtures/words.txt"), Memory);

  while (Remaining(&WordStream))
  {
    counted_string Word = PopWordCounted(&WordStream);
    u32 HashValue = Hash(&Word) % TableSize;
    ++HitTable[HashValue];

    if (HitTable[HashValue] > MaxValue)
    {
      MaxValue = HitTable[HashValue];
    }
  }
  Log("Max: %u\n", MaxValue);

  u32 MappedRowSize = 128;
  for (u32 TableIndex = 0;
      TableIndex < TableSize;
      ++TableIndex)
  {
    u32 TableValue = HitTable[TableIndex];
    r32 Value = (r32)TableValue / (r32)MaxValue;
    u32 Mapped = MapValueToRange(0, Value, MappedRowSize);

    for (u32 ValueIndex = 0;
        ValueIndex < MappedRowSize;
        ++ValueIndex)
    {
      if (ValueIndex < Mapped)
      {
        Log(".");
      }
      else
      {
        Log("_");
      }
    }
    Log(" %u", TableValue);

    Log("\n");
  }
#endif


  TestSuiteEnd();
  exit(TestsFailed);
}


