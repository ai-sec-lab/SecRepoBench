static void  Ins_PUSHW( INS_ARG )
  {
    Int  L, K;

    L = CUR.opcode - 0xB8 + 1;

    if ( BOUNDS( L, CUR.stackSize+1-CUR.top )
      || BOUNDS( CUR.IP + (L * 2), CUR.codeSize ))
    {
      CUR.error = TT_Err_Stack_Overflow;
      return;
    }

    CUR.IP++;

    for ( K = 0; K < L; K++ )
      { args[K] = GET_ShortIns();
        DBG_PRINT1(" %d", args[K]);
      }

    CUR.step_ins = FALSE;
  }