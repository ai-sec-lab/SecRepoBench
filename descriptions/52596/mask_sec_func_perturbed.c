static guint
dissect_compressed_beamforming_and_cqi(proto_tree *tree, tvbuff_t *tvb, packet_info *pinfo, int offset)
{
  int byte_count = 0;
  guint64 mimo_cntl = tvb_get_letoh40(tvb, offset);
  int nc = 0, nr = 0, i;
  int bw, grouping, codebook, feedback, bitpositionoffset, scidx;
  int phi_bits = 0, psi_bits = 0;
  proto_tree *snr_tree = NULL, *feedback_tree = NULL;
  int start_offset;
  int ru_start_index, ru_end_index;

  nc = (int)((mimo_cntl & 0x07) + 1);
  nr = (int)(((mimo_cntl >> 3) & 0x07) + 1);
  bw = (int)((mimo_cntl >> 6) & 0x03);
  grouping = (int)((mimo_cntl >> 8) & 0x01);
  codebook = (int)((mimo_cntl >> 9) & 0x01);
  feedback = (int)((mimo_cntl >> 10) & 0x03);
  ru_start_index = (int)((mimo_cntl >> 16) & 0x7F);
  ru_end_index = (int)((mimo_cntl >> 23) & 0x7F);

  proto_tree_add_bitmask_with_flags(tree, tvb, offset,
    hf_ieee80211_he_mimo_control_field, ett_ff_he_mimo_control,
    he_mimo_control_headers, ENC_LITTLE_ENDIAN,
    BMT_NO_APPEND);
  offset += 5;

  snr_tree = proto_tree_add_subtree(tree, tvb, offset, nc,
                ett_ff_he_mimo_beamforming_report_snr, NULL,
                "Average Signal to Noise Ratio");

  for (i = 0; i < nc; i++) {
    gint8 snr = tvb_get_gint8(tvb, offset);

    proto_tree_add_int_format(snr_tree,
        hf_ieee80211_he_compressed_beamforming_report_snr, tvb, offset, 1,
        snr, "Stream %d: %s%0.2fdB (0x%02x)", i, (snr == 127 ? ">=" :
                                                  (snr == -128 ? "<=" : "")),
                                             (float)((float)88 + snr)/4,
                                             (guint8)snr);
    offset++;
  }

  /*
   * The rest of the data consists of the compressed beamforming matrices, one
   * for each SCIDX per group. Each matrix consists of phi and psi angles
   * encoded using the number of bits specified using the codebook field.
   *
   * The matrices contain a number entries related to Nr -1 & Nc except when
   * Nr == Nc, and then it is Nr -1 x Nc -1, with Nr - 1 phi angles, Nc - 1
   * psi angles, Nr - 2 phi angles, Nc - 2 psi angles ...
   */
  if (feedback == 0) {  /* SU */
    if (codebook == 0) {
      psi_bits = 2; phi_bits = 4;
    } else {
      psi_bits = 4; phi_bits = 6;
    }
  } else if (feedback == 1) { /* MU */
    if (grouping == 1) {
      psi_bits = 9; phi_bits = 7;
    } else {
      if (codebook == 0) {
        psi_bits = 5; phi_bits = 7;
      } else {
        psi_bits = 7; phi_bits = 9;
      }
    }
  }  /* DO something about CQI etc. */

  feedback_tree = proto_tree_add_subtree(tree, tvb, offset, -1,
                        ett_ff_he_mimo_feedback_matrices, NULL,
                        "Feedback Matrices");

  start_offset = offset;
  bitpositionoffset = offset * 8;
  scidx = SCIDX_END_SENTINAL;
  while // <MASK>

  offset = (bitpositionoffset + 7) / 8;
  proto_item_set_len(feedback_tree, offset - start_offset);

  /* Sometimes the FCS is in the buffer as well ... */
  byte_count = tvb_reported_length_remaining(tvb, offset);
  if (byte_count > 0)
    offset += byte_count; /* Should fix the real problem */

  return offset;
}