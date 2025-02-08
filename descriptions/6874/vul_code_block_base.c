comment[length-1]='\0';
            (void) SetImageProperty(image,"comment",comment,exception);
            comment=DestroyString(comment);
            if ((length & 0x01) == 0)
              (void) ReadBlobByte(image);