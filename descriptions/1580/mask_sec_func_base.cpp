void KML::unregisterLayerIfMatchingThisNode(KMLNode* poNode)
{
    for(int i=0;i<nNumLayers_;)
    {
        if( papoLayers_[i] == poNode )
        {
            // <MASK>
        }
        i++;
    }
}