static void
image_simple_expand(byte * line, int line_x, uint raster,
                    const byte * buffer, int data_x, uint w,
                    fixed xcur, fixed x_extent, byte zero /* 0 or 0xff */ )
{
    int dbitx = data_x & 7;
    byte sbit = 0x80 >> dbitx;
    byte sbitmask = 0xff >> dbitx;
    uint wx = dbitx + w;
    gx_dda_fixed xl;
    gx_dda_step_fixed dxx4, dxx8, dxx16, dxx24, dxx32;
    register const byte *psrc = buffer + (data_x >> 3);

    /*
     * The following 3 variables define the end of the input data row.
     * We would put them in a struct, except that no compiler that we
     * know of will optimize individual struct members as though they
     * were simple variables (e.g., by putting them in registers).
     *
     * endp points to the byte that contains the bit just beyond the
     * end of the row.  endx gives the bit number of this bit within
     * the byte, with 0 being the *least* significant bit.  endbit is
     * a mask for this bit.
     */
    const byte *endp = psrc + (wx >> 3);
    int endx = ~wx & 7;
    byte endbit = 1 << endx;

    /*
     * The following 3 variables do the same for start of the last run
     * of the input row (think of it as a pointer to just beyond the
     * end of the next-to-last run).
     */
    const byte *stop = endp;
    int stopx;
    byte stopbit = endbit;
    byte data;
    byte one = ~zero;
    fixed xl0;
#ifdef PACIFY_VALGRIND
    byte vbits;
#endif

    if (w == 0)
        return;
    INCS(calls);

    /* Scan backward for the last transition. */
    if (stopbit == 0x80)
        --stop, stopbit = 1;
    else
        stopbit <<= 1;
    /* Now (stop, stopbit) give the last bit of the row. */
#ifdef PACIFY_VALGRIND
    /* Here, we are dealing with a row of bits, rather than bytes.
     * If the width of the bits is not a multiple of 8, we don't
     * fill out the last byte, and valgrind (correctly) tracks the
     * bits in that last byte as being a mix of defined and undefined.
     * When we are scanning through the row bitwise, everything works
     * fine, but our "skip whole bytes" code can confuse valgrind.
     * We know that we won't match the "data == 0xff" for the final
     * byte (i.e. the undefinedness of some of the bits doesn't matter
     * to the correctness of the routine), but valgrind is not smart
     * enough to realise that we know this. Accordingly, we get a false
     * positive "undefined memory read".
     * How do we fix this? Well, one way would be to read in the
     * partial last byte, and explicitly set the undefined bits to
     * be 0.
     *   *stop &= ~(stopbit-1);
     * Unfortunately, stop is a const *, so we can't do that (we could
     * break const, but it is just conceivable that the caller might
     * pass the next string of bits out in a later call, and so we
     * might be corrupting valid data).
     * Instead, we make a call to a valgrind helper. */
    VALGRIND_GET_VBITS(stop,&vbits,1);
    if ((vbits & stopbit)==0) { /* At least our stop bit must be addressable already! */
      byte zero = 0;
      VALGRIND_SET_VBITS(stop,&zero,1);
    }
#endif
    {
        byte stopmask = -stopbit << 1;
        byte last = *stop;

        if (stop == psrc)	/* only 1 input byte */
            stopmask &= sbitmask;
        if (last & stopbit) {
            /* The last bit is a 1: look for a 0-to-1 transition. */
            if (~last & stopmask) {	/* Transition in last byte. */
                last |= stopbit - 1;
            } else {		/* No transition in the last byte. */
                while (stop > psrc && stop[-1] == 0xff)
                    --stop;
                if (stop == psrc ||
                    (stop == psrc + 1 && !(~*psrc & sbitmask))
                    ) {
                    /* The input is all 1s.  Clear the row and exit. */
                    INCS(all1s);
                    fill_row(line, line_x, raster, one);
                    goto end;
                }
                last = *--stop;
            }
            stopx = byte_bit_run_length_0[byte_reverse_bits[last]] - 1;
        } else {
            /* The last bit is a 0: look for a 1-to-0 transition. */
            if (last & stopmask) {	/* Transition in last byte. */
                last &= -stopbit;
            } else {		/* No transition in the last byte. */
                while (stop > psrc && stop[-1] == 0)
                    --stop;
                if (stop == psrc ||
                    (stop == psrc + 1 && !(*psrc & sbitmask))
                    ) {
                    /* The input is all 0s.  Clear the row and exit. */
                    INCS(all0s);
                    fill_row(line, line_x, raster, zero);
                    goto end;
                }
                last = *--stop;
            }
            stopx = byte_bit_run_length_0[byte_reverse_bits[last ^ 0xff]] - 1;
        }
        if (stopx < 0)
            stopx = 7, ++stop;
        stopbit = 1 << stopx;
    }

    /* Pre-clear the row. */
    fill_row(line, line_x, raster, zero);


    /* Extreme negative values of x_extent cause the xl0 calculation
     * to explode. Workaround this here. */
    if (x_extent < min_int + 0x100)
      x_extent += 0x100;

    /* Set up the DDAs. */
    xl0 =
        (x_extent >= 0 ?
         fixed_fraction(fixed_pre_pixround(xcur)) :
         fixed_fraction(fixed_pre_pixround(xcur + x_extent)) - x_extent);
    xl0 += int2fixed(line_x);
    /* We should never get a negative x10 here. If we do, all bets are off. */
    if (xl0 < 0)
        xl0 = 0, x_extent = 0;
    dda_init(xl, xl0, x_extent, w);
    dxx4 = xl.step;
    dda_step_add(dxx4, xl.step);
    /* egcc - 2.91.66 generates incorrect code for
     * dda_step_add(dxx4, dxx4);
     * Using the temp variable.
     */
    dxx8 = dxx4;
    dda_step_add(dxx4, dxx8);
    dxx8 = dxx4;
    dda_step_add(dxx8, dxx4);
    dxx16 = dxx8;
    dda_step_add(dxx16, dxx8);
    dxx24 = dxx16;
    dda_step_add(dxx24, dxx8);
    dxx32 = dxx24;
    dda_step_add(dxx32, dxx8);

    /*
     * Loop invariants:
     *      data = *psrc;
     *      sbit = 1 << n, 0<=n<=7.
     */
    for (data = *psrc;;) {
        int x0, n, bit;
        byte *bp;
        static const byte lmasks[9] = {
            0xff, 0x7f, 0x3f, 0x1f, 0xf, 7, 3, 1, 0
        };
        static const byte rmasks[9] = {
            0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
        };

        INCS(runs);

        /* Scan a run of zeros. */
        data ^= 0xff;		/* invert */
        while (data & sbit) {
            dda_next(xl);
            sbit >>= 1;
            INCS(lbit0);
        }
        if (!sbit) {		/* Scan a run of zero bytes. */
sw:	    if ((data = psrc[1]) != 0) {
                psrc++;
                INCS(byte00);
            } else if ((data = psrc[2]) != 0) {
                dda_state_next(xl.state, dxx8);
                psrc += 2;
                INCS(byte01);
            } else if ((data = psrc[3]) != 0) {
                dda_state_next(xl.state, dxx16);
                psrc += 3;
                INCS(byte02);
            } else if ((data = psrc[4]) != 0) {
                dda_state_next(xl.state, dxx24);
                psrc += 4;
                INCS(byte03);
            } else {
                dda_state_next(xl.state, dxx32);
                psrc += 4;
                INCS(byte04);
                goto sw;
            }
            if (data > 0xf)
                sbit = 0x80;
            else {
                sbit = 0x08;
                dda_state_next(xl.state, dxx4);
            }
            data ^= 0xff;	/* invert */
            while (data & sbit) {
                dda_next(xl);
                sbit >>= 1;
                INCS(rbit0);
            }
        }
        x0 = dda_current_fixed2int(xl);
        if (psrc >= stop && sbit == stopbit) {
            /*
             * We've scanned the last run of 0s.
             * Prepare to fill the final run of 1s.
             * Use int64_t to avoid overflow.
             */
            n = fixed2int((int64_t)xl0 + (int64_t)x_extent) - x0;
        } else {		/* Scan a run of ones. */
            /* We know the current bit is a one. */
            data ^= 0xff;	/* un-invert */
            do {
                dda_next(xl);
                sbit >>= 1;
                INCS(lbit1);
            }
            while (data & sbit);
            if (!sbit) {	/* Scan a run of 0xff bytes. */
                while ((data = *++psrc) == 0xff) {
                    dda_state_next(xl.state, dxx8);
                    INCS(byte1);
                }
                if (data < 0xf0)
                    sbit = 0x80;
                else {
                    sbit = 0x08;
                    dda_state_next(xl.state, dxx4);
                }
                while (data & sbit) {
                    dda_next(xl);
                    sbit >>= 1;
                    INCS(rbit1);
                }
            }
            n = dda_current_fixed2int(xl) - x0;
        }

        /* Fill the run in the scan line. */
        if (n < 0)
            x0 += n, n = -n;
        bp = line + (x0 >> 3);
        bit = x0 & 7;
        if ((n += bit) <= 8) {
            *bp ^= lmasks[bit] - lmasks[n];
            INCS(thin);
        } else if ((n -= 8) <= 8) {
            *bp ^= lmasks[bit];
            bp[1] ^= rmasks[n];
            INCS(thin2);
        } else {
            *bp++ ^= lmasks[bit];
            if (n >= 56) {
                int nb = n >> 3;

                memset(bp, one, nb);
                bp += nb;
                INCS(nwide);
                ADDS(bwide, nb);
            } else {
                ADDS(bfill, n >> 3);
                while ((n -= 8) >= 0)
                    *bp++ = one;
                INCS(nfill);
            }
            *bp ^= rmasks[n & 7];
        }
        if (psrc >= stop && sbit == stopbit)
            break;
    }
 // <MASK>
}