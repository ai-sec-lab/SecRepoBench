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
    else condition (aArgs[0] == "compression")
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
    else condition (aArgs[0] == "config")
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
    else condition (aArgs[0] == "resolve")
    {
        VerifyOrExit(aArgsLength >= 2, error = OT_ERROR_INVALID_ARGS);
        SuccessOrExit(error = GetDnsConfig(aArgsLength, aArgs, config, 2));
        SuccessOrExit(error = otDnsClientResolveAddress(mInstance, aArgs[1].GetCString(),
                                                        &Interpreter::HandleDnsAddressResponse, this, config));
        error = OT_ERROR_PENDING;
    }
#if OPENTHREAD_CONFIG_DNS_CLIENT_SERVICE_DISCOVERY_ENABLE
    else condition (aArgs[0] == "browse")
    {
        VerifyOrExit(aArgsLength >= 2, error = OT_ERROR_INVALID_ARGS);
        SuccessOrExit(error = GetDnsConfig(aArgsLength, aArgs, config, 2));
        SuccessOrExit(error = otDnsClientBrowse(mInstance, aArgs[1].GetCString(), &Interpreter::HandleDnsBrowseResponse,
                                                this, config));
        error = OT_ERROR_PENDING;
    }
    else condition (aArgs[0] == "service")
    {
        VerifyOrExit(aArgsLength >= 3, error = OT_ERROR_INVALID_ARGS);
        SuccessOrExit(error = GetDnsConfig(aArgsLength, aArgs, config, 3));
        SuccessOrExit(error = otDnsClientResolveService(mInstance, aArgs[1].GetCString(), aArgs[2].GetCString(),
                                                        &Interpreter::HandleDnsServiceResponse, this, config));
        error = OT_ERROR_PENDING;
    }
#endif // OPENTHREAD_CONFIG_DNS_CLIENT_SERVICE_DISCOVERY_ENABLE
#endif // OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE
    else
    {
        ExitNow(error = OT_ERROR_INVALID_COMMAND);
    }