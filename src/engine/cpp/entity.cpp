#define PARTICLE_SYSTEM_EMIT_FOREVER f32_MAX

inline void
Deactivate(particle_system *System)
{
  Clear(System);
  return;
}

inline void
Destroy(entity *Entity)
{
  Assert( Spawned(Entity) );
  Entity->State = EntityState_Destroyed;
  Assert(Entity->Emitter);
  Deactivate(Entity->Emitter);
}

inline void
Unspawn(entity *Entity)
{
  Entity->State = EntityState_Free;
  Assert(Entity->Emitter);
  auto Emitter = Entity->Emitter;

  Clear(Entity);
  Deactivate(Emitter);

  Entity->Emitter = Emitter;
}

inline b32
IsLoot(entity *Entity)
{
  b32 Result = Entity->Type == EntityType_Loot;
  return Result;
}

inline b32
IsPlayer(entity *Entity)
{
  b32 Result = Entity->Type == EntityType_Player;
  return Result;
}

collision_event
GetCollision( world *World, canonical_position TestP, v3 CollisionDim )
{
  TIMED_FUNCTION();

  collision_event Collision = {};

  chunk_dimension WorldChunkDim = World->ChunkDim;

  // TODO(Jesse): Probably just assert this is true
  TestP = Canonicalize(WorldChunkDim, TestP);

  voxel_position MinP = Voxel_Position(TestP.Offset);
  voxel_position MaxP = Voxel_Position(Ceil(TestP.Offset + CollisionDim));

  for ( int z = MinP.z; z < MaxP.z; z++ )
  {
    for ( int y = MinP.y; y < MaxP.y; y++ )
    {
      for ( int x = MinP.x; x < MaxP.x; x++ )
      {
        canonical_position LoopTestP = Canonicalize( WorldChunkDim, V3(x,y,z), TestP.WorldP );
        world_chunk *Chunk = GetWorldChunkFromHashtable( World, LoopTestP.WorldP );

        if ( !Chunk || IsFilledInChunk(Chunk, Voxel_Position(LoopTestP.Offset), World->ChunkDim) )
        {
          if (Collision.Count == 0) { Collision.MinP = LoopTestP; }
          Collision.MaxP = LoopTestP;
          Collision.Count ++;
        }

      }
    }
  }

  return Collision;
}

link_internal aabb GetSimSpaceAABB(world *World, entity *Entity);

inline b32
GetCollision(world *World, entity *First, entity *Second)
{
  TIMED_FUNCTION();
  if (Destroyed(First) || Destroyed(Second))
    return False;

  if (Unspawned(First) || Unspawned(Second))
    return False;

  aabb FirstAABB = GetSimSpaceAABB(World, First);
  aabb SecondAABB = GetSimSpaceAABB(World, Second);

  b32 Result = Intersect(&FirstAABB, &SecondAABB);
  return Result;
}

inline b32
GetCollision(world *World, entity **Entities, entity *Entity)
{
  b32 Result = False;

  for (s32 EntityIndex = 0;
      EntityIndex < TOTAL_ENTITY_COUNT;
      ++EntityIndex)
  {
    entity *TestEntity = Entities[EntityIndex];

    if (TestEntity == Entity)
      continue;

    Result = GetCollision(World, Entity, TestEntity);
  }

  return Result;
}

/* TODO(Jesse, id: 130, tags: be_smarter): This offset is only used to check if
 * entities are grounded.  Can we do that in a more intelligent way?
 */
collision_event
GetCollision(world *World, entity *Entity, v3 Offset = V3(0,0,0) )
{
  TIMED_FUNCTION();

  collision_event C = {};

  if ( !Spawned(Entity) )
    return C;

  C = GetCollision( World,
      Canonicalize(World->ChunkDim, Entity->P + Offset),
      Entity->CollisionVolumeRadius*2.0f );

  return C;
}


#if 0
inline void
SpawnLoot(entity *Entity, random_series *Entropy, model *GameModels)
{
#define LOOT_CHANCE (1)
  b32 ShouldSpawnLoot = (RandomU32(Entropy) % LOOT_CHANCE) == 0;

  if (ShouldSpawnLoot)
  {
    Entity->State = EntityState_Uninitialized;
    Entity->Type = EntityType_Loot;
    Entity->State = EntityState_Spawned;
    Entity->Physics.Velocity = V3(0,0,0);
    Entity->Model = GameModels[EntityType_Loot];
  }

  return;
}
#endif

entity *
GetFreeEntity(entity** EntityTable)
{
  entity *Result = 0;

  for ( s32 EntityIndex = 0;
        EntityIndex < TOTAL_ENTITY_COUNT;
        ++EntityIndex )
  {
    entity *TestEntity = EntityTable[EntityIndex];
    if (TestEntity->State == EntityState_Free)
    {
      Result = TestEntity;
      Result->State = EntityState_Reserved;
      break;
    }
  }

  if (Result)
  {
    Assert(Unspawned(Result));
    Assert(!Destroyed(Result));
  }

  return Result;
}

entity *
AllocateEntity(memory_arena *Memory, chunk_dimension ModelDim)
{
  entity *Entity = Allocate(entity, Memory, 1);
  Entity->Emitter = Allocate(particle_system, Memory, 1);
  Entity->Model.Dim = ModelDim;

  Entity->Scale = 1.0f;

  return Entity;
}

link_internal entity**
AllocateEntityTable(memory_arena* Memory, u32 Count)
{
  entity** Result = Allocate(entity*, Memory, Count);
  for (u32 EntityIndex = 0;
      EntityIndex < Count;
      ++ EntityIndex)
  {
    Result[EntityIndex] = AllocateEntity(Memory, Chunk_Dimension(0, 0, 0));
  }

  return Result;
}

void
SpawnEntity( model *GameModels, entity *Entity, entity_type Type, model_index ModelIndex)
{
  // These are mutually exclusive, so checking both is redundant, but that
  // could change in the future
  Assert(Unspawned(Entity));
  Assert(!Destroyed(Entity));
  Assert(Entity->State == EntityState_Reserved);

  Entity->State = EntityState_Spawned;
  Entity->Type = Type;
  Entity->FireCooldown = Entity->RateOfFire;

  if (ModelIndex)
  {
    Entity->Model = GameModels[ModelIndex];
  }

#if 0
  switch (Type)
  {
    case EntityType_Player:
    {
    } break;

    case EntityType_Enemy:
    {
    } break;

    case EntityType_ParticleSystem:
    {
    } break;

    case EntityType_Default:
    {
    } break;

    InvalidDefaultCase;
  }
#endif

  return;
}

