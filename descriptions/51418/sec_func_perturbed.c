static void  Ins_PUSHB( INS_ARG )
  {
    Int  L, index;

    L = ((Int)CUR.opcode - 0xB0 + 1);

    if ( BOUNDS( L, CUR.stackSize+1-CUR.top )
      || BOUNDS( CUR.IP + L, CUR.codeSize ))
    {
      CUR.error = TT_Err_Stack_Overflow;
      return;
    }

    for ( index = 1; index <= L; index++ )
      { args[index - 1] = CUR.code[CUR.IP + index];
        DBG_PRINT1(" %d", args[index - 1]);
      }
  }