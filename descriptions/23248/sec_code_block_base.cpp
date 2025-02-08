uint32_t family = getFamily();
	if (family > IEEE_802_3_MAX_LEN)
	{
		uint16_t ethType = (uint16_t)family;
		switch (ethType)
		{
		case PCPP_ETHERTYPE_IP:
			m_NextLayer = IPv4Layer::isDataValid(payload, payloadLen)
				? static_cast<Layer*>(new IPv4Layer(payload, payloadLen, this, m_Packet))
				: static_cast<Layer*>(new PayloadLayer(payload, payloadLen, this, m_Packet));
			return;
		case PCPP_ETHERTYPE_IPV6:
			m_NextLayer = IPv6Layer::isDataValid(payload, payloadLen)
				? static_cast<Layer*>(new IPv6Layer(payload, payloadLen, this, m_Packet))
				: static_cast<Layer*>(new PayloadLayer(payload, payloadLen, this, m_Packet));
			return;
		default:
			m_NextLayer = new PayloadLayer(payload, payloadLen, this, m_Packet);
			return;
		}
	}

	switch (family)
	{
	case PCPP_BSD_AF_INET:
		m_NextLayer = IPv4Layer::isDataValid(payload, payloadLen)
			? static_cast<Layer*>(new IPv4Layer(payload, payloadLen, this, m_Packet))
			: static_cast<Layer*>(new PayloadLayer(payload, payloadLen, this, m_Packet));
		break;
	case PCPP_BSD_AF_INET6_BSD:
	case PCPP_BSD_AF_INET6_FREEBSD:
	case PCPP_BSD_AF_INET6_DARWIN:
		m_NextLayer = IPv6Layer::isDataValid(payload, payloadLen)
			? static_cast<Layer*>(new IPv6Layer(payload, payloadLen, this, m_Packet))
			: static_cast<Layer*>(new PayloadLayer(payload, payloadLen, this, m_Packet));
		break;
	default:
		m_NextLayer = new PayloadLayer(payload, payloadLen, this, m_Packet);
	}