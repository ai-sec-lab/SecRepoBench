/******************************************************************************
 *
 * Project:  GRC/GRD Reader
 * Purpose:  Northwood Format basic implementation
 * Author:   Perry Casson
 *
 ******************************************************************************
 * Copyright (c) 2007, Waypoint Information Technology
 * Copyright (c) 2009-2011, Even Rouault <even dot rouault at mines-paris dot org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "gdal_pam.h"

#include "northwood.h"

#include <algorithm>
#include <string>

CPL_CVSID("$Id$");

int nwt_ParseHeader( NWT_GRID * pGrd, char *nwtHeader )
{
    /* double dfTmp; */

    if( nwtHeader[4] == '1' )
        pGrd->cFormat = 0x00;        // grd - surface type
    else if( nwtHeader[4] == '8' )
        pGrd->cFormat = 0x80;        //  grc classified type

    pGrd->stClassDict = NULL;

    memcpy( reinterpret_cast<void *>( &pGrd->fVersion ),
            reinterpret_cast<void *>( &nwtHeader[5] ),
            sizeof( pGrd->fVersion ) );
    CPL_LSBPTR32(&pGrd->fVersion);

    unsigned short usTmp;
    memcpy( reinterpret_cast<void *>( &usTmp ),
            reinterpret_cast<void *>( &nwtHeader[9] ),
            2 );
    CPL_LSBPTR16(&usTmp);
    pGrd->nXSide = static_cast<unsigned int>( usTmp );
    if( pGrd->nXSide == 0 )
    {
        memcpy( reinterpret_cast<void *>( &pGrd->nXSide ),
                reinterpret_cast<void *>( &nwtHeader[128] ),
                sizeof(pGrd->nXSide) );
        CPL_LSBPTR32(&pGrd->nXSide);
    }
    // <MASK>

    return TRUE;
}

// Create a color gradient ranging from ZMin to Zmax using the color
// inflections defined in grid
int nwt_LoadColors( NWT_RGB * pMap, int mapSize, NWT_GRID * pGrd )
{
    int i;
    NWT_RGB sColor;
    int nWarkerMark = 0;

    createIP( 0, 255, 255, 255, pMap, &nWarkerMark );
    if( pGrd->iNumColorInflections == 0 )
        return 0;

    // If Zmin is less than the 1st inflection use the 1st inflections color to
    // the start of the ramp
    if( pGrd->fZMin <= pGrd->stInflection[0].zVal )
    {
        createIP( 1, pGrd->stInflection[0].r,
                     pGrd->stInflection[0].g,
                     pGrd->stInflection[0].b, pMap, &nWarkerMark );
    }
    // find what inflections zmin is between
    for( i = 1; i < pGrd->iNumColorInflections; i++ )
    {
        if( pGrd->fZMin < pGrd->stInflection[i].zVal )
        {
            // then we must be between i and i-1
            linearColor( &sColor, &pGrd->stInflection[i - 1],
                                  &pGrd->stInflection[i],
                                  pGrd->fZMin );
            createIP( 1, sColor.r, sColor.g, sColor.b, pMap, &nWarkerMark );
            break;
        }
    }
    // the interesting case of zmin beig higher than the max inflection value
    if( i >= pGrd->iNumColorInflections )
    {
        createIP( 1,
                  pGrd->stInflection[pGrd->iNumColorInflections - 1].r,
                  pGrd->stInflection[pGrd->iNumColorInflections - 1].g,
                  pGrd->stInflection[pGrd->iNumColorInflections - 1].b,
                  pMap, &nWarkerMark );
        createIP( mapSize - 1,
                  pGrd->stInflection[pGrd->iNumColorInflections - 1].r,
                  pGrd->stInflection[pGrd->iNumColorInflections - 1].g,
                  pGrd->stInflection[pGrd->iNumColorInflections - 1].b,
                  pMap, &nWarkerMark );
    }
    else
    {
        int index = 0;
        for( ; i < pGrd->iNumColorInflections; i++ )
        {
            if( pGrd->fZMax < pGrd->stInflection[i].zVal )
            {
                // then we must be between i and i-1
                linearColor( &sColor, &pGrd->stInflection[i - 1],
                                      &pGrd->stInflection[i], pGrd->fZMax );
                index = mapSize - 1;
                createIP( index, sColor.r, sColor.g, sColor.b, pMap,
                           &nWarkerMark );
                break;
            }
            // save the inflections between zmin and zmax
            index = static_cast<int>(
                ( (pGrd->stInflection[i].zVal - pGrd->fZMin) /
                  (pGrd->fZMax - pGrd->fZMin) )
                * mapSize );

            if ( index >= mapSize )
                index = mapSize - 1;
            createIP( index,
                      pGrd->stInflection[i].r,
                      pGrd->stInflection[i].g,
                      pGrd->stInflection[i].b,
                      pMap, &nWarkerMark );
        }
        if( index < mapSize - 1 )
            createIP( mapSize - 1,
                      pGrd->stInflection[pGrd->iNumColorInflections - 1].r,
                      pGrd->stInflection[pGrd->iNumColorInflections - 1].g,
                      pGrd->stInflection[pGrd->iNumColorInflections - 1].b,
                      pMap, &nWarkerMark );
    }
    return 0;
}

