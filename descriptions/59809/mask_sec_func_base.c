TLVRecordType getFirstTLVRecord(uint8_t* tlvDataBasePtr, size_t tlvDataLen) const
		{
			TLVRecordType resRec(tlvDataBasePtr); // for NRVO optimization

			// resRec pointer is out-bounds of the TLV records memory
			if (resRec.getRecordBasePtr() + resRec.getTotalSize() > tlvDataBasePtr + tlvDataLen)
				resRec.assign(NULL);

			// check if there are records at all and the total size is not zero
			if (!resRec.isNull() && (tlvDataLen == 0 || resRec.getTotalSize() == 0))
				resRec.assign(NULL);

			return resRec;
		}