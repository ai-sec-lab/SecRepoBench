if (getContentLength() > 0)
	{
		m_NextLayer = new SdpLayer(m_Data + headerLen, m_DataLen - headerLen, this, m_Packet);
	}
	else
	{
		m_NextLayer = new PayloadLayer(m_Data + headerLen, m_DataLen - headerLen, this, m_Packet);
	}