if ( BOUNDS( L, CUR.stackSize+1-CUR.top ) )
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