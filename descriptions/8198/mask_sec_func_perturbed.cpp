void GDALRegister_RDA()

{
    if( GDALGetDriverByName( "RDA" ) != nullptr )
        return;

    GDALDriver *poDriver = new GDALDriver();

    poDriver->SetDescription( "RDA" );
    poDriver->SetMetadataItem( GDAL_DCAP_RASTER, "YES" );
    // <MASK>

    poDriver->pfnIdentify = GDALRDADataset::Identify;
    poDriver->pfnOpen = GDALRDADataset::OpenStatic;
    poDriver->pfnUnloadDriver = GDALRDADriverUnload;

    GetGDALDriverManager()->RegisterDriver( poDriver );
}