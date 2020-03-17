

    struct struct_def_cursor
    {
      struct_def* Start;
      struct_def* End;
      struct_def* At;
    };

    function struct_def_cursor
    StructDefCursor(umm ElementCount, memory_arena* Memory)
    {
      struct_def* Start = (struct_def*)PushStruct(Memory, sizeof(struct_def), 1, 1);
      struct_def_cursor Result = {
        .Start = Start,
        .End = Start+ElementCount,
        .At = Start,
      };
      return Result;
    };


