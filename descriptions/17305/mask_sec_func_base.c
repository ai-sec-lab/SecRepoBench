static int
dissect_rrc_PLMN_IdentityWithOptionalMCC_r6(tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 862 "./asn1/rrc/rrc.cnf"
  wmem_strbuf_t* mcc_mnc_strbuf;
  wmem_strbuf_t* temp_strbuf;
  wmem_strbuf_t* last_mcc_strbuf;
  guint32 string_len;
  gchar* mcc_mnc_string;
  tvbuff_t* mcc_mnc_tvb;

  /* Reset the digits string in the private data struct */
  /* Maximal length: 7 = 3 digits MCC + 3 digits MNC + trailing '\0' */
  mcc_mnc_strbuf = wmem_strbuf_sized_new(actx->pinfo->pool,7,7);
  private_data_set_digits_strbuf(actx, mcc_mnc_strbuf);
  /* Reset parsing failure flag*/
  private_data_set_digits_strbuf_parsing_failed_flag(actx, FALSE);
  offset = dissect_per_sequence(tvb, offset, actx, tree, hf_index,
                                   ett_rrc_PLMN_IdentityWithOptionalMCC_r6, PLMN_IdentityWithOptionalMCC_r6_sequence);

  private_data_set_digits_strbuf(actx, NULL);
  /* Check for parsing errors */
  if(private_data_get_digits_strbuf_parsing_failed_flag(actx)) {
    return offset;
  }

  /* Extracting the string collected in the strbuf */
  string_len = (guint32)wmem_strbuf_get_len(mcc_mnc_strbuf);
  mcc_mnc_string = wmem_strbuf_finalize(mcc_mnc_strbuf);
  if (string_len > 3) {
      /* 3 MCC digits and at least 1 MNC digit were found, keep MCC for later
         in case it's missing in other PLMN ids*/
    temp_strbuf = wmem_strbuf_sized_new(actx->pinfo->pool,4,4);
    wmem_strbuf_append_c(temp_strbuf,mcc_mnc_string[0]);
    wmem_strbuf_append_c(temp_strbuf,mcc_mnc_string[1]);
    wmem_strbuf_append_c(temp_strbuf,mcc_mnc_string[2]);
    wmem_strbuf_append_c(temp_strbuf,'\0');
    private_data_set_last_mcc_strbuf(actx,temp_strbuf);
  }
  else {
      /* mcc_mnc_strbuf Probably only has 3/2 digits of MNC */
      /* Try to fill MCC form "last MCC" if we have it stored */
      last_mcc_strbuf = private_data_get_last_mcc_strbuf(actx);
      if(last_mcc_strbuf)
      {
        // <MASK>
      }
  }

  if (string_len >= 5) {
    /* optional MCC was present (or restored above), we can call E.212 dissector */

    /* Creating TVB from extracted string*/
    mcc_mnc_tvb = tvb_new_child_real_data(tvb, (guint8*)mcc_mnc_string, string_len, string_len);
    add_new_data_source(actx->pinfo, mcc_mnc_tvb, "MCC-MNC");

    /* Calling E.212 */
    dissect_e212_mcc_mnc_in_utf8_address(mcc_mnc_tvb, actx->pinfo, tree, 0);
  }



  return offset;
}