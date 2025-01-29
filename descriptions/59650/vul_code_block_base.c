			// record pointer is out-bounds of the TLV records memory
			if ((record.getRecordBasePtr() - tlvDataBasePtr) < 0)
				return resRec;

			// record pointer is out-bounds of the TLV records memory
			if (record.getRecordBasePtr() - tlvDataBasePtr + (int)record.getTotalSize() >= (int)tlvDataLen)
				return resRec;

			resRec.assign(record.getRecordBasePtr() + record.getTotalSize());
			if (resRec.getTotalSize() == 0)
				resRec.assign(NULL);
