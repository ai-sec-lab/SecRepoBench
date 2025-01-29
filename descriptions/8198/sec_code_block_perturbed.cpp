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