link_internal void
SpawnEntity(entity *Entity)
{
  SpawnEntity(0, Entity, EntityType_Default, ModelIndex_None);
}

void
SpawnEntity(
    entity *Entity,
    model *Model,
    entity_type Type,

    physics *Physics,

    canonical_position *InitialP,
    v3 CollisionVolumeRadius,

    r32 Scale = 1.0f,
    r32 RateOfFire = 1.0f,
    s32 Health = 1
  )
{
  TIMED_FUNCTION();

  if (Model)
    Entity->Model = *Model;

  Entity->Type = Type;

  if (Physics)
    Entity->Physics = *Physics;

  if (InitialP)
    Entity->P = *InitialP;

  Entity->CollisionVolumeRadius = CollisionVolumeRadius;

  Entity->Rotation = Quaternion(0,0,0,1);

  Entity->Scale = Scale;
  Entity->RateOfFire = RateOfFire;
  Entity->Health = Health;

  Entity->State = EntityState_Spawned;

  return;
}

entity *
AllocateEntity(memory_arena *Memory)
{
  entity *Entity = Allocate(entity, Memory, 1);
  Entity->Emitter = Allocate(particle_system, Memory, 1);

  Entity->Scale = 1.0f;

  return Entity;
}

entity *
AllocatePlayer(memory_arena *Memory)
{
  /* entity *Player = AllocateEntity(Plat, Memory, DEBUG_ENTITY_DIM); */
  entity *Player = AllocateEntity(Memory);

  return Player;
}

#if 0
void
SpawnEnemy(world *World, entity **WorldEntities, entity *Enemy, random_series *EnemyEntropy, model *GameModels)
{
  NotImplemented;
#if DEBUG_PARTICLE_EFFECTS
  return;
#endif

  TIMED_FUNCTION();
  s32 X = (RandomPositiveS32(EnemyEntropy) % VR_X) - (RandomPositiveS32(EnemyEntropy) % VR_X);
  s32 Y = World->Center.y + (VR_Y / 2) - DEBUG_ENTITY_COLLISION_VOL_RADIUS.y;
  s32 Z = World->Center.z;

  world_position InitialCenter = World_Position(X, Y, Z);

  s32 OffsetX = (RandomPositiveS32(EnemyEntropy) % CD_X);

  canonical_position InitialP =
    Canonicalize(World->ChunkDim, Canonical_Position( V3(OffsetX,0,0), InitialCenter));

  physics Physics = {};
  Physics.Force = V3(0, 0, 0);
  Physics.Mass = 0.6f;
  Physics.Speed = 1200;
  Physics.Drag = 1.2f;

  r32 Scale = 0.5f;
  r32 RateOfFire = 2.0f;
  u32 Health = 1;

  SpawnEntity(
    Enemy,
    &GameModels[ModelIndex_Enemy],
    EntityType_Enemy,

    &Physics,

    &InitialP,
    DEBUG_ENTITY_COLLISION_VOL_RADIUS,

    Scale,
    RateOfFire,
    Health);

  Enemy->Model = GameModels[ModelIndex_Enemy];

  // Respawn entity if it collides against the world or current entities
  if ( GetCollision(World, Enemy).didCollide ||
       GetCollision(WorldEntities, Enemy, World->ChunkDim)    )
    SpawnEnemy(World, WorldEntities, Enemy, EnemyEntropy, GameModels);

  return;
}
#endif

entity *
SpawnProjectile(entity** EntityTable,
                canonical_position *P,
                v3 Velocity,
                entity_type ProjectileType
  )
{
  entity *Projectile = GetFreeEntity(EntityTable);

  v3 CollisionVolumeRadius = V3(0.25f);

  physics Physics = {};
  Physics.Velocity = Velocity;
  Physics.Speed = 60;

  r32 Scale = 1.0f;
  r32 RateOfFire = 1.0f;
  s32 Health = 1;

  SpawnEntity(
    Projectile,
    0,
    ProjectileType,

    &Physics,

    P,
    CollisionVolumeRadius,

    Scale,
    RateOfFire,
    Health);

#if 0
  switch (ProjectileType)
  {
    case EntityType_PlayerProton:
    {
      Projectile->Model = GameModels[ModelIndex_Proton];
    } break;

    case EntityType_PlayerProjectile:
    case EntityType_EnemyProjectile:
    {
      Projectile->Model = GameModels[ModelIndex_Projectile];
    } break;

    InvalidDefaultCase;
  }
#endif

  return Projectile;
}

void
UnspawnParticleSystem(particle_system *System)
{
  System->EmissionLifespan = 0.f;
}

#if 0
void
SpawnParticleSystem(particle_system *System, particle_system_init_params *Params)
{
  Assert(Inactive(System));

  /* poof( */
  /*   func (particle_system_init_params TParams) */
  /*   { */
  /*     TParams.map_members (M) */
  /*     { */
  /*       System->(M.name) = Params->(M.name); */
  /*     } */
  /*   } */
  /* ) */

  System->SpawnRegion = Params->SpawnRegion;
  System->SystemMovementCoefficient = Params->SystemMovementCoefficient;

  System->EmissionLifespan = Params->EmissionLifespan;
  System->ParticlesPerSecond = Params->ParticlesPerSecond;

  System->LifespanMod = Params->LifespanMod;
  System->ParticleLifespan = Params->ParticleLifespan;

  System->ParticleTurbMin = Params->ParticleTurbMin;
  System->ParticleTurbMax = Params->ParticleTurbMax;

  System->ParticleStartingDim = Params->ParticleStartingDim;

  System->Entropy = Params->Entropy;
  System->Drag = Params->Drag;
  System->SpawnType = Params->SpawnType;

  MemCopy( (u8*)&Params->Colors, (u8*)&System->Colors, sizeof(System->Colors) );

  return;
}
#endif

