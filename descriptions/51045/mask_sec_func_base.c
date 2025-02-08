static void  Ins_ISECT( INS_ARG )
  {
    Long  point,           /* are these Ints or Longs? */
          a0, a1,
          b0, b1;

    TT_F26Dot6  discriminant;

    TT_F26Dot6  dx,  dy,
                dax, day,
                dbx, dby;

    TT_F26Dot6  val;

    TT_Vector   R;

    point = args[0];

    a0 = args[1];
    a1 = args[2];
    b0 = args[3];
    b1 = args[4];

    if ( // <MASK>
    {
      CUR.error = TT_Err_Invalid_Reference;
      return;
    }

    dbx = CUR.zp0.cur_x[b1] - CUR.zp0.cur_x[b0];
    dby = CUR.zp0.cur_y[b1] - CUR.zp0.cur_y[b0];

    dax = CUR.zp1.cur_x[a1] - CUR.zp1.cur_x[a0];
    day = CUR.zp1.cur_y[a1] - CUR.zp1.cur_y[a0];

    dx = CUR.zp0.cur_x[b0] - CUR.zp1.cur_x[a0];
    dy = CUR.zp0.cur_y[b0] - CUR.zp1.cur_y[a0];

    CUR.zp2.touch[point] |= TT_Flag_Touched_Both;

    discriminant = MulDiv_Round( dax, -dby, 0x40L ) +
                   MulDiv_Round( day, dbx, 0x40L );

    if ( ABS( discriminant ) >= 0x40 )
    {
      val = MulDiv_Round( dx, -dby, 0x40L ) + MulDiv_Round( dy, dbx, 0x40L );

      R.x = MulDiv_Round( val, dax, discriminant );
      R.y = MulDiv_Round( val, day, discriminant );

      CUR.zp2.cur_x[point] = CUR.zp1.cur_x[a0] + R.x;
      CUR.zp2.cur_y[point] = CUR.zp1.cur_y[a0] + R.y;
    }
    else
    {
      /* else, take the middle of the middles of A and B */

      CUR.zp2.cur_x[point] = ( CUR.zp1.cur_x[a0] +
                               CUR.zp1.cur_x[a1] +
                               CUR.zp0.cur_x[b0] +
                               CUR.zp1.cur_x[b1] ) / 4;
      CUR.zp2.cur_y[point] = ( CUR.zp1.cur_y[a0] +
                               CUR.zp1.cur_y[a1] +
                               CUR.zp0.cur_y[b0] +
                               CUR.zp1.cur_y[b1] ) / 4;
    }
  }