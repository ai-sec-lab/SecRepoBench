const bool LZW_HAS_CLEAR_CODE = !!(blockData[4] & 0x80);
        const int LZW_MAX_BITS = blockData[4] & 0x1f; // Max 13
        if( LZW_MAX_BITS > 13 )
        {
            CPLFree( blockData );
            CPLError( CE_Failure, CPLE_AppDefined,
                      "RIK decompression failed. "
                      "Invalid LZW_MAX_BITS." );
            return CE_Failure;
        }
        const int LZW_BITS_PER_PIXEL = 8;
        const int LZW_OFFSET = 5;

        const int LZW_CLEAR = 1 << LZW_BITS_PER_PIXEL;
        const int LZW_CODES = 1 << LZW_MAX_BITS;
        const int LZW_NO_SUCH_CODE = LZW_CODES + 1;

        int lastAdded = LZW_HAS_CLEAR_CODE ? LZW_CLEAR : LZW_CLEAR - 1;
        int codeBits = LZW_BITS_PER_PIXEL + 1;

        int code;
        int lastCode;
        GByte lastOutput;
        int bitsTaken = 0;

        int prefix[8192];      // only need LZW_CODES for size.
        GByte character[8192]; // only need LZW_CODES for size.

        for( int i = 0; i < LZW_CLEAR; i++ )
          character[i] = static_cast<GByte>( i );
        for( int i = 0; i < LZW_CODES; i++ )
            prefix[i] = LZW_NO_SUCH_CODE;

        filePos = LZW_OFFSET;
        GUInt32 fileAlign = LZW_OFFSET;
        int imageLine = poRDS->nBlockYSize - 1;

        GUInt32 lineBreak = poRDS->nBlockXSize;

        // 32 bit alignment
        lineBreak += 3;
        lineBreak &= 0xfffffffc;

        code = GetNextLZWCode( codeBits, blockData, filePos,
                               fileAlign, bitsTaken );

        OutputPixel( static_cast<GByte>( code ), pImage, poRDS->nBlockXSize,
                     lineBreak, imageLine, imagePos );
        lastOutput = static_cast<GByte>( code );

        while( imageLine >= 0 &&
               (imageLine || imagePos < poRDS->nBlockXSize) &&
               filePos < nBlockSize ) try
        {
            lastCode = code;
            code = GetNextLZWCode( codeBits, blockData,
                                   filePos, fileAlign, bitsTaken );
            if( VSIFEofL( poRDS->fp ) )
            {
                CPLFree( blockData );
                CPLError( CE_Failure, CPLE_AppDefined,
                          "RIK decompression failed. "
                          "Read past end of file.\n" );
                return CE_Failure;
            }

            if( LZW_HAS_CLEAR_CODE && code == LZW_CLEAR )
            {
#if RIK_CLEAR_DEBUG
                CPLDebug( "RIK",
                          "Clearing block %d\n"
                          " x=%d y=%d\n"
                          " pos=%d size=%d\n",
                          nBlockIndex,
                          imagePos, imageLine,
                          filePos, nBlockSize );
#endif

                // Clear prefix table
                for( int i = LZW_CLEAR; i < LZW_CODES; i++ )
                    prefix[i] = LZW_NO_SUCH_CODE;
                lastAdded = LZW_CLEAR;
                codeBits = LZW_BITS_PER_PIXEL + 1;

                filePos = fileAlign;
                bitsTaken = 0;

                code = GetNextLZWCode( codeBits, blockData,
                                       filePos, fileAlign, bitsTaken );

                if( code > lastAdded )
                {
                    throw "Clear Error";
                }

                OutputPixel( (GByte)code, pImage, poRDS->nBlockXSize,
                             lineBreak, imageLine, imagePos );
                lastOutput = (GByte)code;
            }
            else
            {
                // Set-up decoding

                GByte stack[8192]; // only need LZW_CODES for size.

                int stackPtr = 0;
                int decodeCode = code;

                if( code == lastAdded + 1 )
                {
                    // Handle special case
                    *stack = lastOutput;
                    stackPtr = 1;
                    decodeCode = lastCode;
                }
                else if( code > lastAdded + 1 )
                {
                    throw "Too high code";
                }

                // Decode

                int i = 0;
                while( ++i < LZW_CODES &&
                       decodeCode >= LZW_CLEAR &&
                       decodeCode < LZW_NO_SUCH_CODE )
                {
                    stack[stackPtr++] = character[decodeCode];
                    decodeCode = prefix[decodeCode];
                }
                stack[stackPtr++] = static_cast<GByte>( decodeCode );

                if( i == LZW_CODES || decodeCode >= LZW_NO_SUCH_CODE )
                {
                    throw "Decode error";
                }

                // Output stack

                lastOutput = stack[stackPtr - 1];

                while( stackPtr != 0 && imagePos < pixels )
                {
                    OutputPixel( stack[--stackPtr], pImage, poRDS->nBlockXSize,
                                 lineBreak, imageLine, imagePos );
                }

                // Add code to string table

                if( lastCode != LZW_NO_SUCH_CODE &&
                    lastAdded != LZW_CODES - 1 )
                {
                    prefix[++lastAdded] = lastCode;
                    character[lastAdded] = lastOutput;
                }

                // Check if we need to use more bits

                if( lastAdded == (1 << codeBits) - 1 &&
                    codeBits != LZW_MAX_BITS )
                {
                     codeBits++;

                     filePos = fileAlign;
                     bitsTaken = 0;
                }
            }
        }
        catch (const char *errStr)
        {
#if RIK_ALLOW_BLOCK_ERRORS
                CPLDebug( "RIK",
                          "LZW Decompress Failed: %s\n"
                          " blocks: %d\n"
                          " blockindex: %d\n"
                          " blockoffset: %X\n"
                          " blocksize: %d\n",
                          errStr, blocks, nBlockIndex,
                          nBlockOffset, nBlockSize );
                break;
#else
                CPLFree( blockData );
                CPLError( CE_Failure, CPLE_AppDefined,
                          "RIK decompression failed. "
                          "Corrupt image block." );
                return CE_Failure;
#endif
        }