//solve for a color between pIPLow and pIPHigh
void linearColor( NWT_RGB * pRGB, NWT_INFLECTION * pIPLow, NWT_INFLECTION * pIPHigh,
                      float fMid )
{
    if( fMid < pIPLow->zVal )
    {
        pRGB->r = pIPLow->r;
        pRGB->g = pIPLow->g;
        pRGB->b = pIPLow->b;
    }
    else if( fMid > pIPHigh->zVal )
    {
        pRGB->r = pIPHigh->r;
        pRGB->g = pIPHigh->g;
        pRGB->b = pIPHigh->b;
    }
    else
    {
        float scale = (fMid - pIPLow->zVal) / (pIPHigh->zVal - pIPLow->zVal);
        pRGB->r = static_cast<unsigned char>
                (scale * (pIPHigh->r - pIPLow->r) + pIPLow->r + 0.5);
        pRGB->g = static_cast<unsigned char>
                (scale * (pIPHigh->g - pIPLow->g) + pIPLow->g + 0.5);
        pRGB->b = static_cast<unsigned char>
                (scale * (pIPHigh->b - pIPLow->b) + pIPLow->b + 0.5);
    }
}

// insert IP's into the map filling as we go
void createIP( int index, unsigned char r, unsigned char g, unsigned char b,
               NWT_RGB * map, int *pnWarkerMark )
{
    int i;

    if( index == 0 )
    {
        map[0].r = r;
        map[0].g = g;
        map[0].b = b;
        *pnWarkerMark = 0;
        return;
    }

    if( index <= *pnWarkerMark )
        return;

    int wm = *pnWarkerMark;

    float rslope = static_cast<float>(r - map[wm].r) / static_cast<float>(index - wm);
    float gslope = static_cast<float>(g - map[wm].g) / static_cast<float>(index - wm);
    float bslope = static_cast<float>(b - map[wm].b) / static_cast<float>(index - wm);
    for( i = wm + 1; i < index; i++)
    {
        map[i].r = static_cast<unsigned char>(map[wm].r + ((i - wm) * rslope) + 0.5);
        map[i].g = static_cast<unsigned char>(map[wm].g + ((i - wm) * gslope) + 0.5);
        map[i].b = static_cast<unsigned char>(map[wm].b + ((i - wm) * bslope) + 0.5);
    }
    map[index].r = r;
    map[index].g = g;
    map[index].b = b;
    *pnWarkerMark = index;
    return;
}

void nwt_HillShade( unsigned char *r, unsigned char *g, unsigned char *b,
                    char *h )
{
    HLS hls;
    NWT_RGB rgb;
    rgb.r = *r;
    rgb.g = *g;
    rgb.b = *b;
    hls = RGBtoHLS( rgb );
    hls.l += ((short) *h) * HLSMAX / 256;
    rgb = HLStoRGB( hls );

    *r = rgb.r;
    *g = rgb.g;
    *b = rgb.b;
    return;
}

NWT_GRID *nwtOpenGrid( char *filename )
{
    char nwtHeader[1024];
    VSILFILE *fp = VSIFOpenL( filename, "rb" );

    if( fp == NULL )
    {
        CPLError(CE_Failure, CPLE_OpenFailed, "Can't open %s", filename );
        return NULL;
    }

    if( !VSIFReadL( nwtHeader, 1024, 1, fp ) )
        return NULL;

    if( nwtHeader[0] != 'H' ||
        nwtHeader[1] != 'G' ||
        nwtHeader[2] != 'P' ||
        nwtHeader[3] != 'C' )
          return NULL;

    NWT_GRID *pGrd = reinterpret_cast<NWT_GRID *>(
        calloc( sizeof(NWT_GRID), 1 ) );

    if( nwtHeader[4] == '1' )
        pGrd->cFormat = 0x00;        // grd - surface type
    else if( nwtHeader[4] == '8' )
        pGrd->cFormat = 0x80;        //  grc classified type
    else
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "Unhandled Northwood format type = %0xd",
                 nwtHeader[4] );
        if( pGrd )
            free( pGrd );
        return NULL;
    }

    strncpy( pGrd->szFileName, filename, sizeof(pGrd->szFileName) );
    pGrd->szFileName[sizeof(pGrd->szFileName) - 1] = '\0';
    pGrd->fp = fp;
    nwt_ParseHeader( pGrd, nwtHeader );

    return pGrd;
}

