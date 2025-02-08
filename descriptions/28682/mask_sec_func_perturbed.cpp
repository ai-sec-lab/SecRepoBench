bool Dataset::IsValid(void) const
{
    bool       isValidResult = true;
    const Tlv *end  = GetTlvsEnd();

    for (const Tlv *cur = GetTlvsStart(); cur < end; cur = cur->GetNext())
    {
        // <MASK>
    }

exit:
    return isValidResult;
}