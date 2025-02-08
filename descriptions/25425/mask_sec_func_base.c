static int parse_manifest(AVFormatContext *s, const char *url, AVIOContext *in)
{
    DASHContext *c = s->priv_data;
    int ret = 0;
    int close_in = 0;
    uint8_t *new_url = NULL;
    int64_t filesize = 0;
    AVBPrint buf;
    AVDictionary *opts = NULL;
    xmlDoc *doc = NULL;
    xmlNodePtr root_element = NULL;
    xmlNodePtr node = NULL;
    xmlNodePtr period_node = NULL;
    xmlNodePtr tmp_node = NULL;
    xmlNodePtr mpd_baseurl_node = NULL;
    xmlNodePtr period_baseurl_node = NULL;
    xmlNodePtr period_segmenttemplate_node = NULL;
    xmlNodePtr period_segmentlist_node = NULL;
    xmlNodePtr adaptionset_node = NULL;
    xmlAttrPtr attr = NULL;
    char *val  = NULL;
    uint32_t period_duration_sec = 0;
    uint32_t period_start_sec = 0;

    if (!in) {
        close_in = 1;

        av_dict_copy(&opts, c->avio_opts, 0);
        ret = avio_open2(&in, url, AVIO_FLAG_READ, c->interrupt_callback, &opts);
        av_dict_free(&opts);
        if (ret < 0)
            return ret;
    }

    if (av_opt_get(in, "location", AV_OPT_SEARCH_CHILDREN, &new_url) >= 0) {
        c->base_url = av_strdup(new_url);
    } else {
        c->base_url = av_strdup(url);
    }

    filesize = avio_size(in);
    // <MASK>

    av_free(new_url);
    av_bprint_finalize(&buf, NULL);
    if (close_in) {
        avio_close(in);
    }
    return ret;
}