void Leader::HandleCommissioningSet(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo)
{
    uint16_t offset = aMessage.GetOffset();
    // <MASK>

exit:

    if (GetNetif().GetMle().GetRole() == OT_DEVICE_ROLE_LEADER)
    {
        SendCommissioningSetResponse(aHeader, aMessageInfo, state);
    }

    return;
}