void
SpawnSmoke(entity *Entity, random_series *Entropy, v3 Offset, r32 Radius)
{
  particle_system *System = Entity->Emitter;

  System->SpawnType = ParticleSpawnType_Expanding;

  System->Entropy.Seed = RandomU32(Entropy);

  System->Colors[0] = GREY_0;
  System->Colors[1] = GREY_1;
  System->Colors[2] = GREY_2;
  System->Colors[3] = GREY_4;
  System->Colors[4] = GREY_6;
  System->Colors[5] = GREY_8;

  System->SpawnRegion = aabb(Offset, V3(Radius, Radius, Radius) * 0.25f );

  System->EmissionLifespan = 0.25f;
  System->LifespanMod = 0.75f;
  System->ParticleLifespan = 0.5f;
  System->ParticlesPerSecond = 80.0f*Radius;

  /* System->Physics.Speed = 2; */
  /* System->Physics.Drag = V3(2.2f); */
  /* System->Physics.Mass = 3.0f; */

  r32 TurbMin = 1.5f*Radius;
  r32 TurbMax = 3.f*Radius;

  System->ParticleTurbMin = V3(TurbMin);
  System->ParticleTurbMax = V3(TurbMax);

  System->ParticleStartingDim = V3(0.10f, 0.10f, 0.06f) * Radius;
  System->ParticleEndingDim = 2.5f;

  System->SystemMovementCoefficient = 0.1f;
  System->Drag = 2.f;

  /* SpawnParticleSystem(Entity->Emitter, &Params); */

  return;
}

void
SpawnSplotionBitty(entity *Entity, random_series *Entropy, v3 Offset, r32 Radius)
{
  particle_system *System = Entity->Emitter;

  System->SpawnType = ParticleSpawnType_Expanding;

  System->Entropy.Seed = RandomU32(Entropy);

  System->Colors[0] = GREY_0;
  System->Colors[1] = GREY_1;
  System->Colors[2] = GREY_2;
  System->Colors[3] = GREY_3;
  System->Colors[4] = GREY_5;
  System->Colors[5] = RED;

  System->SpawnRegion = aabb(Offset, V3(Radius) );

  System->EmissionLifespan = 1.0f;
  /* System->EmissionLifespan = 0.23f; */
  System->LifespanMod = 0.25f;
  System->ParticleLifespan = 0.25f;
  System->ParticlesPerSecond = 20.0f;
  /* System->ParticlesPerSecond = 0.0f; */

  System->ParticleTurbMin = V3(0.f, 0.f, 0.f);
  System->ParticleTurbMax = V3(0.f, 0.f, .1f);

  System->ParticleStartingDim = V3(0.10f);
  System->ParticleEndingDim = 6.f;

  System->SystemMovementCoefficient = 1.f;
  /* System->Drag = 11.0f; */

  /* SpawnParticleSystem(Entity->Emitter, &Params); */

  return;
}

void
SpawnExplosion(entity *Entity, random_series *Entropy, v3 Offset, r32 Radius)
{
  particle_system *System = Entity->Emitter;

  System->SpawnType = ParticleSpawnType_Expanding;

  System->Entropy.Seed = RandomU32(Entropy);

  System->Colors[0] = BLACK;
  System->Colors[1] = DARK_DARK_RED;
  System->Colors[2] = DARK_RED;
  System->Colors[3] = DARK_ORANGE;
  System->Colors[4] = LIGHT_ORANGE;
  System->Colors[5] = WHITE;

  /* System->Colors[1] = (u8)RandomU32(Entropy); */
  /* System->Colors[2] = (u8)RandomU32(Entropy); */
  /* System->Colors[3] = (u8)RandomU32(Entropy); */
  /* System->Colors[4] = (u8)RandomU32(Entropy); */
  /* System->Colors[5] = (u8)RandomU32(Entropy); */


  System->SpawnRegion = aabb(Offset, V3(Radius*0.10f) );

  System->EmissionLifespan = 0.25f;
  System->LifespanMod = 0.15f;
  System->ParticleLifespan = 0.15f;
  System->ParticlesPerSecond = 500.0f*Radius;

  /* System->Physics.Speed = 2; */
  /* System->Physics.Drag = V3(2.2f); */
  /* System->Physics.Mass = 3.0f; */

  r32 xyTurb = 7.f*Radius;
  /* r32 xyTurb = 2.5f; */
  /* r32 xyTurb = 0.0f; */
  System->ParticleTurbMin = V3(-xyTurb, -xyTurb, -xyTurb);
  System->ParticleTurbMax = V3(xyTurb, xyTurb, xyTurb);

  /* System->Physics.Velocity = V3(0.f, 0.f, 10.f); */

  System->ParticleStartingDim = V3(0.3f) * Radius;
  System->ParticleEndingDim = 0.1f;

  System->SystemMovementCoefficient = 0.1f;
  System->Drag = 11.0f;

  /* SpawnParticleSystem(Entity->Emitter, &Params); */

  return;
}

// FIXME(Jesse): Remove the f32_MAX EmissionLifespan below

void
SpawnFire(entity *Entity, random_series *Entropy, v3 Offset, r32 Dim)
{
  particle_system *System = Entity->Emitter;

  System->SpawnType = ParticleSpawnType_Random;

  System->Drag = 6.0f;

  System->Entropy.Seed = RandomU32(Entropy);

  System->Colors[0] = BLACK;
  System->Colors[1] = DARK_DARK_RED;
  System->Colors[2] = DARK_RED;
  System->Colors[3] = DARK_ORANGE;
  System->Colors[4] = YELLOW;
  System->Colors[5] = WHITE;

  /* System->Colors[1] = (u8)RandomU32(Entropy); */
  /* System->Colors[2] = (u8)RandomU32(Entropy); */
  /* System->Colors[3] = (u8)RandomU32(Entropy); */
  /* System->Colors[4] = (u8)RandomU32(Entropy); */
  /* System->Colors[5] = (u8)RandomU32(Entropy); */


  System->SpawnRegion = aabb(Offset, V3(0.16f)*Dim);

  System->EmissionLifespan = PARTICLE_SYSTEM_EMIT_FOREVER;
  System->LifespanMod = 0.07f;
  System->ParticleLifespan = 0.25f;
  System->ParticlesPerSecond = 150.0f;

  /* System->Physics.Speed = 1; */
  /* System->Physics.Drag = V3(2.2f); */
  /* System->Physics.Mass = 6.0f; */

  r32 xyTurb = 4.f*Dim;
  /* r32 xyTurb = 2.5f; */
  /* r32 xyTurb = 0.0f; */
  System->ParticleTurbMin = V3(-xyTurb, -xyTurb, 20.0f*Dim);
  System->ParticleTurbMax = V3(xyTurb, xyTurb, 35.0f*Dim);

  /* System->Physics.Velocity = V3(0.0f, 0.0f, 9.0f); */

  System->ParticleStartingDim = V3(0.9f)*Dim;
  System->ParticleEndingDim = 0.1f*Dim;

  System->SystemMovementCoefficient = 0.1f;

  /* SpawnParticleSystem(Entity->Emitter, &Params); */

  return;
}

