/* Copyright (C) 2001-2023 Artifex Software, Inc.
   All Rights Reserved.

   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied,
   modified or distributed except as expressly authorized under the terms
   of the license contained in the file LICENSE in this distribution.

   Refer to licensing information at http://www.artifex.com or contact
   Artifex Software, Inc.,  39 Mesa Street, Suite 108A, San Francisco,
   CA 94129, USA, for further information.
*/


/* Common code for ImageType 1 and 4 initialization */
#include "gx.h"
#include "math_.h"
#include "memory_.h"
#include "gpcheck.h"
#include "gscdefs.h"            /* for image class table */
#include "gserrors.h"
#include "gsstruct.h"
#include "gsutil.h"
#include "gxfixed.h"
#include "gxfrac.h"
#include "gxarith.h"
#include "gxmatrix.h"
#include "gsccolor.h"
#include "gspaint.h"
#include "gzstate.h"
#include "gxdevice.h"
#include "gzpath.h"
#include "gzcpath.h"
#include "gxdevmem.h"
#include "gximage.h"
#include "gxiparam.h"
#include "gdevmrop.h"
#include "gscspace.h"
#include "gscindex.h"
#include "gsicc_cache.h"
#include "gsicc_cms.h"
#include "gsicc_manage.h"
#include "gxdevsop.h"

/* Structure descriptors */
private_st_gx_image_enum();

/* Image class procedures */
extern_gx_image_class_table();

/* Enumerator procedures */
static const gx_image_enum_procs_t image1_enum_procs = {
    gx_image1_plane_data, gx_image1_end_image, gx_image1_flush
};

/* GC procedures */
gs_private_st_ptrs2(st_color_cache, gx_image_color_cache_t, "gx_image_color_cache",
                    color_cache_enum_ptrs, color_cache_reloc_ptrs,
                    is_transparent, device_contone);
static
ENUM_PTRS_WITH(image_enum_enum_ptrs, gx_image_enum *eptr)
{
    int bps;
    gs_ptr_type_t ret;

    /* Enumerate the used members of clues.dev_color. */
    index -= gx_image_enum_num_ptrs;
    bps = eptr->unpack_bps;
    if (eptr->spp != 1)
        bps = 8;
    else if (bps > 8 || eptr->unpack == sample_unpack_copy)
        bps = 1;
    if (index >= (1 << bps) * st_device_color_max_ptrs)         /* done */
        return 0;
    /* the clues may have been cleared by gx_image_free_enum, but not freed in that */
    /* function due to being at a different save level. Only trace if dev_color.type != 0. */
    if (eptr->spp == 1) {
        if (eptr->clues != NULL) {
            if (eptr->clues[(index/st_device_color_max_ptrs) *
                (255 / ((1 << bps) - 1))].dev_color.type != 0) {
                ret = ENUM_USING(st_device_color,
                                 &eptr->clues[(index / st_device_color_max_ptrs) *
                                 (255 / ((1 << bps) - 1))].dev_color,
                                 sizeof(eptr->clues[0].dev_color),
                                 index % st_device_color_max_ptrs);
            } else {
                ret = 0;
            }
        } else {
            ret = 0;
        }
    } else {
        ret = 0;
    }
    if (ret == 0)               /* don't stop early */
        ENUM_RETURN(0);
    return ret;
}

#define e1(i,elt) ENUM_PTR(i,gx_image_enum,elt);
gx_image_enum_do_ptrs(e1)
#undef e1
ENUM_PTRS_END

static RELOC_PTRS_WITH(image_enum_reloc_ptrs, gx_image_enum *eptr)
{
    int i;

#define r1(i,elt) RELOC_PTR(gx_image_enum,elt);
    gx_image_enum_do_ptrs(r1)
#undef r1
    {
        int bps = eptr->unpack_bps;

        if (eptr->spp != 1)
            bps = 8;
        else if (bps > 8 || eptr->unpack == sample_unpack_copy)
            bps = 1;
        if (eptr->spp == 1) {
        for (i = 0; i <= 255; i += 255 / ((1 << bps) - 1))
            RELOC_USING(st_device_color,
                        &eptr->clues[i].dev_color, sizeof(gx_device_color));
    }
}
}
RELOC_PTRS_END

