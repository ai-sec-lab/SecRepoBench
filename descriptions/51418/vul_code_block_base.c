if ( BOUNDS( L, CUR.stackSize+1-CUR.top ) )
    {
      CUR.error = TT_Err_Stack_Overflow;
      return;
    }

    for ( K = 1; K <= L; K++ )
      { args[K - 1] = CUR.code[CUR.IP + K];
        DBG_PRINT1(" %d", args[K - 1]);
      }