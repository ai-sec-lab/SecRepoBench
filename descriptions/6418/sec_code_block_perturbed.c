{    /* AFI is good and length is long enough  */

    /* The AFI is either ISO DCC, binary or ISO 6523-ICD, binary,
     * and the area length corresponds either to the area length
     * for RFC 1237 (GOSIP) addresses or that length + 1.
     *
     * XXX - RFC 1237 doesn't mention ISO DCC, binary, as a valid
     * AFI; is that from GOSIP Version 1?  If it's ISO DCC, binary,
     * the IDI is 3 digits, i.e. 1 1/2 octets.
     */
    /* there used to be a check for (length > RFC1237_FULLAREA_LEN + 1) here,
     * in order to report an invalied length of AREA for DCC / ISO 6523 AFI,
     * but that can *never* be the case because the if() test above explicitly
     * tests for (length == RFC1237_FULLAREA_LEN) or (length == RFC1237_FULLAREA_LEN + 1)
     */

    /* Show the one-octet AFI, the two-octet IDI, the one-octet DFI, the
     * 3-octet AA, and the 2 reserved octets.
     */
    cur += g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "[%02x|%02x:%02x][%02x|%02x:%02x:%02x|%02x:%02x]",
                    ad[0], ad[1], ad[2], ad[3], ad[4],
                    ad[5], ad[6], ad[7], ad[8] );
    /* Show the 2-octet RD and the 2-octet Area. */
    cur += g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "[%02x:%02x|%02x:%02x]",
                    ad[9], ad[10],  ad[11], ad[12] );
    /* Show whatever the heck this is; it's not specified by RFC 1237,
     * but we also handle 14-octet areas.  Is it the "Designated IS"
     * stuff mentioned below?  (I didn't find anything in the IS-IS
     * spec about that.)
     */
    if ( (RFC1237_FULLAREA_LEN + 1)*2 == length )
      g_snprintf(cur, (gulong) (buf_len-(cur-addressbuf)), "-[%02x]", ad[13] );
  }
  else {
    /* This doesn't look like a full RFC 1237 IS-IS area, so all we know
     * is that the first octet is an AFI.  Print it separately from all
     * the other octets.
     */
    if ( length == RFC1237_AREA_LEN*2 ) {
      /* XXX - RFC1237_AREA_LEN, which is 3 octets, doesn't seem to
       * correspond to anything in RFC 1237.  Where did it come from?
       */
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