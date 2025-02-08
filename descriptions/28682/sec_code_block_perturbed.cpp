VerifyOrExit(!cur->IsExtended() && (cur + 1) <= end && cur->GetNext() <= end && Tlv::IsValid(*cur),
                     isValidResult = false);