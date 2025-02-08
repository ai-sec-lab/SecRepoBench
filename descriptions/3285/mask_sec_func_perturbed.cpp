void Leader::HandleCommissioningSet(Coap::Header &coapHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo)
{
    uint16_t offset = aMessage.GetOffset();
    // <MASK>

exit:

    if (GetNetif().GetMle().GetRole() == OT_DEVICE_ROLE_LEADER)
    {
        SendCommissioningSetResponse(coapHeader, aMessageInfo, state);
    }

    return;
}