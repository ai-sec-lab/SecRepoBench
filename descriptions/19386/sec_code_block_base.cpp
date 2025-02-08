(strlen(aPassPhrase) >= OT_COMMISSIONING_PASSPHRASE_MIN_SIZE) &&
                     (strlen(aPassPhrase) <= OT_COMMISSIONING_PASSPHRASE_MAX_SIZE) &&
                     (strlen(aNetworkName) <= OT_NETWORK_NAME_MAX_SIZE),
                 error = OT_ERROR_INVALID_ARGS)