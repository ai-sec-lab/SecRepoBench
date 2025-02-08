void IPv4Layer::parseNextLayer()
{
	size_t hdrLen = getHeaderLen();
	if (m_DataLen <= hdrLen || hdrLen == 0)
		return;

	iphdr* ipHdr = getIPv4Header();

	ProtocolType greVer = UnknownProtocol;
	ProtocolType igmpVer = UnknownProtocol;
	bool igmpQuery = false;

	uint8_t ipVersion = 0;

	uint8_t* payload = m_Data + hdrLen;
	size_t payloadLen = m_DataLen - hdrLen;

	// If it's a fragment don't parse upper layers, unless if it's the first fragment
	// TODO: assuming first fragment contains at least L4 header, what if it's not true?
	if (isFragment())
	{
		m_NextLayer = new PayloadLayer(payload, payloadLen, this, m_Packet);
		return;
	}

	switch (ipHdr->protocol)
	{
	case PACKETPP_IPPROTO_UDP:
		if (payloadLen >= sizeof(udphdr))
			m_NextLayer = new UdpLayer(payload, payloadLen, this, m_Packet);
		break;
	case PACKETPP_IPPROTO_TCP:
		m_NextLayer = TcpLayer::isDataValid(payload, payloadLen)
			? static_cast<Layer*>(new TcpLayer(payload, payloadLen, this, m_Packet))
			: static_cast<Layer*>(new PayloadLayer(payload, payloadLen, this, m_Packet));
		break;
	case PACKETPP_IPPROTO_ICMP:
		m_NextLayer = IcmpLayer::isDataValid(payload, payloadLen)
			? static_cast<Layer*>(new IcmpLayer(payload, payloadLen, this, m_Packet))
			: static_cast<Layer*>(new PayloadLayer(payload, payloadLen, this, m_Packet));
		break;
	case PACKETPP_IPPROTO_IPIP:
		ipVersion = *payload >> 4;
		if (ipVersion == 4 && IPv4Layer::isDataValid(payload, payloadLen))
			m_NextLayer = new IPv4Layer(payload, payloadLen, this, m_Packet);
		else if (ipVersion == 6 && IPv6Layer::isDataValid(payload, payloadLen))
			m_NextLayer = new IPv6Layer(payload, payloadLen, this, m_Packet);
		else
			m_NextLayer = new PayloadLayer(payload, payloadLen, this, m_Packet);
		break;
	case PACKETPP_IPPROTO_GRE:
		greVer = GreLayer::getGREVersion(payload, payloadLen);
		if (greVer == GREv0)
			m_NextLayer = new GREv0Layer(payload, payloadLen, this, m_Packet);
		else if (greVer == GREv1)
			m_NextLayer = new GREv1Layer(payload, payloadLen, this, m_Packet);
		else
			m_NextLayer = new PayloadLayer(payload, payloadLen, this, m_Packet);
		break;
	case PACKETPP_IPPROTO_IGMP:
		// <MASK>
	case PACKETPP_IPPROTO_AH:
		m_NextLayer = AuthenticationHeaderLayer::isDataValid(payload, payloadLen)
			? static_cast<Layer*>(new AuthenticationHeaderLayer(payload, payloadLen, this, m_Packet))
			: static_cast<Layer*>(new PayloadLayer(payload, payloadLen, this, m_Packet));
		break;
	case PACKETPP_IPPROTO_ESP:
		m_NextLayer = ESPLayer::isDataValid(payload, payloadLen)
			? static_cast<Layer*>(new ESPLayer(payload, payloadLen, this, m_Packet))
			: static_cast<Layer*>(new PayloadLayer(payload, payloadLen, this, m_Packet));
		break;
	case PACKETPP_IPPROTO_IPV6:
		m_NextLayer = IPv6Layer::isDataValid(payload, payloadLen)
			? static_cast<Layer*>(new IPv6Layer(payload, payloadLen, this, m_Packet))
			: static_cast<Layer*>(new PayloadLayer(payload, payloadLen, this, m_Packet));
		break;
	default:
		m_NextLayer = new PayloadLayer(payload, payloadLen, this, m_Packet);
	}
}