/* Forward declarations */
static int color_draws_b_w(gx_device * dev,
                            const gx_drawing_color * pdcolor);
static int image_init_colors(gx_image_enum * penum, int bps, int spp,
                               gs_image_format_t format,
                               const float *decode,
                               const gs_gstate * pgs, gx_device * dev,
                               const gs_color_space * pcs, bool * pdcb);

/* Procedures for unpacking the input data into bytes or fracs. */
/*extern SAMPLE_UNPACK_PROC(sample_unpack_copy); *//* declared above */

/*
 * Do common initialization for processing an ImageType 1 or 4 image.
 * Allocate the enumerator and fill in the following members:
 *      rect
 */
int
gx_image_enum_alloc(const gs_image_common_t * pic,
                    const gs_int_rect * prect, gs_memory_t * mem,
                    gx_image_enum **ppenum)
{
    const gs_pixel_image_t *pim = (const gs_pixel_image_t *)pic;
    int width = pim->Width, height = pim->Height;
    int bpc = pim->BitsPerComponent;
    gx_image_enum *penum;

    if (width < 0 || height < 0)
        return_error(gs_error_rangecheck);
    switch (pim->format) {
    case gs_image_format_chunky:
    case gs_image_format_component_planar:
        switch (bpc) {
        case 1: case 2: case 4: case 8: case 12: case 16: break;
        default: return_error(gs_error_rangecheck);
        }
        break;
    case gs_image_format_bit_planar:
        if (bpc < 1 || bpc > 8)
            return_error(gs_error_rangecheck);
    }
    if (prect) {
        if (prect->p.x < 0 || prect->p.y < 0 ||
            prect->q.x < prect->p.x || prect->q.y < prect->p.y ||
            prect->q.x > width || prect->q.y > height
            )
            return_error(gs_error_rangecheck);
    }
    *ppenum = NULL;		/* in case alloc fails and caller doesn't check code */
    penum = gs_alloc_struct(mem, gx_image_enum, &st_gx_image_enum,
                            "gx_default_begin_image");
    if (penum == 0)
        return_error(gs_error_VMerror);
    memset(penum, 0, sizeof(gx_image_enum));	/* in case of failure, no dangling pointers */
    if (prect) {
        penum->rect.x = prect->p.x;
        penum->rect.y = prect->p.y;
        penum->rect.w = prect->q.x - prect->p.x;
        penum->rect.h = prect->q.y - prect->p.y;
    } else {
        penum->rect.x = 0, penum->rect.y = 0;
        penum->rect.w = width, penum->rect.h = height;
    }
    penum->rrect.x = penum->rect.x;
    penum->rrect.y = penum->rect.y;
    penum->rrect.w = penum->rect.w;
    penum->rrect.h = penum->rect.h;
    penum->drect.x = penum->rect.x;
    penum->drect.y = penum->rect.y;
    penum->drect.w = penum->rect.w;
    penum->drect.h = penum->rect.h;
#ifdef DEBUG
    if (gs_debug_c('b')) {
        dmlprintf2(mem, "[b]Image: w=%d h=%d", width, height);
        if (prect)
            dmprintf4(mem, " ((%d,%d),(%d,%d))",
                     prect->p.x, prect->p.y, prect->q.x, prect->q.y);
    }
#endif
    *ppenum = penum;
    return 0;
}

/* Convert and restrict to a valid range. */
static inline fixed float2fixed_rounded_boxed(double src) {
    float v = floor(src*fixed_scale + 0.5);

    if (v <= min_fixed)
        return min_fixed;
    else if (v >= max_fixed)
        return max_fixed;
    else
        return 	(fixed)v;
}

