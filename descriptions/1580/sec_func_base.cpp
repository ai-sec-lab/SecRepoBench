void KML::unregisterLayerIfMatchingThisNode(KMLNode* poNode)
{
    for(int i=0;i<nNumLayers_;)
    {
        if( papoLayers_[i] == poNode )
        {
            if( i < nNumLayers_ - 1 )
            {
                memmove( papoLayers_ + i, papoLayers_ + i + 1,
                        (nNumLayers_ - 1 - i) * sizeof(KMLNode*) );
            }
            nNumLayers_ --;
            continue;
        }
        i++;
    }
}