void
SpawnPlayerLikeEntity( platform *Plat,
                       world *World,
                       model* Model,
                       entity *Player,
                       canonical_position InitialP,
                       random_series* Entropy,

                       r32 Scale = 1.0f )
{
  physics Physics = {};
  Physics.Speed = 60.f;
  Physics.Mass = 35.f;

  r32 RateOfFire = 0.8f;
  s32 Health = 10;

  v3 CollisionVolumeRadius = {};

  if (Model)
  {
    CollisionVolumeRadius = Model->Dim * Scale * 0.5f; // 0.5f is to shrink to a radius, instead of dim
  }

  for (s32 z = -1; z < (s32)CollisionVolumeRadius.z; ++ z)
  {
    for (s32 y = -1; y < (s32)CollisionVolumeRadius.y; ++ y)
    {
      for (s32 x = -1; x < (s32)CollisionVolumeRadius.x; ++ x)
      {
        canonical_position CP = Canonicalize(World->ChunkDim, V3(x, y, z), InitialP.WorldP);

        world_chunk *Chunk = GetWorldChunkFromHashtable( World, CP.WorldP );
        if (Chunk == 0)
        {
          Chunk = GetWorldChunkFor(World->Memory, World, CP.WorldP);
          if (Chunk)
          {
            QueueChunkForInit(&Plat->HighPriority, Chunk);
          }
        }
      }
    }
  }

  SpawnEntity(
      Player,
      Model,
      EntityType_Player,

      &Physics,

      &InitialP,
      CollisionVolumeRadius,

      Scale,
      RateOfFire,
      Health
    );

  /* WaitForWorkerThreads(&Plat->HighPriorityWorkerCount); */

  return;
}

void
EntityWorldCollision(world *World, entity *Entity, collision_event *Event, chunk_dimension VisibleRegion)
{
  Assert(Entity->Type != EntityType_None);

  switch (Entity->Type)
  {
    case EntityType_Player:
    {

    } break;

    case EntityType_PlayerProjectile:
    {
      NotImplemented;
#if 0
      if (Event->Chunk)
      {
        s32 i = GetIndex(Voxel_Position(Event->CP.Offset), World->ChunkDim);
        world_chunk *Chunk = Event->Chunk;
        Chunk->Voxels[i] = {};
        /* ZeroMesh(&Chunk->Mesh); */
        // TODO(Jesse, id: 131, tags: not_implemented): This path needs to call CanBuildWorldChunkMesh or something similar
        /* BuildWorldChunkMesh(World, Chunk, World->ChunkDim, Chunk->Mesh, VisibleRegion); */
      }
      Unspawn(Entity);
#endif
    } break;

    default:
    {
      /* Unspawn(Entity); */
    } break;
  }
}

b32
EntitiesCanCollide(entity *First, entity *Second)
{
  b32 Result = True;

  collision_type CollisionType = (collision_type)(First->Type | Second->Type);

  switch (CollisionType)
  {
    case Collision_Player_PlayerProjectile:
    case Collision_Enemy_EnemyProjectile:
    case Collision_Enemy_Enemy:
    {
      Result = False;
    } break;

    default: {} break;
  }

  return Result;
}

#define TOTAL_FRAME_EVENT_COUNT (960)
inline void
PushFrameEvent(event_queue *EventQueue, frame_event *EventInit, u32 FramesToForward=1)
{
  Assert(FramesToForward < TOTAL_FRAME_EVENT_COUNT);

  frame_event *FreeEvent = GetFreeFrameEvent(EventQueue);
  *FreeEvent = *EventInit;

  s32 EventIndex = (EventQueue->CurrentFrameIndex+FramesToForward) % TOTAL_FRAME_EVENT_COUNT;
  frame_event **Event = &EventQueue->Queue[EventIndex];

  while (*Event)
    Event = &(*Event)->Next;

  Assert(*Event == 0);

  *Event = FreeEvent;

  return;
}

void
ProcessCollisionRule(
    entity        *First,
    entity        *Second,
    event_queue   *EventQueue
  )
{
  NotImplemented;

  Assert(First!=Second);

  collision_type CollisionType = (collision_type)(First->Type | Second->Type);

  switch (CollisionType)
  {
#if 0
    case Collision_Player_Loot:
    {
      entity *Player = First;
      entity *Loot = Second;

      if (IsPlayer(Second))
      {
        Player=Second;
        Loot=First;
      }

      Player->RateOfFire = 0.10f + (Player->RateOfFire/1.7f);
      Unspawn(Loot);

    } break;
#endif

    case Collision_Player_EnemyProjectile:
    case Collision_Player_Enemy:
    {
      entity *Player = First;
      entity *Enemy = Second;

      if (IsPlayer(Second))
      {
        Player=Second;
        Enemy=First;
      }

      Player->Health --;

      if (Player->Health <= 0)
      {
        Unspawn(Player);
        frame_event Event(FrameEvent_GameModeLoss);
        PushFrameEvent(EventQueue, &Event);
      }

      Destroy(Enemy);
      frame_event Event(Enemy, FrameEvent_Explosion);
      PushFrameEvent(EventQueue, &Event);

    } break;

    case Collision_Enemy_PlayerProjectile:
    case Collision_Enemy_PlayerProton:
    {
      entity *Enemy = First;
      entity *Projectile = Second;

      if ( Second->Type == EntityType_Enemy )
      {
        Enemy = Second;
        Projectile = First;
      }

      Unspawn(Projectile);

      Destroy(Enemy);
      frame_event Event(Enemy, FrameEvent_Explosion);
      PushFrameEvent(EventQueue, &Event);
    } break;

    default: {} break;
  }

  return;
}


