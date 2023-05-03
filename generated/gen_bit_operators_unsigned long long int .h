link_internal u64
UnsetLeastSignificantSetBit(u64 *Input)
{
  u64 StartValue = *Input;
  u64 Cleared = StartValue & (StartValue - 1);
  *Input = Cleared;

  u64 Result = (StartValue & ~(Cleared));
  return Result;
}

link_internal u32
CountBitsSet_Kernighan(u64 Input)
{
  u32 Result = 0;
  while (Input != 0)
  {
    Input = Input & (Input - 1);
    Result++;
  }
  return Result;
}

link_internal u64
GetNthSetBit(u64 Target, u64 NBit)
{
  u64 Result = u32_MAX;
  for (u64 BitIndex = 0; BitIndex < NBit; ++BitIndex)
  {
    Result = UnsetLeastSignificantSetBit(&Target);
  }
  return Result;
}

