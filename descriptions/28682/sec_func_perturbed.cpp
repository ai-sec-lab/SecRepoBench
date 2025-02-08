bool Dataset::IsValid(void) const
{
    bool       isValidResult = true;
    const Tlv *end  = GetTlvsEnd();

    for (const Tlv *cur = GetTlvsStart(); cur < end; cur = cur->GetNext())
    {
        VerifyOrExit(!cur->IsExtended() && (cur + 1) <= end && cur->GetNext() <= end && Tlv::IsValid(*cur),
                     isValidResult = false);
    }

exit:
    return isValidResult;
}