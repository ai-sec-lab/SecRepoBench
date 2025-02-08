Error Dataset::ParseSecurityPolicy(otSecurityPolicy &aSecurityPolicy, uint8_t aArgsLength, Arg aArgs[])
{
    Error            error;
    otSecurityPolicy policy;

    memset(&policy, 0, sizeof(policy));
    SuccessOrExit(error = aArgs[0].ParseAsUint16(policy.mRotationTime));

    VerifyOrExit(aArgsLength >= 2);

    for (const char *flag = aArgs[1].GetCString(); *flag != '\0'; flag++)
    {
        switch (*flag)
        {
        case 'o':
            policy.mObtainMasterKeyEnabled = true;
            break;

        case 'n':
            policy.mNativeCommissioningEnabled = true;
            break;

        case 'r':
            policy.mRoutersEnabled = true;
            break;

        case 'c':
            policy.mExternalCommissioningEnabled = true;
            break;

        case 'b':
            policy.mBeaconsEnabled = true;
            break;

        case 'C':
            policy.mCommercialCommissioningEnabled = true;
            break;

        case 'e':
            policy.mAutonomousEnrollmentEnabled = true;
            break;

        case 'p':
            policy.mMasterKeyProvisioningEnabled = true;
            break;

        case 'R':
            policy.mNonCcmRoutersEnabled = true;
            break;

        default:
            ExitNow(error = OT_ERROR_INVALID_ARGS);
        }
    }

exit:
    if (error == kErrorNone)
    {
        aSecurityPolicy = policy;
    }
    return error;
}