void GDALRegister_RDA()

{
    if( GDALGetDriverByName( "RDA" ) != nullptr )
        return;

    GDALDriver *poDriver = new GDALDriver();

    poDriver->SetDescription( "RDA" );
    poDriver->SetMetadataItem( GDAL_DCAP_RASTER, "YES" );
    poDriver->SetMetadataItem( GDAL_DMD_LONGNAME,
                               "DigitalGlobe Raster Data Access driver" );
    poDriver->SetMetadataItem( GDAL_DMD_HELPTOPIC,
                               "frmt_rda.html" );
    poDriver->SetMetadataItem( GDAL_DMD_EXTENSION, "dgrda" );

    poDriver->SetMetadataItem( GDAL_DMD_OPENOPTIONLIST,
"<OpenOptionList>"
"  <Option name='MAXCONNECT' type='int' min='1' max='256' "
                        "description='Maximum number of connections'/>"
"</OpenOptionList>" );

    poDriver->pfnIdentify = GDALRDADataset::Identify;
    poDriver->pfnOpen = GDALRDADataset::OpenStatic;
    poDriver->pfnUnloadDriver = GDALRDADriverUnload;

    GetGDALDriverManager()->RegisterDriver( poDriver );
}