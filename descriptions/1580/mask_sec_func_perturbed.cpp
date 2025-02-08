void KML::unregisterLayerIfMatchingThisNode(KMLNode* poNode)
{
    for(int index=0;index<nNumLayers_;)
    {
        if( papoLayers_[index] == poNode )
        {
            // <MASK>
        }
        index++;
    }
}