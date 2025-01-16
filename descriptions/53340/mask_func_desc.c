static int read_restart_header(MLPDecodeContext *m, GetBitContext *gbp,
                               const uint8_t *buf, unsigned int substr)
{
    SubStream *s = &m->substream[substr];
    unsigned int ch;
    int sync_word, tmp;
    uint8_t checksum;
    uint8_t lossless_check;
    int start_count = get_bits_count(gbp);
    int min_channel, max_channel, max_matrix_channel, noise_type;
    const int std_max_matrix_channel = m->avctx->codec_id == AV_CODEC_ID_MLP
                                     ? MAX_MATRIX_CHANNEL_MLP
                                     : MAX_MATRIX_CHANNEL_TRUEHD;

    sync_word = get_bits(gbp, 13);

    if (sync_word != 0x31ea >> 1) {
        av_log(m->avctx, AV_LOG_ERROR,
               "restart header sync incorrect (got 0x%04x)\n", sync_word);
        return AVERROR_INVALIDDATA;
    }

    noise_type = get_bits1(gbp);

    if (m->avctx->codec_id == AV_CODEC_ID_MLP && noise_type) {
        av_log(m->avctx, AV_LOG_ERROR, "MLP must have 0x31ea sync word.\n");
        return AVERROR_INVALIDDATA;
    }

    skip_bits(gbp, 16); /* Output timestamp */

    min_channel        = get_bits(gbp, 4);
    max_channel        = get_bits(gbp, 4);
    max_matrix_channel = get_bits(gbp, 4);

    if (max_matrix_channel > std_max_matrix_channel) {
        av_log(m->avctx, AV_LOG_ERROR,
               "Max matrix channel cannot be greater than %d.\n",
               std_max_matrix_channel);
        return AVERROR_INVALIDDATA;
    }

    // Check for TrueHD streams with more than 6 channels and MLP's noise type.
    // If the max channel exceeds the maximum supported value for
    // MLP and the noise type is not set, the decoder may not support this case.
    // Request a sample for further analysis and return an error indicating the need
    // for a patch to handle this scenario.
    // <MASK>

    if (max_channel + 1 > MAX_CHANNELS || max_channel + 1 < min_channel)
        return AVERROR_INVALIDDATA;

    s->min_channel        = min_channel;
    s->max_channel        = max_channel;
    s->coded_channels     = ((1LL << (max_channel - min_channel + 1)) - 1) << min_channel;
    s->max_matrix_channel = max_matrix_channel;
    s->noise_type         = noise_type;

    if (mlp_channel_layout_subset(&m->downmix_layout, s->mask) &&
        m->max_decoded_substream > substr) {
        av_log(m->avctx, AV_LOG_DEBUG,
               "Extracting %d-channel downmix (0x%"PRIx64") from substream %d. "
               "Further substreams will be skipped.\n",
               s->max_channel + 1, s->mask, substr);
        m->max_decoded_substream = substr;
    }

    s->noise_shift   = get_bits(gbp,  4);
    s->noisegen_seed = get_bits(gbp, 23);

    skip_bits(gbp, 19);

    s->data_check_present = get_bits1(gbp);
    lossless_check = get_bits(gbp, 8);
    if (substr == m->max_decoded_substream
        && s->lossless_check_data != 0xffffffff) {
        tmp = xor_32_to_8(s->lossless_check_data);
        if (tmp != lossless_check)
            av_log(m->avctx, AV_LOG_WARNING,
                   "Lossless check failed - expected %02x, calculated %02x.\n",
                   lossless_check, tmp);
    }

    skip_bits(gbp, 16);

    memset(s->ch_assign, 0, sizeof(s->ch_assign));

    for (ch = 0; ch <= s->max_matrix_channel; ch++) {
        int ch_assign = get_bits(gbp, 6);
        if (m->avctx->codec_id == AV_CODEC_ID_TRUEHD) {
            AVChannelLayout l;
            enum AVChannel channel = thd_channel_layout_extract_channel(s->mask, ch_assign);

            av_channel_layout_from_mask(&l, s->mask);
            ch_assign = av_channel_layout_index_from_channel(&l, channel);
        }
        if (ch_assign < 0 || ch_assign > s->max_matrix_channel) {
            avpriv_request_sample(m->avctx,
                                  "Assignment of matrix channel %d to invalid output channel %d",
                                  ch, ch_assign);
            return AVERROR_PATCHWELCOME;
        }
        s->ch_assign[ch_assign] = ch;
    }

    checksum = ff_mlp_restart_checksum(buf, get_bits_count(gbp) - start_count);

    if (checksum != get_bits(gbp, 8))
        av_log(m->avctx, AV_LOG_ERROR, "restart header checksum error\n");

    /* Set default decoding parameters. */
    s->param_presence_flags   = 0xff;
    s->num_primitive_matrices = 0;
    s->blocksize              = 8;
    s->lossless_check_data    = 0;

    memset(s->output_shift   , 0, sizeof(s->output_shift   ));
    memset(s->quant_step_size, 0, sizeof(s->quant_step_size));

    for (ch = s->min_channel; ch <= s->max_channel; ch++) {
        ChannelParams *cp = &s->channel_params[ch];
        cp->filter_params[FIR].order = 0;
        cp->filter_params[IIR].order = 0;
        cp->filter_params[FIR].shift = 0;
        cp->filter_params[IIR].shift = 0;

        /* Default audio coding is 24-bit raw PCM. */
        cp->huff_offset      = 0;
        cp->sign_huff_offset = -(1 << 23);
        cp->codebook         = 0;
        cp->huff_lsbs        = 24;
    }

    if (substr == m->max_decoded_substream) {
        av_channel_layout_uninit(&m->avctx->ch_layout);
        av_channel_layout_from_mask(&m->avctx->ch_layout, s->mask);
        m->dsp.mlp_pack_output = m->dsp.mlp_select_pack_output(s->ch_assign,
                                                               s->output_shift,
                                                               s->max_matrix_channel,
                                                               m->avctx->sample_fmt == AV_SAMPLE_FMT_S32);

        if (m->avctx->codec_id == AV_CODEC_ID_MLP && m->needs_reordering) {
            if (s->mask == (AV_CH_LAYOUT_QUAD|AV_CH_LOW_FREQUENCY) ||
                s->mask == AV_CH_LAYOUT_5POINT0_BACK) {
                int i = s->ch_assign[4];
                s->ch_assign[4] = s->ch_assign[3];
                s->ch_assign[3] = s->ch_assign[2];
                s->ch_assign[2] = i;
            } else if (s->mask == AV_CH_LAYOUT_5POINT1_BACK) {
                FFSWAP(int, s->ch_assign[2], s->ch_assign[4]);
                FFSWAP(int, s->ch_assign[3], s->ch_assign[5]);
            }
        }

    }

    return 0;
}