uint8_t length = static_cast<uint8_t>(aMessage.GetLength() - aMessage.GetOffset());
    uint8_t tlvs[NetworkData::kMaxSize];
    MeshCoP::StateTlv::State state = MeshCoP::StateTlv::kAccept;
    bool hasSessionId = false;
    bool hasValidTlv = false;
    uint16_t sessionId = 0;

    VerifyOrExit(GetNetif().GetMle().GetRole() == OT_DEVICE_ROLE_LEADER, state = MeshCoP::StateTlv::kReject);

    aMessage.Read(offset, length, tlvs);

    // Session Id and Border Router Locator MUST NOT be set, but accept including unexpected or
    // unknown TLV as long as there is at least one valid TLV.
    for (MeshCoP::Tlv *cur = reinterpret_cast<MeshCoP::Tlv *>(tlvs);
         cur < reinterpret_cast<MeshCoP::Tlv *>(tlvs + length);
         cur = cur->GetNext())
    {
        MeshCoP::Tlv::Type type = cur->GetType();

        if (type == MeshCoP::Tlv::kJoinerUdpPort || type == MeshCoP::Tlv::kSteeringData)
        {
            hasValidTlv = true;
        }
        else if (type == MeshCoP::Tlv::kBorderAgentLocator)
        {
            ExitNow(state = MeshCoP::StateTlv::kReject);
        }
        else if (type == MeshCoP::Tlv::kCommissionerSessionId)
        {
            hasSessionId = true;
            sessionId = static_cast<MeshCoP::CommissionerSessionIdTlv *>(cur)->GetCommissionerSessionId();
        }
        else
        {
            // do nothing for unexpected or unknown TLV
        }
    }

    // verify whether or not commissioner session id TLV is included
    VerifyOrExit(hasSessionId, state = MeshCoP::StateTlv::kReject);

    // verify whether or not MGMT_COMM_SET.req includes at least one valid TLV
    VerifyOrExit(hasValidTlv, state = MeshCoP::StateTlv::kReject);

    // Find Commissioning Data TLV
    for (NetworkDataTlv *netDataTlv = reinterpret_cast<NetworkDataTlv *>(mTlvs);
         netDataTlv < reinterpret_cast<NetworkDataTlv *>(mTlvs + mLength);
         netDataTlv = netDataTlv->GetNext())
    {
        if (netDataTlv->GetType() == NetworkDataTlv::kTypeCommissioningData)
        {
            // Iterate over MeshCoP TLVs and extract desired data
            for (MeshCoP::Tlv *cur = reinterpret_cast<MeshCoP::Tlv *>(netDataTlv->GetValue());
                 cur < reinterpret_cast<MeshCoP::Tlv *>(netDataTlv->GetValue() + netDataTlv->GetLength());
                 cur = cur->GetNext())
            {
                if (cur->GetType() == MeshCoP::Tlv::kCommissionerSessionId)
                {
                    VerifyOrExit(sessionId ==
                                 static_cast<MeshCoP::CommissionerSessionIdTlv *>(cur)->GetCommissionerSessionId(),
                                 state = MeshCoP::StateTlv::kReject);
                }
                else if (cur->GetType() == MeshCoP::Tlv::kBorderAgentLocator)
                {
                    memcpy(tlvs + length, reinterpret_cast<uint8_t *>(cur), cur->GetLength() + sizeof(MeshCoP::Tlv));
                    length += (cur->GetLength() + sizeof(MeshCoP::Tlv));
                }
            }
        }
    }

    SetCommissioningData(tlvs, length);