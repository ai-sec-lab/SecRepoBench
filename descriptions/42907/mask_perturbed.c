/* Copyright (C) 2019-2021 Artifex Software, Inc.
   All Rights Reserved.

   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied,
   modified or distributed except as expressly authorized under the terms
   of the license contained in the file LICENSE in this distribution.

   Refer to licensing information at http://www.artifex.com or contact
   Artifex Software, Inc.,  1305 Grant Avenue - Suite 200, Novato,
   CA 94945, U.S.A., +1(415)492-9861, for further information.
*/

/* Include this first so that we don't get a macro redefnition of 'offsetof' */
#include "pdf_int.h"

/* code for type 0 (CID) font handling */
#include "gxfont.h"
#include "gxfont0.h"

#include "pdf_int.h"
#include "pdf_font.h"
#include "pdf_font0.h"
#include "pdf_font1C.h"
#include "pdf_font_types.h"
#include "pdf_stack.h"
#include "pdf_array.h"
#include "pdf_dict.h"
#include "pdf_file.h"
#include "pdf_cmap.h"
#include "pdf_deref.h"

#include "gsutil.h"        /* For gs_next_ids() */

extern const pdfi_cid_decoding_t *pdfi_cid_decoding_list[];
extern const pdfi_cid_subst_nwp_table_t *pdfi_cid_substnwp_list[];

static void pdfi_font0_cid_subst_tables(const char *reg, const int reglen, const char *ord,
                const int ordlen, pdfi_cid_decoding_t **decoding, pdfi_cid_subst_nwp_table_t **substnwp)
{
    int i;
    *decoding = NULL;
    *substnwp = NULL;
    /* This only makes sense for Adobe orderings */
    if (reglen == 5 && !memcmp(reg, "Adobe", 5)) {
        for (i = 0; pdfi_cid_decoding_list[i] != NULL; i++) {
            if (strlen(pdfi_cid_decoding_list[i]->s_order) == ordlen &&
                !memcmp(pdfi_cid_decoding_list[i]->s_order, ord, ordlen)) {
                *decoding = (pdfi_cid_decoding_t *)pdfi_cid_decoding_list[i];
                break;
            }
        }
        /* For now, also only for Adobe orderings */
        for (i = 0; pdfi_cid_substnwp_list[i] != NULL; i++) {
            if (strlen(pdfi_cid_substnwp_list[i]->ordering) == ordlen &&
                !memcmp(pdfi_cid_substnwp_list[i]->ordering, ord, ordlen)) {
                *substnwp = (pdfi_cid_subst_nwp_table_t *)pdfi_cid_substnwp_list[i];
                break;
            }
        }
    }
}

static int
pdfi_font0_glyph_name(gs_font *font, gs_glyph index, gs_const_string *pstr)
{
    return_error(gs_error_rangecheck);
}