//close the file and free the mem
void nwtCloseGrid( NWT_GRID * pGrd )
{
    if( (pGrd->cFormat & 0x80) && pGrd->stClassDict )        // if is GRC - free the Dictionary
    {
        for( unsigned short usTmp = 0; usTmp < pGrd->stClassDict->nNumClassifiedItems; usTmp++ )
        {
            free( pGrd->stClassDict->stClassifedItem[usTmp] );
        }
        free( pGrd->stClassDict->stClassifedItem );
        free( pGrd->stClassDict );
    }
    if( pGrd->fp )
        VSIFCloseL( pGrd->fp );
    free( pGrd );
        return;
}

void nwtPrintGridHeader( NWT_GRID * pGrd )
{
    if( pGrd->cFormat & 0x80 )
    {
        printf( "\n%s\n\nGrid type is Classified ", pGrd->szFileName );/*ok*/
        if( pGrd->cFormat == 0x81 )
            printf( "4 bit (Less than 16 Classes)" );/*ok*/
        else if( pGrd->cFormat == 0x82 )
            printf( "8 bit (Less than 256 Classes)" );/*ok*/
        else if( pGrd->cFormat == 0x84 )
            printf( "16 bit (Less than 65536 Classes)" );/*ok*/
        else
        {
            printf( "GRC - Unhandled Format or Type %d", pGrd->cFormat );/*ok*/
            return;
        }
    }
    else
    {
        printf( "\n%s\n\nGrid type is Numeric ", pGrd->szFileName );/*ok*/
        if( pGrd->cFormat == 0x00 )
            printf( "16 bit (Standard Precision)" );/*ok*/
        else if( pGrd->cFormat == 0x01 )
            printf( "32 bit (High Precision)" );/*ok*/
        else
        {
            printf( "GRD - Unhandled Format or Type %d", pGrd->cFormat );/*ok*/
            return;
        }
    }
    printf( "\nDim (x,y) = (%u,%u)", pGrd->nXSide, pGrd->nYSide );/*ok*/
    printf( "\nStep Size = %f", pGrd->dfStepSize );/*ok*/
    printf( "\nBounds = (%f,%f) (%f,%f)", pGrd->dfMinX, pGrd->dfMinY,/*ok*/
            pGrd->dfMaxX, pGrd->dfMaxY );
    printf( "\nCoordinate System = %s", pGrd->cMICoordSys );/*ok*/

    if( !(pGrd->cFormat & 0x80) )    // print the numeric specific stuff
    {
        printf( "\nMin Z = %f Max Z = %f Z Units = %d \"%s\"", pGrd->fZMin,/*ok*/
                pGrd->fZMax, pGrd->iZUnits, pGrd->cZUnits );

        printf( "\n\nDisplay Mode =" );/*ok*/
        if( pGrd->bShowGradient )
            printf( " Color Gradient" );/*ok*/

        if( pGrd->bShowGradient && pGrd->bShowHillShade )
            printf( " and" );/*ok*/

        if( pGrd->bShowHillShade )
            printf( " Hill Shading" );/*ok*/

        for( int i = 0; i < pGrd->iNumColorInflections; i++ )
        {
            printf( "\nColor Inflection %d - %f (%d,%d,%d)", i + 1,/*ok*/
                    pGrd->stInflection[i].zVal, pGrd->stInflection[i].r,
                    pGrd->stInflection[i].g, pGrd->stInflection[i].b );
        }

        if( pGrd->bHillShadeExists )
        {
            printf("\n\nHill Shade Azumith = %.1f Inclination = %.1f "/*ok*/
                   "Brightness = %d Contrast = %d",
                   pGrd->fHillShadeAzimuth, pGrd->fHillShadeAngle,
                   pGrd->cHillShadeBrightness, pGrd->cHillShadeContrast );
        }
        else
            printf( "\n\nNo Hill Shade Data" );/*ok*/
    }
    else                            // print the classified specific stuff
    {
        printf( "\nNumber of Classes defined = %u",/*ok*/
                pGrd->stClassDict->nNumClassifiedItems );
        for( int i = 0; i < static_cast<int>( pGrd->stClassDict->nNumClassifiedItems ); i++ )
        {
            printf( "\n%s - (%d,%d,%d)  Raw = %d  %d %d",/*ok*/
                    pGrd->stClassDict->stClassifedItem[i]->szClassName,
                    pGrd->stClassDict->stClassifedItem[i]->r,
                    pGrd->stClassDict->stClassifedItem[i]->g,
                    pGrd->stClassDict->stClassifedItem[i]->b,
                    pGrd->stClassDict->stClassifedItem[i]->usPixVal,
                    pGrd->stClassDict->stClassifedItem[i]->res1,
                    pGrd->stClassDict->stClassifedItem[i]->res2 );
        }
    }
}

