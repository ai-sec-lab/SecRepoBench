long            enterpriseNumber = 0;

        VerifyOrExit(argc > 3, error = OT_ERROR_INVALID_ARGS);

        SuccessOrExit(error = ParseLong(argv[1], enterpriseNumber));

        cfg.mServiceDataLength = static_cast<uint8_t>(strlen(argv[2]));
        memcpy(cfg.mServiceData, argv[2], cfg.mServiceDataLength);
        cfg.mEnterpriseNumber               = static_cast<uint32_t>(enterpriseNumber);
        cfg.mServerConfig.mStable           = true;
        cfg.mServerConfig.mServerDataLength = static_cast<uint8_t>(strlen(argv[3]));
        memcpy(cfg.mServerConfig.mServerData, argv[3], cfg.mServerConfig.mServerDataLength);
