otError Interpreter::ProcessDns(uint8_t aArgsLength, Arg aArgs[])
{
    OT_UNUSED_VARIABLE(aArgs);

    otError error = OT_ERROR_NONE;
#if OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE
    otDnsQueryConfig  queryConfig;
    otDnsQueryConfig *config = &queryConfig;
#endif

    if (aArgsLength == 0)
    {
        error = OT_ERROR_INVALID_ARGS;
    }
#if OPENTHREAD_CONFIG_REFERENCE_DEVICE_ENABLE
    else if (aArgs[0] == "compression")
    {
        if (aArgsLength == 1)
        {
            OutputEnabledDisabledStatus(otDnsIsNameCompressionEnabled());
        }
        else
        {
            bool enable;

            VerifyOrExit(aArgsLength == 2, error = OT_ERROR_INVALID_ARGS);
            SuccessOrExit(error = ParseEnableOrDisable(aArgs[1], enable));
            otDnsSetNameCompressionEnabled(enable);
        }
    }
#endif // OPENTHREAD_CONFIG_REFERENCE_DEVICE_ENABLE
#if OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE
    else if (aArgs[0] == "config")
    {
        if (aArgsLength == 1)
        {
            const otDnsQueryConfig *defaultConfig = otDnsClientGetDefaultConfig(mInstance);

            OutputFormat("Server: [");
            OutputIp6Address(defaultConfig->mServerSockAddr.mAddress);
            OutputLine("]:%d", defaultConfig->mServerSockAddr.mPort);
            OutputLine("ResponseTimeout: %u ms", defaultConfig->mResponseTimeout);
            OutputLine("MaxTxAttempts: %u", defaultConfig->mMaxTxAttempts);
            OutputLine("RecursionDesired: %s",
                       (defaultConfig->mRecursionFlag == OT_DNS_FLAG_RECURSION_DESIRED) ? "yes" : "no");
        }
        else
        {
            SuccessOrExit(error = GetDnsConfig(aArgsLength, aArgs, config, 1));
            otDnsClientSetDefaultConfig(mInstance, config);
        }
    }
    else if (aArgs[0] == "resolve")
    // <MASK> // OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE
    else
    {
        ExitNow(error = OT_ERROR_INVALID_COMMAND);
    }