
chunk_data*
AllocateChunk(memory_arena *Storage, chunk_dimension Dim)
{
  // Note(Jesse): Not sure the alignment is completely necessary, but it may be
  // because multiple threads go to town on these memory blocks
  chunk_data *Result = AllocateAlignedProtection(chunk_data, Storage, 1, 64, false);

  s32 Vol = Volume(Dim);
  if (Vol) { Result->Voxels = AllocateAlignedProtection(voxel, Storage , Vol, 64, false); }

  ZeroChunk(Result);

  return Result;
}

inline b32
IsFilledInChunk( chunk_data *Chunk, voxel_position VoxelP, chunk_dimension Dim)
{
  b32 isFilled = True;

  if (Chunk && ( IsSet(Chunk, Chunk_Initialized) || IsSet(Chunk, Chunk_MeshComplete) ))
  {
    s32 i = GetIndex(VoxelP, Dim);

    Assert(i > -1);
    Assert(i < Volume(Dim));

    isFilled = IsSet(&Chunk->Voxels[i], Voxel_Filled);
  }

  return isFilled;
}

inline b32
NotFilledInChunk( chunk_data *Chunk, voxel_position VoxelP, chunk_dimension Dim)
{
  b32 Result = !IsFilledInChunk(Chunk, VoxelP, Dim);
  return Result;
}

inline b32
NotFilledInChunk(chunk_data *Chunk, s32 Index)
{
  Assert(Chunk);
  b32 NotFilled = True;

  if (Index > 1)
  {
    NotFilled = !IsSet(&Chunk->Voxels[Index], Voxel_Filled);
  }

  return NotFilled;
}

void
FillChunk(chunk_data *chunk, chunk_dimension Dim, u8 ColorIndex = BLACK)
{
  s32 Vol = Volume(Dim);

  for (int i = 0; i < Vol; ++i)
  {
    // TODO(Jesse): Pretty sure we don't have to set the faces anymore??
    SetFlag(&chunk->Voxels[i], (voxel_flag)(Voxel_Filled     |
                                            Voxel_TopFace    |
                                            Voxel_BottomFace |
                                            Voxel_FrontFace  |
                                            Voxel_BackFace   |
                                            Voxel_LeftFace   |
                                            Voxel_RightFace));


    chunk->Voxels[i].Color = ColorIndex;
  }

  SetFlag(chunk, Chunk_Initialized);
}
