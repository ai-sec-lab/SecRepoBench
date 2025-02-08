if ( BOUNDS( L, CUR.stackSize+1-CUR.top ) )
    {
      CUR.error = TT_Err_Stack_Overflow;
      return;
    }

    for ( index = 1; index <= L; index++ )
      { args[index - 1] = CUR.code[CUR.IP + index];
        DBG_PRINT1(" %d", args[index - 1]);
      }