/* Compute the image matrix combining the ImageMatrix with either the pmat or the pgs ctm */
int
gx_image_compute_mat(const gs_gstate *pgs, const gs_matrix *pmat, const gs_matrix *ImageMatrix,
                     gs_matrix_double *rmat)
{
    int code = 0;

    if (pmat == 0)
        pmat = &ctm_only(pgs);
    if (ImageMatrix->xx == pmat->xx && ImageMatrix->xy == pmat->xy &&
        ImageMatrix->yx == pmat->yx && ImageMatrix->yy == pmat->yy) {
        /* Process common special case separately to accept singular matrix. */
        rmat->xx = rmat->yy = 1.;
        rmat->xy = rmat->yx = 0.;
        rmat->tx = pmat->tx - ImageMatrix->tx;
        rmat->ty = pmat->ty - ImageMatrix->ty;
    } else {
        if ((code = gs_matrix_invert_to_double(ImageMatrix, rmat)) < 0 ||
            (code = gs_matrix_multiply_double(rmat, pmat, rmat)) < 0
            ) {
            return code;
        }
    }
    return code;
}

/*
 * Finish initialization for processing an ImageType 1 or 4 image.
 * Assumes the following members of *penum are set in addition to those
 * set by gx_image_enum_alloc:
 *      alpha, use_mask_color, mask_color (if use_mask_color is true),
 *      masked, adjust
 */
int
gx_image_enum_begin(gx_device * dev, const gs_gstate * pgs,
                    const gs_matrix *pmat, const gs_image_common_t * pic,
                const gx_drawing_color * pdcolor, const gx_clip_path * pcpath,
                gs_memory_t * mem, gx_image_enum *penum)
{
    const gs_pixel_image_t *pim = (const gs_pixel_image_t *)pic;
    gs_image_format_t format = pim->format;
    const int width = pim->Width;
    const int height = pim->Height;
    const int bps = pim->BitsPerComponent;
    bool masked = penum->masked;
    const float *decode = pim->Decode;
    gs_matrix_double mat;
    int index_bps;
    gs_color_space *pcs = pim->ColorSpace;
    gs_logical_operation_t lop = (pgs ? pgs->log_op : lop_default);
    int code;
    int log2_xbytes = (bps <= 8 ? 0 : arch_log2_sizeof_frac);
    int spp, nplanes, spread;
    uint bsize;
    byte *buffer = NULL;
    fixed mtx, mty;
    gs_fixed_point row_extent, col_extent, x_extent, yrange;
    bool device_color = true;
    gs_fixed_rect obox, cbox;
    bool gridfitimages = 0;
    bool in_pattern_accumulator;
    bool in_smask;
    int orthogonal;
    int force_interpolation = 0;

    // <MASK>
    penum->pcs = NULL;
    gs_free_object(mem, penum, "gx_begin_image1");
    return code;
}

/* If a drawing color is black or white, return 0 or 1 respectively, */
/* otherwise return -1. */
static int
color_draws_b_w(gx_device * dev, const gx_drawing_color * pdcolor)
{
    if (color_is_pure(pdcolor)) {
        gx_color_value rgb[3];

        (*dev_proc(dev, map_color_rgb)) (dev, gx_dc_pure_color(pdcolor),
                                         rgb);
        if (!(rgb[0] | rgb[1] | rgb[2]))
            return 0;
        if ((rgb[0] & rgb[1] & rgb[2]) == gx_max_color_value)
            return 1;
    }
    return -1;
}


static void
image_cache_decode(gx_image_enum *penum, byte input, byte *output, bool scale)
{
    float temp;

    switch ( penum->map[0].decoding ) {
        case sd_none:
            *output = input;
            break;
        case sd_lookup:
            temp = penum->map[0].decode_lookup[input >> 4]*255.0f;
            if (temp > 255) temp = 255;
            if (temp < 0 ) temp = 0;
            *output = (unsigned char) temp;
            break;
        case sd_compute:
            temp = penum->map[0].decode_base +
                (float) input * penum->map[0].decode_factor;
            if (scale) {
                temp = temp * 255.0;
            }
            if (temp > 255) temp = 255;
            if (temp < 0 ) temp = 0;
            *output = (unsigned char) temp;
            break;
        default:
            *output = 0;
            break;
    }
}

