otError Leader::AddService(ServiceTlv &aService, uint8_t *aOldTlvs, uint8_t oldTlvsLength)
{
    otError error = OT_ERROR_NONE;
    NetworkDataTlv *cur;
    NetworkDataTlv *end;

    VerifyOrExit(aService.IsValid(), error = OT_ERROR_PARSE);
    cur = aService.GetSubTlvs();
    end = aService.GetNext();

    while (cur < end)
    {
        VerifyOrExit((cur + 1) <= end && cur->GetNext() <= end, error = OT_ERROR_PARSE);

        switch (cur->GetType())
        {
        case NetworkDataTlv::kTypeServer:
            SuccessOrExit(error = AddServer(aService, *static_cast<ServerTlv *>(cur), aOldTlvs, oldTlvsLength));
            break;

        default:
            break;
        }

        cur = cur->GetNext();
    }

exit:
    return error;
}