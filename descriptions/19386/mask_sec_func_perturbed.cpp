otError Commissioner::GeneratePskc(const char *              aPassPhrase,
                                   const char *              aNetworkName,
                                   const Mac::ExtendedPanId &aExtendedPanId,
                                   Pskc &                    aPskc)
{
    otError     error      = OT_ERROR_NONE;
    const char *saltPrefix = "Thread";
    uint8_t     salt[OT_PBKDF2_SALT_MAX_LEN];
    uint16_t    saltLen = 0;

    VerifyOrExit(// <MASK>;

    memset(salt, 0, sizeof(salt));
    memcpy(salt, saltPrefix, strlen(saltPrefix));
    saltLen += static_cast<uint16_t>(strlen(saltPrefix));

    memcpy(salt + saltLen, aExtendedPanId.m8, sizeof(aExtendedPanId));
    saltLen += OT_EXT_PAN_ID_SIZE;

    memcpy(salt + saltLen, aNetworkName, strlen(aNetworkName));
    saltLen += static_cast<uint16_t>(strlen(aNetworkName));

    otPbkdf2Cmac(reinterpret_cast<const uint8_t *>(aPassPhrase), static_cast<uint16_t>(strlen(aPassPhrase)),
                 reinterpret_cast<const uint8_t *>(salt), saltLen, 16384, OT_PSKC_MAX_SIZE, aPskc.m8);

exit:
    return error;
}