link_internal collision_event
DoEntityCollisions(world *World, entity** EntityTable, entity *Entity)
{
  TIMED_FUNCTION();

  Assert(Spawned(Entity));

  collision_event Result = {};
  for (s32 EntityIndex = 0;
      EntityIndex < TOTAL_ENTITY_COUNT;
      ++EntityIndex)
  {
    entity *TestEntity = EntityTable[EntityIndex];
    if (Entity != TestEntity)
    {
      if (GetCollision(World, Entity, TestEntity))
      {
        // TODO(Jesse): Should we actually test the overlapping area here?  Probably.
        Result.Count ++;
      }
    }
  }

  return Result;
}

#if 0
// Defunct, but I'm keeping it for posterity in case it's relevant
void
DoEntityCollisions(entity** EntityTable, entity *Entity, event_queue* EventQueue, chunk_dimension WorldChunkDim)
{
  TIMED_FUNCTION();

  Assert(Spawned(Entity));

  for (s32 EntityIndex = 0;
      EntityIndex < TOTAL_ENTITY_COUNT;
      ++EntityIndex)
  {
    entity *TestEntity = EntityTable[EntityIndex];
    if (TestEntity == Entity)
      continue;

    if (!EntitiesCanCollide(Entity, TestEntity))
      continue;

    if (GetCollision(Entity, TestEntity, WorldChunkDim))
      ProcessCollisionRule(Entity, TestEntity, EventQueue);
  }

  return;
}
#endif

v3
ClampBetween( r32 Min, v3 Gross, r32 Max )
{
  v3 Result = Gross;

  if ( Gross.x > Max )
  {
    Result.x = Max;
  }
  if ( Gross.x < Min )
  {
    Result.x = Min;
  }


  if ( Gross.y > Max )
  {
    Result.y = Max;
  }
  if ( Gross.y < Min )
  {
    Result.y = Min;
  }


  if ( Gross.z > Max )
  {
    Result.z = Max;
  }
  if ( Gross.z < Min )
  {
    Result.z = Min;
  }

  return Result;
}

void
UpdateEntityP(world* World, entity *Entity, v3 Delta)
{
  Entity->P.Offset += Delta;
  Canonicalize(World, &Entity->P);
}

void
MoveEntityInWorld(world* World, entity *Entity, v3 GrossDelta, chunk_dimension VisibleRegion)
{
  TIMED_FUNCTION();

  /* DebugLine("GrossDelta (%f %f %f)", GrossDelta.x, GrossDelta.y, GrossDelta.z); */

  chunk_dimension WorldChunkDim = World->ChunkDim;
  collision_event C = {};
  v3 Remaining = GrossDelta;

  while (LengthSq(Remaining) > 0)
  {
    v3 StepDelta = ClampBetween(-1.0f, Remaining, 1.0f);
    Remaining -= StepDelta;

    for ( u32 AxisIndex = 0;
              AxisIndex < 3;
            ++AxisIndex )
    {
      if (StepDelta.E[AxisIndex] != 0.0f)
      {
        Entity->P.Offset.E[AxisIndex] += StepDelta.E[AxisIndex];
        Entity->P = Canonicalize(WorldChunkDim, Entity->P);

        canonical_position CollisionBasis = Entity->P;

        v3 CollisionVolume = Entity->CollisionVolumeRadius*2.0f;
        if (StepDelta.E[AxisIndex] > 0.f) // We're going in the positive direction
        {
          CollisionBasis.Offset.E[AxisIndex] += Truncate(CollisionVolume.E[AxisIndex]);
          CollisionBasis = Canonicalize(WorldChunkDim, CollisionBasis);
        }

        CollisionVolume.E[AxisIndex] = Min(CollisionVolume.E[AxisIndex], 1.f);

        C = GetCollision(World, CollisionBasis, CollisionVolume );

        if ( C.Count > 0 ) //&& C.Chunk && IsSet(C.Chunk, Chunk_VoxelsInitialized) )
        {
          // TODO(Jesse): Parameterize by adding something to physics struct
          /* Entity->Physics.Velocity.E[AxisIndex] *= -0.25f; */
          Entity->Physics.Velocity.E[AxisIndex] = 0.f;

          /* Entity->Physics.Velocity.E[AxisIndex] = 0.f; */
          Entity->Physics.Delta.E[AxisIndex] = 0;

#if 1
          Entity->P.Offset.E[AxisIndex] -= StepDelta.E[AxisIndex];
          Entity->P = Canonicalize(WorldChunkDim, Entity->P);
#else
          Entity->P.Offset.E[AxisIndex] -= StepDelta.E[AxisIndex];
          if (StepDelta.E[AxisIndex] > 0)
          {
            r32 Max = Entity->P.Offset.E[AxisIndex] + Entity->CollisionVolumeRadius.E[AxisIndex];

            Entity->P.WorldP.E[AxisIndex] = C.MinP.WorldP.E[AxisIndex];
            Entity->P.Offset.E[AxisIndex] = C.MinP.Offset.E[AxisIndex];
            Entity->P.Offset.E[AxisIndex] -= ((Entity->CollisionVolumeRadius.E[AxisIndex]*2.f) + 0.001f);
          }

          if (StepDelta.E[AxisIndex] < 0)
          {
#if 1
            Entity->P.Offset.E[AxisIndex] = Truncate(Entity->P.Offset.E[AxisIndex] += 1.0f);
#else
            Entity->P.WorldP.E[AxisIndex] = C.MaxP.WorldP.E[AxisIndex];
            Entity->P.Offset.E[AxisIndex] = C.MaxP.Offset.E[AxisIndex];
            /* Entity->P.Offset.E[AxisIndex] += 1.0f; */
            /* Truncate(Entity->P.Offset.E[AxisIndex]);// += 1.0f; */
            Entity->P.Offset.E[AxisIndex] += 1.001f;
            /* Entity->P.Offset.E[AxisIndex] ++; */
            /* Entity->P.Offset.E[AxisIndex] = Truncate(Entity->P.Offset.E[AxisIndex] += 1.0f); */
#endif
          }
#endif

          Entity->P = Canonicalize(WorldChunkDim, Entity->P);
          /* EntityWorldCollision(World, Entity, &C, VisibleRegion); */
        }
      }
    }

    if (Unspawned(Entity) || Destroyed(Entity))
      break;

    /* DoEntityCollisions(GameState, Entity, &GameState->Entropy, GameState->World->ChunkDim); */
  }

  Entity->P = Canonicalize(WorldChunkDim, Entity->P);

  collision_event AssertCollision = GetCollision(World, Entity);
  /* Assert(AssertCollision.Count == 0); */

  // Entites that aren't moving can still be positioned outside the world if
  // the player moves the world to do so
  if (AssertCollision.Count)
    EntityWorldCollision(World, Entity, &C, VisibleRegion);

  return;
}

