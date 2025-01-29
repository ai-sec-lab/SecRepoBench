igmpVer = IgmpLayer::getIGMPVerFromData(
			payload, std::min<size_t>(payloadLen, be16toh(getIPv4Header()->totalLength) - headerLength), igmpQuery);
		if (igmpVer == IGMPv1)
			m_NextLayer = new IgmpV1Layer(payload, payloadLen, this, m_Packet);
		else if (igmpVer == IGMPv2)
			m_NextLayer = new IgmpV2Layer(payload, payloadLen, this, m_Packet);
		else if (igmpVer == IGMPv3)
		{
			if (igmpQuery)
				m_NextLayer = new IgmpV3QueryLayer(payload, payloadLen, this, m_Packet);
			else
				m_NextLayer = new IgmpV3ReportLayer(payload, payloadLen, this, m_Packet);
		}
		else
			m_NextLayer = new PayloadLayer(payload, payloadLen, this, m_Packet);
		break;