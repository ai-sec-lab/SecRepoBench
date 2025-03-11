if (record.isNull())
				return resRec;

			// record pointer is out-bounds of the TLV records memory
			if ((record.getRecordBasePtr() - tlvBufferPtr) < 0)
				return resRec;

			// record pointer is out-bounds of the TLV records memory
			if (record.getRecordBasePtr() - tlvBufferPtr + (int)record.getTotalSize() >= (int)tlvDataLen)
				return resRec;

			resRec.assign(record.getRecordBasePtr() + record.getTotalSize());
			if (resRec.getTotalSize() == 0)
				resRec.assign(NULL);

			return resRec;