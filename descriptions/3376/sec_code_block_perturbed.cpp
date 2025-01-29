MeshCoP::CommissionerSessionIdTlv *tlv = static_cast<MeshCoP::CommissionerSessionIdTlv *>(cur);

            VerifyOrExit(tlv->IsValid());
            sessionId = tlv->GetCommissionerSessionId();
            sessionIdPresent = true;