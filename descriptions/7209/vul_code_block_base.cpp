switch (codec->getScanlineOrder()) {
                    case SkCodec::kTopDown_SkScanlineOrder:
                    case SkCodec::kBottomUp_SkScanlineOrder:
                        // We do not need to check the return value.  On an incomplete
                        // image, memory will be filled with a default value.
                        codec->getScanlines(dst, height, rowBytes);
                        break;
                }