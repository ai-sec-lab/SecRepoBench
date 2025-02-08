/* GET_ShortIns() reads two values from the execution stream */
    if ( BOUNDS( L, CUR.stackSize+1-CUR.top )
      || BOUNDS( L * 2, CUR.codeSize+1-CUR.IP))
    {
      CUR.error = TT_Err_Stack_Overflow;
      return;
    }

    CUR.IP += 2;

    for ( index = 0; index < L; index++ )
      { args[index] = GET_ShortIns();
        DBG_PRINT1(" %d", args[index]);
      }

    CUR.step_ins = FALSE;
    CUR.new_top += L;