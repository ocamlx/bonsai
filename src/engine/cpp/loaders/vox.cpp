// NOTE(Jesse): These chunk identifiers are actually present in the file in the
// reverse byte order.. ie. ID_VOX == "VOX " in the file.
enum Chunk_ID
{
  ID_NONE = 0,

  ID_VOX  = ' XOV',
  ID_MAIN = 'NIAM',
  ID_PACK = 'KCAP',
  ID_SIZE = 'EZIS',
  ID_XYZI = 'IZYX',
  ID_RGBA = 'ABGR',

  // chunks that are present in vox version 200

  ID_nTRN = 'NRTn',
  ID_nGRP = 'PRGn',
  ID_nSHP = 'PHSn',
  ID_MATL = 'LTAM',
  ID_LAYR = 'RYAL',
  ID_rOBJ = 'JBOr',
  ID_rCAM = 'MACr',
  ID_NOTE = 'ETON',
  ID_IMAP = 'PAMI',
};

inline u8
ReadChar(FILE* File, s32* byteCounter)
{
  u8 c;
  ReadBytesIntoBuffer(File, sizeof(u8), &c);
  *byteCounter -= sizeof(u8);
  return c;
}

inline s32
ReadInt(FILE* File)
{
  s32 i;
  ReadBytesIntoBuffer(File, sizeof(s32), (u8*)&i);
  return i;
}

inline s32
ReadInt(FILE* File, s32* byteCounter)
{
  *byteCounter -= sizeof(s32);
  return ReadInt(File);
}

link_internal void
ParseVoxString(FILE* File, s32* byteCounter)
{
  s32 Count = ReadInt(File, byteCounter);
  Assert(*byteCounter >= Count);
  *byteCounter -= Count;
  u8 Garbage;
  for (s32 Index = 0; Index < Count; ++Index)
  {
    ReadBytesIntoBuffer(File, 1, &Garbage);
  }
}

link_internal void
ParseVoxDict(FILE* File, s32* byteCounter)
{
  s32 KVPairCount = ReadInt(File, byteCounter);
  for (s32 Index = 0; Index < KVPairCount; ++Index)
  {
    ParseVoxString(File, byteCounter);
    ParseVoxString(File, byteCounter);
  }
}

link_internal void
ParseVoxRotation(FILE* File, s32* byteCounter)
{
  ReadChar(File, byteCounter);
}

inline Chunk_ID
GetHeaderType(FILE* File, s32* byteCounter)
{
  s32 ID = ReadInt(File, byteCounter);

  /* if ( ID == ID_VOX  || */
  /*      ID == ID_MAIN || */
  /*      ID == ID_PACK || */
  /*      ID == ID_SIZE || */
  /*      ID == ID_XYZI || */
  /*      ID == ID_RGBA ) */
  /* { */
  /* } */
  /* else */
  /* { */
  /*   Error("Unknown Header type (%d)", ID); */
  /* } */

  return (Chunk_ID)ID;
}

inline void
ReadVoxChunk(FILE *File)
{
  s32 Version = s32_MAX;
  s32 ID = ReadInt(File);
  Assert( ID == ID_VOX );

  Version = ReadInt(File);
  if (Version > 200)
  {
    Error("Unsupported VOX version (%d)", Version);
  }

  return;
}

inline s32
ReadMainChunk(FILE *File)
{
  s32 ID = ReadInt(File);

  if ( ID != ID_MAIN )
  {
    Error("Invalid Main Chunk");
    return 0;
  }

  s32 selfSize = ReadInt(File);
  if ( selfSize != 0 ) 
  {
    Error("Invalid Main Chunk");
    return 0;
  }

  s32 childSize = ReadInt(File);
  if (childSize == 0 )
  {
    Error("Invalid Main Chunk");
    return 0;
  }

  return childSize;
}

inline chunk_dimension
ReadSizeChunk(FILE *File, s32* byteCounter)
{
  s32 ChunkContentBytes = ReadInt(File, byteCounter);
  s32 ChildChunkCount = ReadInt(File, byteCounter);

  s32 chunkX = ReadInt(File, byteCounter);
  s32 chunkY = ReadInt(File, byteCounter);
  s32 chunkZ = ReadInt(File, byteCounter);

  chunk_dimension Result = Chunk_Dimension(chunkX, chunkY, chunkZ);
  return Result;
}

inline s32
ReadPackChunk(FILE *File, s32* byteCounter)
{
  s32 ChunkContentBytes = ReadInt(File, byteCounter);
  s32 ChildChunkCount = ReadInt(File, byteCounter);

  s32 TotalChunkCount = ReadInt(File, byteCounter);
  return TotalChunkCount;
}

inline s32
ReadXYZIChunk(FILE *File, s32* byteCounter)
{
  s32 ChunkContentBytes = ReadInt(File, byteCounter);
  s32 ChildChunkCount = ReadInt(File, byteCounter);

  s32 nVoxels = ReadInt(File, byteCounter);
  return nVoxels;
}