static bool
decode_range_needed(gx_image_enum *penum)
{
    bool scale = true;

    if (penum->map[0].decoding == sd_compute) {
        if (!(gs_color_space_is_ICC(penum->pcs) ||
            gs_color_space_is_PSCIE(penum->pcs))) {
            scale = false;
        }
    }
    return scale;
}

/* A special case where we go ahead and initialize the whole index cache with
   contone.  Device colors.  If we are halftoning we will then go ahead and
   apply the thresholds to the device contone values.  Only used for gray,
   rgb or cmyk source colors (No DeviceN for now) */
/* TO DO  Add in PSCIE decoder */
int
image_init_color_cache(gx_image_enum * penum, int bps, int spp)
{
    int num_des_comp = penum->dev->color_info.num_components;
    int num_src_comp;
    int num_entries = 1 << bps;
    bool need_decode = penum->icc_setup.need_decode;
    bool has_transfer = penum->icc_setup.has_transfer;
    byte value;
    bool decode_scale = true;
    int k, kk;
    byte psrc[4];
    byte *temp_buffer;
    byte *byte_ptr;
    bool is_indexed = (gs_color_space_get_index(penum->pcs) ==
                                            gs_color_space_index_Indexed);
    bool free_temp_buffer = true;
    gsicc_bufferdesc_t input_buff_desc;
    gsicc_bufferdesc_t output_buff_desc;
    gx_color_value conc[GX_DEVICE_COLOR_MAX_COMPONENTS];
    int code;

    if (penum->icc_link == NULL) {
        return gs_rethrow(-1, "ICC Link not created during image render color");
    }
    if (is_indexed) {
        num_src_comp = gs_color_space_num_components(penum->pcs->base_space);
    } else {
        /* Detect case where cache is not needed.  Colors are already in the
           device space.  Need to fast track this one and halftone row directly.
           Detected in gximono.c by looking if penum->color_cache is NULL */
        if (penum->icc_link->is_identity && !need_decode && !has_transfer) {
            return 0;
        }
        num_src_comp = 1;
    }
    /* Allocate cache of device contone values */
    penum->color_cache = gs_alloc_struct(penum->memory, gx_image_color_cache_t,
                                         &st_color_cache,
                                         "image_init_color_cache");
    penum->color_cache->device_contone = (byte*) gs_alloc_bytes(penum->memory,
                   num_des_comp * num_entries * sizeof(byte), "image_init_color_cache");
    penum->color_cache->is_transparent = (bool*) gs_alloc_bytes(penum->memory,
             num_entries * sizeof(bool), "image_init_color_cache");
    /* Initialize */
    memset(penum->color_cache->is_transparent,0,num_entries * sizeof(bool));
    /* Depending upon if we need decode and ICC CM, fill the cache a couple
       different ways. If the link is the identity, then we don't need to do any
       color conversions except for potentially a decode.  This is written in
       the manner shown below so that the common case of no decode and indexed
       image with a look-up-table uses the table data directly or does as many
       operations with memcpy as we can */
    /* Need to check the decode output range so we know how we need to scale.
       We want 8 bit output */
    if (need_decode) {
        decode_scale = decode_range_needed(penum);
    }
    if (penum->icc_link->is_identity) {
        /* No CM needed.  */
        if (need_decode || has_transfer) {
            /* Slower case.  This could be sped up later to avoid the tests
               within the loop by use of specialized loops.  */
            for (k = 0; k < num_entries; k++) {
                /* Data is in k */
                if (need_decode) {
                    image_cache_decode(penum, k, &value, decode_scale);
                } else {
                    value = k;
                }
                /* Data is in value */
                if (is_indexed) {
                    gs_cspace_indexed_lookup_bytes(penum->pcs, value, psrc);
                } else {
                    psrc[0] = value;
                }
                /* Data is in psrc */
                /* These silly transforms need to go away. ToDo. */
                if (has_transfer) {
                    for (kk = 0; kk < num_des_comp; kk++) {
                        conc[kk] = gx_color_value_from_byte(psrc[kk]);
                    }
                    cmap_transfer(&(conc[0]), penum->pgs, penum->dev);
                    for (kk = 0; kk < num_des_comp; kk++) {
                        psrc[kk] = gx_color_value_to_byte(conc[kk]);
                    }
                }
                memcpy(&(penum->color_cache->device_contone[k * num_des_comp]),
                               psrc, num_des_comp);
            }
        } else {
            /* Indexing only.  No CM, decode or transfer functions. */
            for (k = 0; k < num_entries; k++) {
                gs_cspace_indexed_lookup_bytes(penum->pcs, (float)k, psrc);
                memcpy(&(penum->color_cache->device_contone[k * num_des_comp]),
                           psrc, num_des_comp);
            }
        }
    } else {
        /* Need CM */
        /* We need to worry about if the source is indexed and if we need
           to decode first.  Then we can apply CM. Create a temp buffer in
           the source space and then transform it with one call */
        temp_buffer = (byte*) gs_alloc_bytes(penum->memory,
                                             (size_t)num_entries * num_src_comp,
                                             "image_init_color_cache");
        if (need_decode) {
            if (is_indexed) {
                /* Decode and lookup in index */
                for (k = 0; k < num_entries; k++) {
                    image_cache_decode(penum, k, &value, decode_scale);
                    gs_cspace_indexed_lookup_bytes(penum->pcs, value, psrc);
                    memcpy(&(temp_buffer[k * num_src_comp]), psrc, num_src_comp);
                }
            } else {
                /* Decode only */
                for (k = 0; k < num_entries; k++) {
                    image_cache_decode(penum, k, &(temp_buffer[k]), decode_scale);
                }
            }
        } else {
            /* No Decode */
            if (is_indexed) {
                /* If index uses a num_entries sized table then just use its pointer */
                if (penum->pcs->params.indexed.use_proc ||
                    penum->pcs->params.indexed.hival < (num_entries - 1)) {
                    /* Have to do the slow way */
                    for (k = 0; k <= penum->pcs->params.indexed.hival; k++) {
                        gs_cspace_indexed_lookup_bytes(penum->pcs, (float)k, psrc);
                        memcpy(&(temp_buffer[k * num_src_comp]), psrc, num_src_comp);
                    }
                    /* just use psrc results from converting 'hival' to fill the remaining slots */
                    for (; k < num_entries; k++) {
                        memcpy(&(temp_buffer[k * num_src_comp]), psrc, num_src_comp);
                    }
                } else {
                    /* Use the index table directly. */
                    gs_free_object(penum->memory, temp_buffer, "image_init_color_cache");
                    free_temp_buffer = false;
                    temp_buffer = (byte *)(penum->pcs->params.indexed.lookup.table.data);
                }
            } else {
                /* CM only */
                for (k = 0; k < num_entries; k++) {
                    temp_buffer[k] = k;
                }
            }
        }
        /* Set up the buffer descriptors. */
        gsicc_init_buffer(&input_buff_desc, num_src_comp, 1, false, false, false,
                          0, num_entries * num_src_comp, 1, num_entries);
        gsicc_init_buffer(&output_buff_desc, num_des_comp, 1, false, false, false,
                          0, num_entries * num_des_comp,
                      1, num_entries);
        code = (penum->icc_link->procs.map_buffer)(penum->dev, penum->icc_link,
                                            &input_buff_desc, &output_buff_desc,
                                            (void*) temp_buffer,
                                            (void*) penum->color_cache->device_contone);
        if (code < 0)
            return gs_rethrow(code, "Failure to map color buffer");

        /* Check if we need to apply any transfer functions.  If so then do it now */
        if (has_transfer) {
            for (k = 0; k < num_entries; k++) {
                byte_ptr =
                    &(penum->color_cache->device_contone[k * num_des_comp]);
                for (kk = 0; kk < num_des_comp; kk++) {
                    conc[kk] = gx_color_value_from_byte(byte_ptr[kk]);
                }
                cmap_transfer(&(conc[0]), penum->pgs, penum->dev);
                for (kk = 0; kk < num_des_comp; kk++) {
                    byte_ptr[kk] = gx_color_value_to_byte(conc[kk]);
                }
            }
        }
        if (free_temp_buffer)
            gs_free_object(penum->memory, temp_buffer, "image_init_color_cache");
    }
    return 0;
}

