void KML::unregisterLayerIfMatchingThisNode(KMLNode* poNode)
{
    for(int index=0;index<nNumLayers_;)
    {
        if( papoLayers_[index] == poNode )
        {
            if( index < nNumLayers_ - 1 )
            {
                memmove( papoLayers_ + index, papoLayers_ + index + 1,
                        (nNumLayers_ - 1 - index) * sizeof(KMLNode*) );
            }
            nNumLayers_ --;
            continue;
        }
        index++;
    }
}