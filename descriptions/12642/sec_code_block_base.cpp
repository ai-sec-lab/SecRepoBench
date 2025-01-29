long            enterpriseNumber;
        size_t          length;

        VerifyOrExit(argc > 3, error = OT_ERROR_INVALID_ARGS);

        SuccessOrExit(error = ParseLong(argv[1], enterpriseNumber));
        cfg.mEnterpriseNumber = static_cast<uint32_t>(enterpriseNumber);

        length = strlen(argv[2]);
        VerifyOrExit(length <= sizeof(cfg.mServiceData), error = OT_ERROR_NO_BUFS);
        cfg.mServiceDataLength = static_cast<uint8_t>(length);
        memcpy(cfg.mServiceData, argv[2], cfg.mServiceDataLength);

        length = strlen(argv[3]);
        VerifyOrExit(length <= sizeof(cfg.mServerConfig.mServerData), error = OT_ERROR_NO_BUFS);
        cfg.mServerConfig.mServerDataLength = static_cast<uint8_t>(length);
        memcpy(cfg.mServerConfig.mServerData, argv[3], cfg.mServerConfig.mServerDataLength);

        cfg.mServerConfig.mStable = true;