#if 0
void
SimulateEnemy(game_state *GameState, entity *Enemy, r32 dt)
{
  v3 PlayerP = GetAbsoluteP(Player->P, GameState->World->ChunkDim);
  v3 EnemyP = GetAbsoluteP(Enemy->P, GameState->World->ChunkDim);

  if ( EnemyP.y > PlayerP.y ) // Enemy is in front of Player
  {
    v3 EnemyToPlayer = Normalize(PlayerP - EnemyP);
    Enemy->Physics.Force += EnemyToPlayer*dt;

    Enemy->FireCooldown -= dt;
    if ( Enemy->FireCooldown < 0 )
    {
      v3 ProjectileDirection = Normalize(Enemy->Physics.Velocity);
      if ( LengthSq(ProjectileDirection) == 0)
      {
        ProjectileDirection = EnemyToPlayer;
      }

      SpawnProjectile(GameState, &Enemy->P, ProjectileDirection*PROJECTILE_SPEED , EntityType_EnemyProjectile);
      Enemy->FireCooldown = Enemy->RateOfFire;
    }
  }

  return;
}
#endif

void
UpdateVisibleRegion(world* World, world_position WorldDisp)
{
  World->Center += WorldDisp;
}

#if 1
link_internal particle *
SpawnParticle(particle_system *System)
{
  particle *Particle = &System->Particles[System->ActiveParticles++];
  Assert(System->ActiveParticles < PARTICLES_PER_SYSTEM);

  r32 X = RandomBilateral(&System->Entropy);
  r32 Y = RandomBilateral(&System->Entropy);
  r32 Z = RandomBilateral(&System->Entropy);

  // NOTE(Jesse): Normalizing this vector makes the spawn region spherical.
  // Not sure how we want to control that, but it should probably be parameterized
  v3 Random = Normalize(V3(X,Y,Z));

  Particle->Offset = (Random*System->SpawnRegion.Radius) + System->SpawnRegion.Center;

  v3 TurbMin = System->ParticleTurbMin;
  v3 TurbMax = System->ParticleTurbMax;

  switch(System->SpawnType)
  {
    InvalidCase(ParticleSpawnType_None);

    case ParticleSpawnType_Random:
    {
      r32 TurbX = MapValueToRange(TurbMin.x, Abs(X), TurbMax.x);
      r32 TurbY = MapValueToRange(TurbMin.y, Abs(Y), TurbMax.y);
      r32 TurbZ = MapValueToRange(TurbMin.z, Abs(Z), TurbMax.z);
      Particle->Velocity = V3(TurbX, TurbY, TurbZ);
    } break;

    case ParticleSpawnType_Expanding:
    {
      r32 RangeX = TurbMax.x - TurbMin.x;
      r32 RangeY = TurbMax.y - TurbMin.y;
      r32 RangeZ = TurbMax.z - TurbMin.z;
      Particle->Velocity = Random * V3(RangeX, RangeY, RangeZ);
    } break;

    case ParticleSpawnType_Contracting:
    {
      r32 RangeX = TurbMax.x - TurbMin.x;
      r32 RangeY = TurbMax.y - TurbMin.y;
      r32 RangeZ = TurbMax.z - TurbMin.z;
      Particle->Velocity = -1.f * Random * V3(RangeX, RangeY, RangeZ);
    } break;
  }

  Particle->RemainingLifespan = System->ParticleLifespan + RandomBetween(0.f,  &System->Entropy, System->LifespanMod);

  return Particle;
}
#endif

inline b32
ShouldEmit(particle_system *System)
{
  b32 Result = (System->EmissionLifespan == f32_MAX || System->EmissionLifespan > 0);
  return Result;
}

void
DoLight(game_lights *Lights, v3 Position, v3 Color)
{
  Assert(Lights->Count < MAX_LIGHTS);

  if (Lights->Count < MAX_LIGHTS)
  {
    light *Light = Lights->Lights + Lights->Count++;
    Light->Position = Position;
    Light->Color = Color;
  }

 return;
}

link_internal void
SimulateParticle(particle_system *System, particle *Particle, r32 dt, v3 EntityDelta)
{
  Particle->Velocity -= (Particle->Velocity * System->Drag * dt);
  v3 Delta = Particle->Velocity * dt;

  Particle->Offset += Delta;
  Particle->Offset -= EntityDelta * System->SystemMovementCoefficient;
  Particle->RemainingLifespan -= dt;
}

