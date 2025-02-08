static void
print_address_prefix_buf(const guint8 *ad, int length, gchar *addressbuf, int buf_len)
{
  gchar *cur;
  int  tmp  = 0;

  /* to do : all real area decoding now: NET is assumed if id len is 1 more byte
   */
  if (length <= 0 || length > MAX_AREA_LEN*2) {
    g_strlcpy(addressbuf, "<Invalid length of AREA>", buf_len);
    return;
  }

  cur = addressbuf;
  /* Check the AFI and length. */
  if ( (  ( NSAP_IDI_ISO_DCC_BIN      == *ad )
       || ( NSAP_IDI_ISO_6523_ICD_BIN == *ad )
       )
       &&
       (  ( RFC1237_FULLAREA_LEN*2       ==  length )
       || ( (RFC1237_FULLAREA_LEN + 1)*2 ==  length )
       )
     ) // <MASK>
}