/* Export this for use by image_render_ functions */
void
image_init_clues(gx_image_enum * penum, int bps, int spp)
{
    /* Initialize the color table */
#define ictype(i)\
  penum->clues[i].dev_color.type

    switch ((spp == 1 ? bps : 8)) {
        case 8:         /* includes all color images */
            {
                register gx_image_clue *pcht = &penum->clues[0];
                register int n = 64;    /* 8 bits means 256 clues, do   */
                                        /* 4 at a time for efficiency   */
                do {
                    pcht[0].dev_color.type =
                        pcht[1].dev_color.type =
                        pcht[2].dev_color.type =
                        pcht[3].dev_color.type =
                        gx_dc_type_none;
                    pcht[0].key = pcht[1].key =
                        pcht[2].key = pcht[3].key = 0;
                    pcht += 4;
                }
                while (--n > 0);
                penum->clues[0].key = 1;        /* guarantee no hit */
                break;
            }
        case 4:
            ictype(17) = ictype(2 * 17) = ictype(3 * 17) =
                ictype(4 * 17) = ictype(6 * 17) = ictype(7 * 17) =
                ictype(8 * 17) = ictype(9 * 17) = ictype(11 * 17) =
                ictype(12 * 17) = ictype(13 * 17) = ictype(14 * 17) =
                gx_dc_type_none;
            /* falls through */
        case 2:
            ictype(5 * 17) = ictype(10 * 17) = gx_dc_type_none;
#undef ictype
    }
}