void
SimulatePlayer(world* World, entity *Player, camera* Camera, hotkeys *Hotkeys, r32 dt, chunk_dimension VisibleRegion)
{
  TIMED_FUNCTION();

  if (Spawned(Player))
  {
    /* DebugLine("force(%d, %d, %d)", Player->Physics.Force.x, Player->Physics.Force.y, Player->Physics.Force.z ); */
    PhysicsUpdate(&Player->Physics, dt);

    world_position OriginalPlayerP = Player->P.WorldP;
    MoveEntityInWorld( World, Player, Player->Physics.Delta, VisibleRegion);

    /* world_position WorldDisp = ( Player->P.WorldP - OriginalPlayerP ); */
    /* UpdateVisibleRegion(World, WorldDisp); */

#if 0
    Player->FireCooldown -= dt;

    // Regular Fire
    if ( Hotkeys->Player_Fire && (Player->FireCooldown < 0) )
    {
      canonical_position SpawnP = Canonicalize(Player->P + V3(0, 0, 2));
      v3 InitialProjVelocity = PROJECTILE_SPEED * (Normalize(V3(Camera->Front.x, Camera->Front.y, 0.25f) ));
      entity *Projectile = SpawnProjectile(GameState, &SpawnP, InitialProjVelocity, EntityType_PlayerProjectile);
      SpawnFire(Projectile, &GameState->Entropy, V3(0));
      Player->FireCooldown = Player->RateOfFire;
    }

    // Proton Torpedo!!
    if ( Hotkeys->Player_Proton && (Player->FireCooldown < 0) )
    {
      SpawnProjectile(GameState, &Player->P, V3(0,1,0), EntityType_PlayerProton);
      Player->FireCooldown = Player->RateOfFire;
    }
#endif

  }

  return;
}

link_internal void SimulateParticleSystem(work_queue_entry_sim_particle_system *Job);

void
SimulateEntities(engine_resources *Resources, r32 dt, chunk_dimension VisibleRegion, untextured_3d_geometry_buffer *Dest, work_queue *Queue)
{
  TIMED_FUNCTION();
  UNPACK_ENGINE_RESOURCES(Resources);

  for ( s32 EntityIndex = 0;
        EntityIndex < TOTAL_ENTITY_COUNT;
        ++EntityIndex )
  {
    entity *Entity = EntityTable[EntityIndex];

    if (!Spawned(Entity))
        continue;

    if (Entity->Update)
    {
      Entity->Update(Resources, Entity);
    }

    switch (Entity->Type)
    {
      case EntityType_None: { InvalidCodePath(); } break;

      case EntityType_PlayerProton:
      case EntityType_Loot:
      case EntityType_Static:
      {
      } break;

      case EntityType_Enemy:
      {
      } break;

      case EntityType_PlayerProjectile:
      case EntityType_EnemyProjectile:
      {
        PhysicsUpdate(&Entity->Physics, dt);
        MoveEntityInWorld(World, Entity, Entity->Physics.Delta, VisibleRegion);
      } break;

      case EntityType_ParticleSystem:
      {
        particle_system *System = Entity->Emitter;
        if (Inactive(System))
        {
          Unspawn(Entity);
        }
        else
        {
          PhysicsUpdate(&Entity->Physics, dt);
          MoveEntityInWorld(World, Entity, Entity->Physics.Delta, VisibleRegion);
        }

      } break;

      case EntityType_Player:
      {
        SimulatePlayer(World, Entity, Camera, Hotkeys, dt, VisibleRegion);
      } break;

      case EntityType_Default:
      {
        /* PhysicsUpdate(&Entity->Physics, dt); */
        /* MoveEntityInWorld(World, Entity, Entity->Physics.Delta, VisibleRegion); */
      } break;

      /* default: { InvalidCodePath(); } break; */
    }

    particle_system *System = Entity->Emitter;
    if (Active(System))
    {
      auto EntityDelta = Entity->Physics.Delta;

      v3 RenderSpaceP  = GetRenderP(Entity->P, Camera, World->ChunkDim);
      auto Job = WorkQueueEntry(System, Dest, EntityDelta, RenderSpaceP, dt);
      /* SimulateParticleSystem(&Job.work_queue_entry_sim_particle_system); */
      PushWorkQueueEntry(Queue, &Job);
    }
  }

  return;
}

#if BONSAI_NETWORK_IMPLEMENTATION
entity *
GetPlayer(entity **Players, client_state *OurClient)
{
  entity *Player = Players[0];

  if (OurClient)
  {
    Player = Players[OurClient->Id];
  }

  return Player;
}
#endif

link_internal void
SimulateParticleSystem(work_queue_entry_sim_particle_system *Job)
{
  TIMED_FUNCTION();

  auto System      = Job->System;
  auto EntityDelta = Job->EntityDelta;
  auto dt          = Job->dt;
  v3 RenderSpaceP  = Job->RenderSpaceP;

  if (System->EmissionLifespan < PARTICLE_SYSTEM_EMIT_FOREVER)
  {
    System->EmissionLifespan -= dt;
  }

  System->ElapsedSinceLastEmission += dt;

  u32 SpawnCount = (u32)(System->ElapsedSinceLastEmission * System->ParticlesPerSecond);

  r32 SpawnInterval = 1.f/System->ParticlesPerSecond;

  if (System->EmissionLifespan > 0)
  {
    for (u32 SpawnIndex = 0; SpawnIndex < SpawnCount; ++SpawnIndex)
    {
      /* particle *Particle = System->Particles + System->ActiveParticles + SpawnIndex; */
      particle *Particle = SpawnParticle(System);
      SimulateParticle(System, Particle, SpawnIndex*SpawnInterval, EntityDelta);
    }
  }

  System->ElapsedSinceLastEmission -= (r32)SpawnCount*SpawnInterval;

  if (System->ActiveParticles)
  {
    auto Dest = ReserveBufferSpace(Job->Dest, System->ActiveParticles*VERTS_PER_PARTICLE);
    for ( u32 ParticleIndex = 0;
          ParticleIndex < System->ActiveParticles;
          )
    {
      particle *Particle = &System->Particles[ParticleIndex];

      if (Particle->RemainingLifespan > 0.f)
      {
        v3 MinDiameter = System->ParticleStartingDim * System->ParticleEndingDim;
        r32 MaxParticleLifespan = (System->ParticleLifespan+System->LifespanMod);
        r32 t = (Particle->RemainingLifespan / MaxParticleLifespan);
        v3 Diameter = Lerp(t, MinDiameter, System->ParticleStartingDim);

        u8 ColorIndex = (u8)((Particle->RemainingLifespan / MaxParticleLifespan) * (PARTICLE_SYSTEM_COLOR_COUNT-0.0001f));
        Assert(ColorIndex >= 0 && ColorIndex < PARTICLE_SYSTEM_COLOR_COUNT);

        DrawVoxel( &Dest, RenderSpaceP + Particle->Offset, System->Colors[ColorIndex], Diameter, 3.0f );

#if 0
        v3 EmissionColor = Normalize(V3(3,1,0));
        if (RandomUnilateral(&System->Entropy) > 0.99f)
        {
          DoLight(Graphics->Lights, RenderSpaceP + Particle->Offset, EmissionColor);
        }
#endif
      }

      SimulateParticle(System, Particle, dt, EntityDelta);
      if ( Particle->RemainingLifespan < 0)
      {
        // Swap out last partcile for the current partcile and decrement
        particle *SwapParticle = &System->Particles[--System->ActiveParticles];
        *Particle = *SwapParticle;
      }
      else
      {
        ++ParticleIndex;
      }
    }
  }

#if 0
  v3 RenderSpaceP = GetRenderP(Entity->P, Graphics->Camera, WorldChunkDim) + System->SpawnRegion.Center;
  DoLight(Graphics->Lights, RenderSpaceP, 10.0f*EmissionColor);
#endif
}

