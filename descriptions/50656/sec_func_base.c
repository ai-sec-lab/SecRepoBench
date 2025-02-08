static void  Ins_NPUSHB( INS_ARG )
  {
    Int  L, K;

    L = (Int)CUR.code[CUR.IP + 1];

    if ( BOUNDS( L, CUR.stackSize+1-CUR.top )
      || BOUNDS( L, CUR.codeSize+1-CUR.IP))
    {
      CUR.error = TT_Err_Stack_Overflow;
      return;
    }

    for ( K = 1; K <= L; K++ )
      { args[K - 1] = CUR.code[CUR.IP + K + 1];
        DBG_PRINT1(" %d", args[K - 1]);
      }

    CUR.new_top += L;
  }