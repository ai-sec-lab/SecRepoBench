if (record.isNull())
				return resRec;

			resRec.assign(record.getRecordBasePtr() + record.getTotalSize());
			if (resRec.getTotalSize() == 0)
				resRec.assign(NULL);

			// resRec pointer is out-bounds of the TLV records memory
			if ((resRec.getRecordBasePtr() - tlvBufferPtr) < 0)
				resRec.assign(NULL);

			// resRec pointer is out-bounds of the TLV records memory
			if (!resRec.isNull() && resRec.getRecordBasePtr() + resRec.getTotalSize() > tlvBufferPtr + tlvDataLen)
				resRec.assign(NULL);

			return resRec;