switch (getFamily())
	{
	case PCPP_BSD_AF_INET:
		m_NextLayer = IPv4Layer::isDataValid(payload, payloadLength)
			? static_cast<Layer*>(new IPv4Layer(payload, payloadLength, this, m_Packet))
			: static_cast<Layer*>(new PayloadLayer(payload, payloadLength, this, m_Packet));
		break;
	case PCPP_BSD_AF_INET6_BSD:
	case PCPP_BSD_AF_INET6_FREEBSD:
	case PCPP_BSD_AF_INET6_DARWIN:
		m_NextLayer = IPv6Layer::isDataValid(payload, payloadLength)
			? static_cast<Layer*>(new IPv6Layer(payload, payloadLength, this, m_Packet))
			: static_cast<Layer*>(new PayloadLayer(payload, payloadLength, this, m_Packet));
		break;
	default:
		m_NextLayer = new PayloadLayer(payload, payloadLength, this, m_Packet);
	}