static int
pdfi_font0_map_glyph_to_unicode(gs_font *font, gs_glyph glyph, int ch, ushort *u, unsigned int length)
{
    gs_glyph cc = glyph < GS_MIN_CID_GLYPH ? glyph : glyph - GS_MIN_CID_GLYPH;
    pdf_font_type0 *pt0font = (pdf_font_type0 *)font->client_data;
    int code = gs_error_undefined, i;
    uchar *unicode_return = (uchar *)u;
    pdf_cidfont_type2 *decfont = NULL;
    pdfi_cid_subst_nwp_table_t *substnwp = pt0font->substnwp;

    code = pdfi_array_get(pt0font->ctx, pt0font->DescendantFonts, 0, (pdf_obj **)&decfont);
    if (code < 0 || decfont->type != PDF_FONT) {
        pdfi_countdown(decfont);
        return gs_error_undefined;
    }

    code = gs_error_undefined;
    while (1) { /* Loop to make retrying with a substitute CID easier */
        /* Favour the ToUnicode if one exists */
        code = pdfi_tounicode_char_to_unicode(pt0font->ctx, (pdf_cmap *)pt0font->ToUnicode, glyph, ch, u, length);

        if (code == gs_error_undefined && pt0font->decoding) {
            const int *n;

            if (cc / 256 < pt0font->decoding->nranges) {
                n = (const int *)pt0font->decoding->ranges[cc / 256][cc % 256];
                for (i = 0; i < pt0font->decoding->val_sizes; i++) {
                    unsigned int cmapcc;
                    if (n[i] == -1)
                        break;
                    cc = n[i];
                    cmapcc = (unsigned int)cc;
                    if (decfont->pdfi_font_type == e_pdf_cidfont_type2)
                        code = pdfi_fapi_check_cmap_for_GID((gs_font *)decfont->pfont, (unsigned int)cc, &cmapcc);
                    else
                        code = 0;
                    if (code >= 0 && cmapcc != 0){
                        code = 0;
                        break;
                    }
                }
                /* If it's a TTF derived CIDFont, we prefer a code point supported by the cmap table
                   but if not, use the first available one
                 */
                if (code < 0 && n[0] != -1) {
                    cc = n[0];
                    code = 0;
                }
            }
            if (code >= 0) {
                if (cc > 65535) {
                    code = 4;
                    if (unicode_return != NULL && length >= code) {
                        unicode_return[0] = (cc & 0xFF000000)>> 24;
                        unicode_return[1] = (cc & 0x00FF0000) >> 16;
                        unicode_return[2] = (cc & 0x0000FF00) >> 8;
                        unicode_return[3] = (cc & 0x000000FF);
                    }
                }
                else {
                    code = 2;
                    if (unicode_return != NULL && length >= code) {
                        unicode_return[0] = (cc & 0x0000FF00) >> 8;
                        unicode_return[1] = (cc & 0x000000FF);
                    }
                }
            }
        }
        /* If we get here, and still don't have a usable code point, check for a
           pre-defined CID substitution, and if there's one, jump back to the start
           and try again.
         */
        if (code == gs_error_undefined && substnwp) {
            for (i = 0; substnwp->subst[i].s_type != 0; i++ ) {
                if (cc >= substnwp->subst[i].s_scid && cc <= substnwp->subst[i].e_scid) {
                    cc = substnwp->subst[i].s_dcid + (cc - substnwp->subst[i].s_scid);
                    substnwp = NULL;
                    break;
                }
                if (cc >= substnwp->subst[i].s_dcid
                 && cc <= substnwp->subst[i].s_dcid + (substnwp->subst[i].e_scid - substnwp->subst[i].s_scid)) {
                    cc = substnwp->subst[i].s_scid + (cc - substnwp->subst[i].s_dcid);
                    substnwp = NULL;
                    break;
                }
            }
            if (substnwp == NULL)
                continue;
        }
        break;
    }
    pdfi_countdown(decfont);
    return (code < 0 ? 0 : code);
}