enum vox_loader_clip_behavior
{
  VoxLoaderClipBehavior_ClipToVoxels,
  VoxLoaderClipBehavior_NoClipping,
};

vox_data
LoadVoxData(memory_arena *WorldStorage, heap_allocator *Heap, char const *filepath, vox_loader_clip_behavior ClipBehavior, v3i HalfApronMin = {}, v3i HalfApronMax = {}, v3i ModDim = {})
{
  vox_data Result = {};
  v3i ReportedDim = {};

  native_file ModelFile = OpenFile(filepath, "r+b");

  if (ModelFile.Handle)
  {
    // Ensure we're dealing with a .vox file
    ReadVoxChunk(ModelFile.Handle);

    s32 totalChunkBytes = ReadMainChunk(ModelFile.Handle);
    s32 bytesRemaining = totalChunkBytes;

    s32 TotalChunkCount = 1; // Sometimes overwritten if theres a 'PACK' chunk
    // s32 TotalChunksRead = 0;

    while (bytesRemaining > 0)
    {
      Chunk_ID CurrentId = GetHeaderType(ModelFile.Handle, &bytesRemaining);
      switch ( CurrentId )
      {
        case ID_RGBA:
        {
          s32 ChunkContentBytes = ReadInt(ModelFile.Handle, &bytesRemaining);
          s32 ChildChunkCount = ReadInt(ModelFile.Handle, &bytesRemaining);

          Assert(ChunkContentBytes == 256*4);
          Assert(ChildChunkCount == 0);

          /* Result.Palette = (v4*)HeapAllocate(Heap, 256); */
          Result.Palette = Allocate(v4, WorldStorage, 256);

          for (u32 PaletteIndex = 0; PaletteIndex < 256; ++PaletteIndex)
          {
            u8 R = ReadChar(ModelFile.Handle, &bytesRemaining);
            u8 G = ReadChar(ModelFile.Handle, &bytesRemaining);
            u8 B = ReadChar(ModelFile.Handle, &bytesRemaining);
            u8 A = ReadChar(ModelFile.Handle, &bytesRemaining);
            Result.Palette[PaletteIndex].r = R;
            Result.Palette[PaletteIndex].g = G;
            Result.Palette[PaletteIndex].b = B;
            Result.Palette[PaletteIndex].a = A;
          }

        } break;

        case ID_PACK:
        {
          TotalChunkCount = ReadPackChunk(ModelFile.Handle, &bytesRemaining);
        } break;

        case ID_SIZE:
        {
          ReportedDim = ReadSizeChunk(ModelFile.Handle, &bytesRemaining);
        } break;

        case ID_XYZI:
        {
          // ++TotalChunksRead;

          s32 ReportedVoxelCount = ReadXYZIChunk(ModelFile.Handle, &bytesRemaining);

          s32 ActualVoxelCount = 0;

          s32 maxX = s32_MIN;
          s32 maxY = s32_MIN;
          s32 maxZ = s32_MIN;

          s32 minX = s32_MAX;
          s32 minY = s32_MAX;
          s32 minZ = s32_MAX;

          // TODO(Jesse)(calloc, memory): WTF is calloc doing in here?!
          boundary_voxel *LocalVoxelCache = (boundary_voxel *)calloc((umm)ReportedVoxelCount, sizeof(boundary_voxel) );
          for( s32 VoxelCacheIndex = 0;
               VoxelCacheIndex < ReportedVoxelCount;
               ++VoxelCacheIndex)
          {
            s32 X = (s32)ReadChar(ModelFile.Handle, &bytesRemaining);
            s32 Y = (s32)ReadChar(ModelFile.Handle, &bytesRemaining);
            s32 Z = (s32)ReadChar(ModelFile.Handle, &bytesRemaining);
            u8 W = ReadChar(ModelFile.Handle, &bytesRemaining); // Color

            voxel_position TestP = Voxel_Position(X,Y,Z);
            if (IsInsideDim(ReportedDim, TestP))
            {
              ++ActualVoxelCount;
              maxX = Max(TestP.x, maxX);
              maxY = Max(TestP.y, maxY);
              maxZ = Max(TestP.z, maxZ);

              minX = Min(TestP.x, minX);
              minY = Min(TestP.y, minY);
              minZ = Min(TestP.z, minZ);

              LocalVoxelCache[VoxelCacheIndex] = boundary_voxel(TestP.x, TestP.y, TestP.z, W);
              SetFlag(&LocalVoxelCache[VoxelCacheIndex], Voxel_Filled);
            }
            else
            {
              BUG("Voxel detected outside the dimention it should be in while loading (%s)!", filepath);
            }
          }

          v3i Max = {};
          v3i Min = {};
          v3i ModelDim = {};
          switch (ClipBehavior)
          {
            case VoxLoaderClipBehavior_ClipToVoxels:
            {
              s32 IndexToPosition = 1;  // max(X,Y,Z) are indicies, convert to positions
              Max = V3i(maxX+IndexToPosition, maxY+IndexToPosition, maxZ+IndexToPosition);
              Min = V3i(minX, minY, minZ);
              ModelDim = Max - Min + HalfApronMin + HalfApronMax;
            } break;

            case VoxLoaderClipBehavior_NoClipping:
            {
              ModelDim = ReportedDim + HalfApronMin + HalfApronMax;
            } break;
          }

          if (ModDim.x || ModDim.y || ModDim.z)
          {
            v3i Fixup = ModDim - (ModelDim % ModDim);
            ModelDim += Fixup;
            Assert(ModelDim % ModDim == V3i(0));
          }


          Result.ChunkData = AllocateChunkData(WorldStorage, ModelDim);
          Result.ChunkData->Dim = ModelDim;

          for( s32 VoxelCacheIndex = 0;
               VoxelCacheIndex < ActualVoxelCount;
               ++VoxelCacheIndex)
          {
            boundary_voxel *Voxel = &LocalVoxelCache[VoxelCacheIndex];
            Voxel->Offset = Voxel->Offset - Min + HalfApronMin;
            s32 Index = GetIndex(Voxel->Offset, ModelDim);
            Result.ChunkData->Voxels[Index] = Voxel->V;
          }

          free(LocalVoxelCache);

          FullBarrier;

          Result.ChunkData->Flags = Chunk_VoxelsInitialized;
        } break;

        case ID_nTRN:
        {
          s32 ChunkContentBytes = ReadInt(ModelFile.Handle, &bytesRemaining);
          s32 ChildChunkCount = ReadInt(ModelFile.Handle, &bytesRemaining);

          s32 NodeId = ReadInt(ModelFile.Handle, &bytesRemaining);

          // Transform node chunk
          ParseVoxDict(ModelFile.Handle, &bytesRemaining);
          s32 ChildNodeId = ReadInt(ModelFile.Handle, &bytesRemaining);
          Ensure(ReadInt(ModelFile.Handle, &bytesRemaining) == -1); // reserved field
          s32 LayerId = ReadInt(ModelFile.Handle, &bytesRemaining);
          s32 FrameCount = ReadInt(ModelFile.Handle, &bytesRemaining);
          Assert(FrameCount); // Apparently this has to be greater than 0

          for (s32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
          {
            ParseVoxDict(ModelFile.Handle, &bytesRemaining);
          }
        } break;

        case ID_rOBJ:
        case ID_rCAM:
        case ID_NOTE:
        case ID_IMAP:
        case ID_LAYR:
        case ID_MATL:
        case ID_nSHP:
        case ID_nGRP:
        {
          s32 ChunkContentBytes = ReadInt(ModelFile.Handle, &bytesRemaining);
          s32 ChildChunkCount = ReadInt(ModelFile.Handle, &bytesRemaining);

          for (s32 ByteIndex = 0; ByteIndex < ChunkContentBytes; ++ByteIndex)
          {
            ReadChar(ModelFile.Handle, &bytesRemaining);
          }

          Info("Skipping unsupported Chunk type (%S) while parsing (%s)", CS((const char*)&CurrentId, 4), filepath);
        } break;

        InvalidDefaultCase;
      }
    }

    Assert(bytesRemaining == 0);

    CloseFile(&ModelFile);
  }
  else
  {
    Error("Couldn't read model file '%s' .", filepath);
  }

  MarkBoundaryVoxels_NoExteriorFaces( Result.ChunkData->Voxels, Result.ChunkData->Dim, {}, Result.ChunkData->Dim);

  if (Result.Palette == 0)
  {
    Result.Palette = DefaultPalette;
  }

  return Result;
}

link_internal void
AllocateAndBuildMesh(vox_data *Vox, model *DestModel, memory_arena *TempMemory, memory_arena *PermMemory)
{
  chunk_data *ChunkData = Vox->ChunkData;
  DestModel->Dim = ChunkData->Dim;

  // TODO(Jesse): This wastes a shit-ton of memory.  Should probably have a way
  // of realloc-ing, or sum up how much memory we'll need first?
  AllocateMesh(&DestModel->Mesh, 6*VERTS_PER_FACE*(u32)Volume(DestModel->Dim), PermMemory);
  /* DestModel->Mesh = GetMeshForChunk(); */

  BuildWorldChunkMeshFromMarkedVoxels_Greedy(Vox, &DestModel->Mesh, TempMemory);
  // TODO(Jesse): Roll back what memory we don't use here.. or maybe allocate the initial buffer with temp and copy to perm?
}

link_internal model
LoadVoxModel(memory_arena *PermMemory, heap_allocator *Heap, char const *filepath, memory_arena *TempMemory)
{
  TIMED_FUNCTION();

  model Result = {};
  vox_data Vox = LoadVoxData(PermMemory, Heap, filepath, VoxLoaderClipBehavior_NoClipping );
  AllocateAndBuildMesh(&Vox, &Result, TempMemory, PermMemory );

  return Result;
}