inline b32
IsGrounded( world *World, entity *entity )
{
  collision_event c = GetCollision(World, entity, V3(0.0f, 0.0f, -0.0001f));
  return c.Count > 0;
}

link_internal v3
GetSimSpaceCenterP(entity *E, v3 SimSpaceP)
{
  v3 Result = SimSpaceP + E->CollisionVolumeRadius;
  return Result;
}

link_internal v3
GetSimSpaceBaseP(entity *E, v3 SimSpaceP)
{
  v3 Result = SimSpaceP + E->CollisionVolumeRadius.xy;
  return Result;
}

link_internal v3
GetSimSpaceP(world *World, entity *Entity)
{
  v3 Result = GetSimSpaceP(World, Entity->P);
  return Result;
}

link_internal v3
GetSimSpaceBaseP(world *World, entity *E)
{
  v3 SimSpaceP = GetSimSpaceP(World, E);
  v3 Result = GetSimSpaceBaseP(E, SimSpaceP);
  return Result;
}

link_internal aabb
GetSimSpaceAABB(world *World, entity *Entity)
{
  v3 SimSpaceP = GetSimSpaceP(World, Entity);
  aabb Result = AABBMinRad(SimSpaceP, Entity->CollisionVolumeRadius);
  return Result;
}

link_internal u32_buffer
GatherEntitiesIntersecting(world *World, entity **EntityTable, sphere *S, memory_arena *Memory)
{
  u32_stream Stream = {};
  for (u32 EIndex = 0; EIndex < TOTAL_ENTITY_COUNT; ++EIndex)
  {
    entity *E = EntityTable[EIndex];
    if (E)
    {
      aabb EntityAABB = GetSimSpaceAABB(World, E);

      /* v3 ESimP = GetSimSpaceP(World, E->P); */
      /* v3 ESimCenterP = GetSimSpaceCenterP(E, ESimP); */

      if (Intersect(&EntityAABB, S))
      {
        Push(&Stream, EIndex);
      }
    }
  }
  u32_buffer Result = Compact(&Stream, Memory);
  return Result;
}

link_internal b32
Intersect(world *World, ray *Ray, entity *Entity)
{
  aabb EntityAABB = GetSimSpaceAABB(World, Entity);
  b32 Result = (Intersect(EntityAABB, Ray) != f32_MAX);
  return Result;
}

link_internal entity *
GetEntitiesIntersectingRay(world *World, entity **EntityTable, ray *Ray)
{
  entity *Result = {};
  for ( s32 EntityIndex = 0; EntityIndex < TOTAL_ENTITY_COUNT; ++EntityIndex )
  {
    entity *Entity = EntityTable[EntityIndex];
    if (!Spawned(Entity)) continue;

    if (Intersect(World, Ray, Entity))
    {
      Result = Entity;
    }
  }

  return Result;
}

link_internal entity *
RayTraceEntityCollision(engine_resources *Resources, ray *Ray )
{
  entity *Result = GetEntitiesIntersectingRay(Resources->World, Resources->EntityTable, Ray);
  return Result;
}


link_internal entity *
MousePickEntity(engine_resources *Resources)
{
  TIMED_FUNCTION();
  UNPACK_ENGINE_RESOURCES(Resources);

  entity *Result = {};

  maybe_ray MaybeRay = ComputeRayFromCursor(Plat, &gBuffer->ViewProjection, Camera, World->ChunkDim);

  if (MaybeRay.Tag == Maybe_Yes)
  {

    v3 SimOrigin = GetSimSpaceP(World, Canonical_Position(World->ChunkDim, MaybeRay.Ray.Origin, V3i(0)));
    ray SimRay = {SimOrigin, MaybeRay.Ray.Dir};
    Result = RayTraceEntityCollision( Resources, &SimRay );
  }

  return Result;
}


link_internal void
RebuildWorldChunkMesh(thread_local_state *Thread, world_chunk *Chunk)
{
  Assert( IsSet(Chunk->Flags, Chunk_VoxelsInitialized) );

  untextured_3d_geometry_buffer *NewMesh = 0;

  {
    untextured_3d_geometry_buffer *GeneratedMesh = GetMeshForChunk(&Thread->EngineResources->MeshFreelist, Thread->PermMemory);

    BuildWorldChunkMeshFromMarkedVoxels_Greedy( Chunk->Voxels, Chunk->Dim, {}, Chunk->Dim, GeneratedMesh, GetTranArena() );

    if (GeneratedMesh->At)
    {
      NewMesh = GeneratedMesh;
    }
    else
    {
      DeallocateMesh(&GeneratedMesh, &Thread->EngineResources->MeshFreelist, Thread->PermMemory);
    }
  }

  #ifdef __APPLE__
  umm Timestamp = NewMesh ? NewMesh->Timestamp : __builtin_readcyclecounter();
  #else
  umm Timestamp = NewMesh ? NewMesh->Timestamp : __rdtsc();
  #endif

  auto Replaced = AtomicReplaceMesh(&Chunk->Meshes, MeshBit_Main, NewMesh, Timestamp);
  if (Replaced) { DeallocateMesh(&Replaced, &Thread->EngineResources->MeshFreelist, Thread->PermMemory); }
}
