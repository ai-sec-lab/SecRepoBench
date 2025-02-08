static void  Ins_PUSHB( INS_ARG )
  {
    Int  L, K;

    L = ((Int)CUR.opcode - 0xB0 + 1);

    if ( BOUNDS( L, CUR.stackSize+1-CUR.top )
      || BOUNDS( CUR.IP + L, CUR.codeSize ))
    {
      CUR.error = TT_Err_Stack_Overflow;
      return;
    }

    for ( K = 1; K <= L; K++ )
      { args[K - 1] = CUR.code[CUR.IP + K];
        DBG_PRINT1(" %d", args[K - 1]);
      }
  }