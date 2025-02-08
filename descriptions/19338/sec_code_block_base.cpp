case Tlv::kNetworkData:
            SuccessOrExit(error = AppendNetworkData(*message, !aChild->IsFullNetworkData()));
            SuccessOrExit(error = AppendActiveTimestamp(*message));
            SuccessOrExit(error = AppendPendingTimestamp(*message));
            break;

        case Tlv::kResponse:
            SuccessOrExit(error = AppendResponse(*message, aChallenge.GetChallenge(), aChallenge.GetChallengeLength()));
            break;

        case Tlv::kSourceAddress:
            SuccessOrExit(error = AppendSourceAddress(*message));
            break;