/* Initialize the color mapping tables for a non-mask image. */
static int
image_init_colors(gx_image_enum * penum, int bps, int spp,
                  gs_image_format_t format, const float *decode /*[spp*2] */ ,
                  const gs_gstate * pgs, gx_device * dev,
                  const gs_color_space * pcs, bool * pdcb)
{
    int ci, decode_type, code;
    static const float default_decode[] = {
        0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0
    };

    /* Clues are only used with image_mono_render */
    if (spp == 1) {
        image_init_clues(penum, bps, spp);
    }
    decode_type = 3; /* 0=custom, 1=identity, 2=inverted, 3=impossible */
    for (ci = 0; ci < spp; ci +=2 ) {
        decode_type &= (decode[ci] == 0. && decode[ci + 1] == 1.) |
                       (decode[ci] == 1. && decode[ci + 1] == 0.) << 1;
    }

    /* Initialize the maps from samples to intensities. */
    for (ci = 0; ci < spp; ci++) {
        sample_map *pmap = &penum->map[ci];

        /* If the decoding is [0 1] or [1 0], we can fold it */
        /* into the expansion of the sample values; */
        /* otherwise, we have to use the floating point method. */

        const float *this_decode = &decode[ci * 2];
        const float *map_decode;        /* decoding used to */
                                        /* construct the expansion map */
        const float *real_decode;       /* decoding for expanded samples */

        map_decode = real_decode = this_decode;
        if (!(decode_type & 1)) {
            if ((decode_type & 2) && bps <= 8) {
                real_decode = default_decode;
            } else {
                *pdcb = false;
                map_decode = default_decode;
            }
        }
        if (bps > 2 || format != gs_image_format_chunky) {
            if (bps <= 8)
                image_init_map(&pmap->table.lookup8[0], 1 << bps,
                               map_decode);
        } else {                /* The map index encompasses more than one pixel. */
            byte map[4];
            register int i;

            image_init_map(&map[0], 1 << bps, map_decode);
            switch (bps) {
                case 1:
                    {
                        register bits32 *p = &pmap->table.lookup4x1to32[0];

                        if (map[0] == 0 && map[1] == 0xff)
                            memcpy((byte *) p, lookup4x1to32_identity, 16 * 4);
                        else if (map[0] == 0xff && map[1] == 0)
                            memcpy((byte *) p, lookup4x1to32_inverted, 16 * 4);
                        else
                            for (i = 0; i < 16; i++, p++)
                                ((byte *) p)[0] = map[i >> 3],
                                    ((byte *) p)[1] = map[(i >> 2) & 1],
                                    ((byte *) p)[2] = map[(i >> 1) & 1],
                                    ((byte *) p)[3] = map[i & 1];
                    }
                    break;
                case 2:
                    {
                        register bits16 *p = &pmap->table.lookup2x2to16[0];

                        for (i = 0; i < 16; i++, p++)
                            ((byte *) p)[0] = map[i >> 2],
                                ((byte *) p)[1] = map[i & 3];
                    }
                    break;
            }
        }
        pmap->decode_base /* = decode_lookup[0] */  = real_decode[0];
        pmap->decode_factor =
            (real_decode[1] - real_decode[0]) /
            (bps <= 8 ? 255.0 : (float)frac_1);
        pmap->decode_max /* = decode_lookup[15] */  = real_decode[1];
        if (decode_type) {
            pmap->decoding = sd_none;
            pmap->inverted = map_decode[0] != 0;
        } else if (bps <= 4) {
            int step = 15 / ((1 << bps) - 1);
            int i;

            pmap->decoding = sd_lookup;
            for (i = 15 - step; i > 0; i -= step)
                pmap->decode_lookup[i] = pmap->decode_base +
                    i * (255.0 / 15) * pmap->decode_factor;
            pmap->inverted = 0;
        } else {
            pmap->decoding = sd_compute;
            pmap->inverted = 0;
        }
        if (spp == 1) {         /* and ci == 0 *//* Pre-map entries 0 and 255. */
            gs_client_color cc;

            /* Image clues are used in this case */
            cc.paint.values[0] = real_decode[0];
            code = (*pcs->type->remap_color) (&cc, pcs, penum->icolor0,
                                       pgs, dev, gs_color_select_source);
            if (code < 0)
                return code;
            cc.paint.values[0] = real_decode[1];
            code = (*pcs->type->remap_color) (&cc, pcs, penum->icolor1,
                                       pgs, dev, gs_color_select_source);
            if (code < 0)
                return code;
        }
    }
    return 0;
}
/* Construct a mapping table for sample values. */
/* map_size is 2, 4, 16, or 256.  Note that 255 % (map_size - 1) == 0, */
/* so the division 0xffffL / (map_size - 1) is always exact. */
void
image_init_map(byte * map, int map_size, const float *decode)
{
    float min_v = decode[0];
    float diff_v = decode[1] - min_v;

    if (diff_v == 1 || diff_v == -1) {  /* We can do the stepping with integers, without overflow. */
        byte *limit = map + map_size;
        uint value = (uint)(min_v * 0xffffL);
        int diff = (int)(diff_v * (0xffffL / (map_size - 1)));

        for (; map != limit; map++, value += diff)
            *map = value >> 8;
    } else {                    /* Step in floating point, with clamping. */
        int i;

        for (i = 0; i < map_size; ++i) {
            int value = (int)((min_v + diff_v * i / (map_size - 1)) * 255);

            map[i] = (value < 0 ? 0 : value > 255 ? 255 : value);
        }
    }
}

/*
 * Scale a pair of mask_color values to match the scaling of each sample to
 * a full byte, and complement and swap them if the map incorporates
 * a Decode = [1 0] inversion.
 */
void
gx_image_scale_mask_colors(gx_image_enum *penum, int component_index)
{
    uint scale = 255 / ((1 << penum->bps) - 1);
    uint *values = &penum->mask_color.values[component_index * 2];
    uint v0 = values[0] *= scale;
    uint v1 = values[1] *= scale;

    if (penum->map[component_index].decoding == sd_none &&
        penum->map[component_index].inverted
        ) {
        values[0] = 255 - v1;
        values[1] = 255 - v0;
    }
}

/* Used to indicate for ICC procesing if we have decoding to do */
bool
gx_has_transfer(const gs_gstate *pgs, int num_comps)
{
    int k;

    for (k = 0; k < num_comps; k++) {
        if (pgs->effective_transfer[k]->proc != gs_identity_transfer) {
            return(true);
        }
    }
    return(false);
}