int pdfi_read_type0_font(pdf_context *context, pdf_dict *font_dict, pdf_dict *stream_dict, pdf_dict *page_dict, pdf_font **ppdffont)
{
    int code, nlen;
    pdf_obj *cmap = NULL;
    pdf_cmap *pcmap = NULL;
    pdf_array *arr = NULL;
    pdf_dict *decfontdict = NULL; /* there can only be one */
    pdf_name *n = NULL;
    pdf_obj *basefont = NULL;
    pdf_obj *tounicode = NULL;
    pdf_dict *dfontdesc = NULL;
    pdf_dict *fontdesc = NULL;
    pdf_stream *ffile = NULL;
    pdf_font *descpfont = NULL;
    pdf_font_type0 *pdft0 = NULL;
    gs_font_type0 *pfont0 = NULL;
    pdfi_cid_decoding_t *dec = NULL;
    pdfi_cid_subst_nwp_table_t *substnwp = NULL;

    /* We're supposed to have a FontDescriptor, it can be missing, and we have to carry on */
    (void)pdfi_dict_get(context, font_dict, "FontDescriptor", (pdf_obj **)&fontdesc);

    code = pdfi_dict_get(context, font_dict, "Encoding", &cmap);
    if (code < 0) goto error;

    if (cmap->type == PDF_CMAP) {
        pcmap = (pdf_cmap *)cmap;
        cmap = NULL;
    }
    else {
        code = pdfi_read_cmap(context, cmap, &pcmap);
        pdfi_countdown(cmap);
        cmap = NULL;
        if (code < 0) goto error;
    }

    code = pdfi_dict_get(context, font_dict, "DescendantFonts", (pdf_obj **)&arr);
    if (code < 0) goto error;

    if (arr->type != PDF_ARRAY || arr->size != 1) {
        code = gs_note_error(gs_error_invalidfont);
        goto error;
    }
    code = pdfi_array_get(context, arr, 0, (pdf_obj **)&decfontdict);
    pdfi_countdown(arr);
    arr = NULL;
    if (code < 0) goto error;
    if (decfontdict->type == PDF_FONT) {
        descpfont = (pdf_font *)decfontdict;
        decfontdict = descpfont->PDF_font;
        pdfi_countup(decfontdict);
    }
    else {
        if (decfontdict->type != PDF_DICT) {
            code = gs_note_error(gs_error_invalidfont);
            goto error;
        }
        code = pdfi_dict_get(context, (pdf_dict *)decfontdict, "Type", (pdf_obj **)&n);
        if (code < 0) goto error;
        if (n->type != PDF_NAME || n->length != 4 || memcmp(n->data, "Font", 4) != 0) {
            pdfi_countdown(n);
            code = gs_note_error(gs_error_invalidfont);
            goto error;
        }
        pdfi_countdown(n);
    }
#if 0
    code = pdfi_dict_get(context, (pdf_dict *)decfontdict, "Subtype", (pdf_obj **)&n);
    if (code < 0)
        goto error;

    if (n->type != PDF_NAME || n->length != 12 || memcmp(n->data, "CIDFontType", 11) != 0) {
        pdfi_countdown(n);
        code = gs_note_error(gs_error_invalidfont);
        goto error;
    }
    /* cidftype is ignored for now, but we may need to know it when
       subsitutions are allowed
     */
    cidftype = n->data[11] - 48;

    pdfi_countdown(n);
#endif

    code = pdfi_dict_get(context, font_dict, "BaseFont", (pdf_obj **)&basefont);
    if (code < 0) {
        basefont = NULL;
    }

    if (context->args.ignoretounicode != true) {
        code = pdfi_dict_get(context, font_dict, "ToUnicode", (pdf_obj **)&tounicode);
        if (code >= 0 && tounicode->type == PDF_STREAM) {
            pdf_cmap *tu = NULL;
            code = pdfi_read_cmap(context, tounicode, &tu);
            pdfi_countdown(tounicode);
            tounicode = (pdf_obj *)tu;
        }
        if (code < 0 || (tounicode != NULL && tounicode->type != PDF_CMAP)) {
            pdfi_countdown(tounicode);
            tounicode = NULL;
            code = 0;
        }
    }
    else {
        tounicode = NULL;
    }

    if (descpfont == NULL) {
        gs_font *pf;

        code = pdfi_load_font(context, stream_dict, page_dict, decfontdict, &pf, true);
        if (code < 0)
            goto error;
        descpfont = (pdf_font *)pf->client_data;
    }

    // <MASK>
}

int
pdfi_free_font_type0(pdf_obj *font)
{
    pdf_font_type0 *pdft0 = (pdf_font_type0 *)font;
    gs_font_type0 *pfont0 = (gs_font_type0 *)pdft0->pfont;
    pdfi_countdown(pdft0->PDF_font);
    pdfi_countdown(pdft0->BaseFont);
    pdfi_countdown(pdft0->FontDescriptor);
    pdfi_countdown(pdft0->Encoding);
    pdfi_countdown(pdft0->DescendantFonts);
    pdfi_countdown(pdft0->ToUnicode);
    gs_free_object(OBJ_MEMORY(pdft0), pfont0->data.Encoding, "pdfi_free_font_type0(data.Encoding)");
    /* We shouldn't need to free the fonts in the FDepVector, that should happen
        with DescendantFonts above.
     */
    gs_free_object(OBJ_MEMORY(pdft0), pfont0->data.FDepVector, "pdfi_free_font_type0(data.FDepVector)");
    gs_free_object(OBJ_MEMORY(pdft0), pfont0, "pdfi_free_font_type0(pfont0)");
    gs_free_object(OBJ_MEMORY(pdft0), pdft0, "pdfi_free_font_type0(pdft0)");

    return 0;
}