HLS RGBtoHLS( NWT_RGB rgb )
{
    /* get R, G, and B out of DWORD */
    short R = rgb.r;
    short G = rgb.g;
    short B = rgb.b;

    /* calculate lightness */
    unsigned char cMax = static_cast<unsigned char>( std::max( std::max(R,G), B ) );
    unsigned char cMin = static_cast<unsigned char>( std::min( std::min(R,G), B ) );
    HLS hls;
    hls.l = (((cMax + cMin) * HLSMAX) + RGBMAX) / (2 * RGBMAX);

    short Rdelta, Gdelta, Bdelta;    /* intermediate value: % of spread from max */
    if( cMax == cMin )
    {                            /* r=g=b --> achromatic case */
        hls.s = 0;                /* saturation */
        hls.h = UNDEFINED;        /* hue */
    }
    else
    {                            /* chromatic case */
        /* saturation */
        if( hls.l <= (HLSMAX / 2) )
            hls.s =
              (((cMax - cMin) * HLSMAX) + ((cMax + cMin) / 2)) / (cMax + cMin);
        else
            hls.s= (((cMax - cMin) * HLSMAX) + ((2 * RGBMAX - cMax - cMin) / 2))
              / (2 * RGBMAX - cMax - cMin);

        /* hue */
        Rdelta =
            (((cMax - R) * (HLSMAX / 6)) + ((cMax - cMin) / 2)) / (cMax - cMin);
        Gdelta =
            (((cMax - G) * (HLSMAX / 6)) + ((cMax - cMin) / 2)) / (cMax - cMin);
        Bdelta =
            (((cMax - B) * (HLSMAX / 6)) + ((cMax - cMin) / 2)) / (cMax - cMin);

        if( R == cMax )
            hls.h = Bdelta - Gdelta;
        else if( G == cMax )
            hls.h = (HLSMAX / 3) + Rdelta - Bdelta;
        else                        /* B == cMax */
            hls.h = ((2 * HLSMAX) / 3) + Gdelta - Rdelta;

        if( hls.h < 0 )
            hls.h += HLSMAX;
        if( hls.h > HLSMAX )
            hls.h -= HLSMAX;
    }
    return hls;
}

/* utility routine for HLStoRGB */
static short HueToRGB( short n1, short n2, short hue )
{
    /* range check: note values passed add/subtract thirds of range */
    if( hue < 0 )
        hue += HLSMAX;

    if( hue > HLSMAX )
        hue -= HLSMAX;

    /* return r,g, or b value from this tridrant */
    if( hue < (HLSMAX / 6) )
        return n1 + (((n2 - n1) * hue + (HLSMAX / 12)) / (HLSMAX / 6));
    if( hue < (HLSMAX / 2) )
        return n2;
    if( hue < ((HLSMAX * 2) / 3) )
        return
            n1 +
            (((n2 - n1) * (((HLSMAX * 2) / 3) - hue) +
              (HLSMAX / 12)) / (HLSMAX / 6));
    else
        return n1;
}

NWT_RGB HLStoRGB( HLS hls )
{
    NWT_RGB rgb;

    if( hls.s == 0 )
    {                            /* achromatic case */
        rgb.r = static_cast<unsigned char>( (hls.l * RGBMAX) / HLSMAX );
        rgb.g = rgb.r;
        rgb.b = rgb.r;
        if( hls.h != UNDEFINED )
        {
            /* ERROR */
        }
    }
    else
    {                            /* chromatic case */
        /* set up magic numbers */
        short Magic1, Magic2;            /* calculated magic numbers (really!) */
        if( hls.l <= (HLSMAX / 2) )
            Magic2 = (hls.l * (HLSMAX + hls.s) + (HLSMAX / 2)) / HLSMAX;
        else
            Magic2 = hls.l + hls.s - ((hls.l * hls.s) + (HLSMAX / 2)) / HLSMAX;
        Magic1 = 2 * hls.l - Magic2;

        /* get RGB, change units from HLSMAX to RGBMAX */
        rgb.r = static_cast<unsigned char> ((HueToRGB (Magic1, Magic2, hls.h + (HLSMAX / 3)) * RGBMAX + (HLSMAX / 2)) / HLSMAX);
        rgb.g = static_cast<unsigned char> ((HueToRGB (Magic1, Magic2, hls.h) * RGBMAX + (HLSMAX / 2)) / HLSMAX);
        rgb.b = static_cast<unsigned char> ((HueToRGB (Magic1, Magic2, hls.h - (HLSMAX / 3)) * RGBMAX + (HLSMAX / 2)) / HLSMAX);
    }

    return rgb;
}
