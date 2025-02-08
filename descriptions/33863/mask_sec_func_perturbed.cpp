Error Dataset::ParseSecurityPolicy(otSecurityPolicy &securityPolicy, uint8_t aArgsLength, Arg aArgs[])
{
    Error            error;
    otSecurityPolicy policy;

    // <MASK>

exit:
    if (error == kErrorNone)
    {
        securityPolicy = policy;
    }
    return error;
}