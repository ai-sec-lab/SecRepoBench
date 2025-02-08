filesize = filesize > 0 ? filesize : DEFAULT_MANIFEST_SIZE;

    if (filesize > MAX_BPRINT_READ_SIZE) {
        av_log(s, AV_LOG_ERROR, "Manifest too large: %"PRId64"\n", filesize);
        return AVERROR_INVALIDDATA;
    }

    av_bprint_init(&buf, filesize + 1, AV_BPRINT_SIZE_UNLIMITED);

    if ((ret = avio_read_to_bprint(in, &buf, MAX_BPRINT_READ_SIZE)) < 0 ||
        !avio_feof(in) ||
        (filesize = buf.len) == 0) {
        av_log(s, AV_LOG_ERROR, "Unable to read to manifest '%s'\n", url);
        if (ret == 0)
            ret = AVERROR_INVALIDDATA;
    } else {
        LIBXML_TEST_VERSION

        doc = xmlReadMemory(buf.str, filesize, c->base_url, NULL, 0);
        root_element = xmlDocGetRootElement(doc);
        node = root_element;

        if (!node) {
            ret = AVERROR_INVALIDDATA;
            av_log(s, AV_LOG_ERROR, "Unable to parse '%s' - missing root node\n", url);
            goto cleanup;
        }

        if (node->type != XML_ELEMENT_NODE ||
            av_strcasecmp(node->name, (const char *)"MPD")) {
            ret = AVERROR_INVALIDDATA;
            av_log(s, AV_LOG_ERROR, "Unable to parse '%s' - wrong root node name[%s] type[%d]\n", url, node->name, (int)node->type);
            goto cleanup;
        }

        val = xmlGetProp(node, "type");
        if (!val) {
            av_log(s, AV_LOG_ERROR, "Unable to parse '%s' - missing type attrib\n", url);
            ret = AVERROR_INVALIDDATA;
            goto cleanup;
        }
        if (!av_strcasecmp(val, (const char *)"dynamic"))
            c->is_live = 1;
        xmlFree(val);

        attr = node->properties;
        while (attr) {
            val = xmlGetProp(node, attr->name);

            if (!av_strcasecmp(attr->name, (const char *)"availabilityStartTime")) {
                c->availability_start_time = get_utc_date_time_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->availability_start_time = [%"PRId64"]\n", c->availability_start_time);
            } else if (!av_strcasecmp(attr->name, (const char *)"availabilityEndTime")) {
                c->availability_end_time = get_utc_date_time_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->availability_end_time = [%"PRId64"]\n", c->availability_end_time);
            } else if (!av_strcasecmp(attr->name, (const char *)"publishTime")) {
                c->publish_time = get_utc_date_time_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->publish_time = [%"PRId64"]\n", c->publish_time);
            } else if (!av_strcasecmp(attr->name, (const char *)"minimumUpdatePeriod")) {
                c->minimum_update_period = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->minimum_update_period = [%"PRId64"]\n", c->minimum_update_period);
            } else if (!av_strcasecmp(attr->name, (const char *)"timeShiftBufferDepth")) {
                c->time_shift_buffer_depth = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->time_shift_buffer_depth = [%"PRId64"]\n", c->time_shift_buffer_depth);
            } else if (!av_strcasecmp(attr->name, (const char *)"minBufferTime")) {
                c->min_buffer_time = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->min_buffer_time = [%"PRId64"]\n", c->min_buffer_time);
            } else if (!av_strcasecmp(attr->name, (const char *)"suggestedPresentationDelay")) {
                c->suggested_presentation_delay = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->suggested_presentation_delay = [%"PRId64"]\n", c->suggested_presentation_delay);
            } else if (!av_strcasecmp(attr->name, (const char *)"mediaPresentationDuration")) {
                c->media_presentation_duration = get_duration_insec(s, (const char *)val);
                av_log(s, AV_LOG_TRACE, "c->media_presentation_duration = [%"PRId64"]\n", c->media_presentation_duration);
            }
            attr = attr->next;
            xmlFree(val);
        }

        tmp_node = find_child_node_by_name(node, "BaseURL");
        if (tmp_node) {
            mpd_baseurl_node = xmlCopyNode(tmp_node,1);
        } else {
            mpd_baseurl_node = xmlNewNode(NULL, "BaseURL");
        }

        // at now we can handle only one period, with the longest duration
        node = xmlFirstElementChild(node);
        while (node) {
            if (!av_strcasecmp(node->name, (const char *)"Period")) {
                period_duration_sec = 0;
                period_start_sec = 0;
                attr = node->properties;
                while (attr) {
                    val = xmlGetProp(node, attr->name);
                    if (!av_strcasecmp(attr->name, (const char *)"duration")) {
                        period_duration_sec = get_duration_insec(s, (const char *)val);
                    } else if (!av_strcasecmp(attr->name, (const char *)"start")) {
                        period_start_sec = get_duration_insec(s, (const char *)val);
                    }
                    attr = attr->next;
                    xmlFree(val);
                }
                if ((period_duration_sec) >= (c->period_duration)) {
                    period_node = node;
                    c->period_duration = period_duration_sec;
                    c->period_start = period_start_sec;
                    if (c->period_start > 0)
                        c->media_presentation_duration = c->period_duration;
                }
            } else if (!av_strcasecmp(node->name, "ProgramInformation")) {
                parse_programinformation(s, node);
            }
            node = xmlNextElementSibling(node);
        }
        if (!period_node) {
            av_log(s, AV_LOG_ERROR, "Unable to parse '%s' - missing Period node\n", url);
            ret = AVERROR_INVALIDDATA;
            goto cleanup;
        }

        adaptionset_node = xmlFirstElementChild(period_node);
        while (adaptionset_node) {
            if (!av_strcasecmp(adaptionset_node->name, (const char *)"BaseURL")) {
                period_baseurl_node = adaptionset_node;
            } else if (!av_strcasecmp(adaptionset_node->name, (const char *)"SegmentTemplate")) {
                period_segmenttemplate_node = adaptionset_node;
            } else if (!av_strcasecmp(adaptionset_node->name, (const char *)"SegmentList")) {
                period_segmentlist_node = adaptionset_node;
            } else if (!av_strcasecmp(adaptionset_node->name, (const char *)"AdaptationSet")) {
                parse_manifest_adaptationset(s, url, adaptionset_node, mpd_baseurl_node, period_baseurl_node, period_segmenttemplate_node, period_segmentlist_node);
            }
            adaptionset_node = xmlNextElementSibling(adaptionset_node);
        }
cleanup:
        /*free the document */
        xmlFreeDoc(doc);
        xmlCleanupParser();
        xmlFreeNode(mpd_baseurl_node);
    }