{
        SuccessOrExit(error = GetDnsConfig(aArgsLength, aArgs, config, 2));
        SuccessOrExit(error = otDnsClientResolveAddress(mInstance, aArgs[1].GetCString(),
                                                        &Interpreter::HandleDnsAddressResponse, this, config));
        error = OT_ERROR_PENDING;
    }
#if OPENTHREAD_CONFIG_DNS_CLIENT_SERVICE_DISCOVERY_ENABLE
    else if (aArgs[0] == "browse")
    {
        SuccessOrExit(error = GetDnsConfig(aArgsLength, aArgs, config, 2));
        SuccessOrExit(error = otDnsClientBrowse(mInstance, aArgs[1].GetCString(), &Interpreter::HandleDnsBrowseResponse,
                                                this, config));
        error = OT_ERROR_PENDING;
    }
    else if (aArgs[0] == "service")
    {
        SuccessOrExit(error = GetDnsConfig(aArgsLength, aArgs, config, 3));
        SuccessOrExit(error = otDnsClientResolveService(mInstance, aArgs[1].GetCString(), aArgs[2].GetCString(),
                                                        &Interpreter::HandleDnsServiceResponse, this, config));
        error = OT_ERROR_PENDING;
    }
#endif // OPENTHREAD_CONFIG_DNS_CLIENT_SERVICE_DISCOVERY_ENABLE
#endif