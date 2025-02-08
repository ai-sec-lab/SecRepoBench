Error Dataset::ParseSecurityPolicy(otSecurityPolicy &aSecurityPolicy, uint8_t aArgsLength, Arg aArgs[])
{
    Error            error;
    otSecurityPolicy policy;

    // <MASK>

exit:
    if (error == kErrorNone)
    {
        aSecurityPolicy = policy;
    }
    return error;
}