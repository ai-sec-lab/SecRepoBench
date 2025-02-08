void
MultiPartInputFile::initialize()
{
    readMagicNumberAndVersionField(*_data->is, _data->version);
    
    bool multipart = isMultiPart(_data->version);
    bool tiled = isTiled(_data->version);

    //
    // Multipart files don't have and shouldn't have the tiled bit set.
    //

    if (tiled && multipart)
        throw IEX_NAMESPACE::InputExc ("Multipart files cannot have the tiled bit set");

    
    int pos = 0;
    while (true)
    {
        Header headerinfo;
        headerinfo.readFrom(*_data->is, _data->version);

        //
        // If we read nothing then we stop reading.
        //

        if (headerinfo.readsNothing())
        {
            pos++;
            break;
        }

        _data->_headers.push_back(headerinfo);
        
        if(multipart == false)
          break;
    }

    //
    // Perform usual check on headers.
    //

    // <MASK>
}