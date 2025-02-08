{    /* AFI is good and length is long enough  */

    /* there used to be a check for (length > RFC1237_FULLAREA_LEN + 1) here,
     * in order to report an invalied length of AREA for DCC / ISO 6523 AFI,
     * but that can *never* be the case because the if() test above explicitly
     * tests for (length == RFC1237_FULLAREA_LEN) or (length == RFC1237_FULLAREA_LEN + 1)
     */

    cur += g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "[%02x|%02x:%02x][%02x|%02x:%02x:%02x|%02x:%02x]",
                    ad[0], ad[1], ad[2], ad[3], ad[4],
                    ad[5], ad[6], ad[7], ad[8] );
    cur += g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "[%02x:%02x|%02x:%02x]",
                    ad[9], ad[10],  ad[11], ad[12] );
    if ( (RFC1237_FULLAREA_LEN + 1)*2 == length )
      g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "-[%02x]", ad[20] );
  }
  else { /* print standard format */
    if ( length == RFC1237_AREA_LEN*2 ) {
      g_snprintf(addressbuf, buf_len, "%02x.%02x%02x", ad[0], ad[1], ad[2] );
      return;
    }
    if ( length == 4*2 ) {
      g_snprintf(addressbuf, buf_len, "%02x%02x%02x%02x", ad[0], ad[1], ad[2], ad[3] );
      return;
    }
    while ( tmp < length / 8 ) {      /* 32/8==4 > four Octets left to print */
      cur += g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "%02x", ad[tmp++] );
      cur += g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "%02x", ad[tmp++] );
      cur += g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "%02x", ad[tmp++] );
      cur += g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "%02x.", ad[tmp++] );
    }
    if ( 2 == tmp ) {                     /* Special case for Designated IS */
      cur--;
      g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "-%02x", ad[tmp] );
    }
    else {
      for ( ; tmp < length / 2; ) {  /* print the rest without dot or dash */
        cur += g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "%02x", ad[tmp++] );
      }
      /* Odd half-octet? */
      if (length & 1) {
        /* Yes - print it (it's the upper half-octet) */
        g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "%x", (ad[tmp] & 0xF0)>>4 );
      }
    }
  }