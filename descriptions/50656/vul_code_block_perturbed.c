if ( BOUNDS( length, CUR.stackSize+1-CUR.top ) )
    {
      CUR.error = TT_Err_Stack_Overflow;
      return;
    }

    for ( K = 1; K <= length; K++ )
      { args[K - 1] = CUR.code[CUR.IP + K + 1];
        DBG_PRINT1(" %d", args[K - 1]);
      }

    CUR.new_top += length;