if ( BOUNDS( args[0], CUR.zp1.n_points ) )
    {
      CUR.error = TT_Err_Invalid_Reference;
      return;
    }

    /* XXX: Undocumented behaviour */

    if ( CUR.GS.gep0 == 0 )   /* if in twilight zone */
    {
      CUR.zp1.org_x[point] = CUR.zp0.org_x[CUR.GS.rp0];
      CUR.zp1.org_y[point] = CUR.zp0.org_y[CUR.GS.rp0];
      CUR.zp1.cur_x[point] = CUR.zp1.org_x[point];
      CUR.zp1.cur_y[point] = CUR.zp1.org_y[point];
    }

    distance = CUR_Func_project( CUR.zp1.cur_x[point] -
                                   CUR.zp0.cur_x[CUR.GS.rp0],
                                 CUR.zp1.cur_y[point] -
                                   CUR.zp0.cur_y[CUR.GS.rp0] );

    CUR_Func_move( &CUR.zp1, point, args[1] - distance );

    CUR.GS.rp1 = CUR.GS.rp0;
    CUR.GS.rp2 = point;

    if ( (CUR.opcode & 1) != 0 )
      CUR.GS.rp0 = point;