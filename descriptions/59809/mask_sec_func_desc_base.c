TLVRecordType getFirstTLVRecord(uint8_t* tlvDataBasePtr, size_t tlvDataLen) const
		{
			// Get the first TLV record out of a byte stream.
			// Return a instance of type TLVRecordType that contains the first TLV record.
			// If tlvDataBasePtr is NULL or tlv data length is zero the returned TLVRecordType 
			// instance will be logically NULL, meaning TLVRecordType.isNull() will return true.
			// <MASK>
		}