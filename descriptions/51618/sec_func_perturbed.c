static void  Ins_PUSHW( INS_ARG )
  {
    Int  length, K;

    length = CUR.opcode - 0xB8 + 1;

    if ( BOUNDS( length, CUR.stackSize+1-CUR.top )
      || BOUNDS( CUR.IP + (length * 2), CUR.codeSize ))
    {
      CUR.error = TT_Err_Stack_Overflow;
      return;
    }

    CUR.IP++;

    for ( K = 0; K < length; K++ )
      { args[K] = GET_ShortIns();
        DBG_PRINT1(" %d", args[K]);
      }

    CUR.step_ins = FALSE;
  }