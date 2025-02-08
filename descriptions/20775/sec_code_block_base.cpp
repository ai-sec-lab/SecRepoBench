MeshCoP::Tlv::Type type;

        VerifyOrExit(((cur + 1) <= end) && !cur->IsExtended() && (cur->GetNext() <= end));

        type = cur->GetType();

        if (type == MeshCoP::Tlv::kJoinerUdpPort || type == MeshCoP::Tlv::kSteeringData)
        {
            hasValidTlv = true;
        }
        else if (type == MeshCoP::Tlv::kBorderAgentLocator)
        {
            ExitNow();
        }
        else if (type == MeshCoP::Tlv::kCommissionerSessionId)
        {
            MeshCoP::CommissionerSessionIdTlv *tlv = static_cast<MeshCoP::CommissionerSessionIdTlv *>(cur);

            VerifyOrExit(tlv->IsValid());
            sessionId    = tlv->GetCommissionerSessionId();
            hasSessionId = true;
        }
        else
        {
            // do nothing for unexpected or unknown TLV
        }

        cur = cur->GetNext();