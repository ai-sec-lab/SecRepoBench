size_t length;

                    length = strlen(argv[2]);
                    VerifyOrExit(length <= sizeof(mPsk), error = OT_ERROR_INVALID_ARGS);
                    mPskLength = static_cast<uint8_t>(length);
                    memcpy(mPsk, argv[2], mPskLength);

                    length = strlen(argv[3]);
                    VerifyOrExit(length <= sizeof(mPskId), error = OT_ERROR_INVALID_ARGS);
                    mPskIdLength = static_cast<uint8_t>(length);