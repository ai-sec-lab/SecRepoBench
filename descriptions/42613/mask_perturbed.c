/* Copyright (C) 2020-2022 Artifex Software, Inc.
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

/* common code for Postscript-type font handling */
#include "scanchar.h"
#include "sfilter.h"
#include "stream.h"
#include "strimpl.h"

#include "pdf_int.h"
#include "pdf_types.h"
#include "pdf_array.h"
#include "pdf_dict.h"
#include "pdf_font.h"
#include "pdf_font_types.h"
#include "pdf_fontps.h"

static const char *const notdefnamestr = ".notdef";

int
pdfi_pscript_stack_init(pdf_context *pdfi_ctx, pdf_ps_oper_list_t *ops, void *client_data,
                        pdf_ps_ctx_t *s)
{
    int i, size = PDF_PS_STACK_SIZE;
    int initsizebytes = sizeof(pdf_ps_stack_object_t) * PDF_PS_STACK_SIZE;
    s->pdfi_ctx = pdfi_ctx;
    s->ops = ops;
    s->client_data = client_data;

    s->stack = (pdf_ps_stack_object_t *)gs_alloc_bytes(pdfi_ctx->memory, initsizebytes, "pdfi_pscript_stack_init(stack)");
    if (s->stack == NULL)
        return_error(gs_error_VMerror);

    s->cur = s->stack + 1;
    s->toplim = s->cur + size;

    for (i = 0; i < PDF_PS_STACK_GUARDS; i++)
        s->stack[i].type = PDF_PS_OBJ_STACK_BOTTOM;

    for (i = 0; i < PDF_PS_STACK_GUARDS; i++)
        s->stack[size - 1 + i].type = PDF_PS_OBJ_STACK_TOP;

    for (i = 0; i < size - 1; i++) {
        pdf_ps_make_null(&(s->cur[i]));
    }
    return 0;
}

void
pdfi_pscript_stack_finit(pdf_ps_ctx_t *s)
{
    int stackdepth;

    if ((stackdepth = pdf_ps_stack_count(s)) > 0) {
        pdf_ps_stack_pop(s, stackdepth);
    }
    gs_free_object(s->pdfi_ctx->memory, s->stack, "pdfi_pscript_stack_finit(stack)");
}

int
ps_pdf_null_oper_func(gs_memory_t *mem, pdf_ps_ctx_t *stack, byte *buf, byte *bufend)
{
    return 0;
}

int
clear_stack_oper_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    int depth = s->cur - &(s->stack[1]);

    return pdf_ps_stack_pop(s, depth);
}

int
pdf_ps_pop_oper_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    return pdf_ps_stack_pop(s, 1);
}

int
pdf_ps_pop_and_pushmark_func(gs_memory_t *mem, pdf_ps_ctx_t *stack, byte *buf, byte *bufend)
{
    int code = pdf_ps_stack_pop(stack, 1);

    if (code >= 0)
        code = pdf_ps_stack_push_mark(stack);
    return code;
}

static inline int
pdf_ps_is_whitespace(int c)
{
    return (c == 0x20) || (c == 0x9) || (c == 0xD) || (c == 0xA);
}

static inline int
pdf_ps_end_object(int c)
{
    return pdf_ps_is_whitespace(c) || (c == '/') || (c == '[') || (c == ']') || c == '{' || c == '}' || (c == '(') || (c == '<');
}

static inline int
pdf_ps_end_number_object(int c)
{
    return (c != '.' && c != 'e' && c != '-' && (c < '0' || c > '9'));
}

static inline bool
ishex(char c)
{
    if (c < 0x30)
        return false;

    if (c > 0x39) {
        if (c > 'F') {
            if (c < 'a')
                return false;
            if (c > 'f')
                return false;
            return true;
        }
        else {
            if (c < 'A')
                return false;
            return true;
        }
    }
    else
        return true;
}

static inline int
decodehex(char c)
{
    int retval = 0;

    if (ishex(c)) {
        if (c > 0x39) {
            if (c > 'F') {
                retval = c - 0x57;
            }
            else {
                retval = c - 0x37;
            }
        }
        else {
            retval = c - 0x30;
        }
    }
    return retval;
}

int
pdfi_pscript_interpret(pdf_ps_ctx_t *cs, byte *pdfpsbuf, int64_t buflen)
{
    int code = 0;
    byte *buflim = pdfpsbuf + buflen;
    int arraydepth = 0;
    int stackdepth;

    while (pdfpsbuf < buflim && code >= 0) {
        switch (*pdfpsbuf++) {
            case '%':          /* Comment */
                {
                    while (pdfpsbuf < buflim && *pdfpsbuf != char_EOL && *pdfpsbuf != '\f' &&
                           *pdfpsbuf != char_CR)
                        pdfpsbuf++;

                    if (*pdfpsbuf == char_EOL)
                        pdfpsbuf++;
                }
                break;
            case '/':          /* name */
                {
                    byte *n = pdfpsbuf;
                    int len;

                    while (pdfpsbuf < buflim && !pdf_ps_end_object((int)*pdfpsbuf))
                        pdfpsbuf++;
                    len = pdfpsbuf - n;
                    code = pdf_ps_stack_push_name(cs, n, len);
                } break;
            case '(':          /* string */
                {
                    byte *s = pdfpsbuf;
                    int len;
                    int depth = 1;

                    while (pdfpsbuf < buflim && depth > 0) {
                        if (*pdfpsbuf == '(') {
                            depth++;
                        }
                        else if (*pdfpsbuf == ')') {
                            depth--;
                        }
                        pdfpsbuf++;
                    }
                    len = (pdfpsbuf - s) - 1;
                    code = pdf_ps_stack_push_string(cs, s, len);
                }
                break;
            case '<':          /* hex string */
                {
                    byte *s = pdfpsbuf;
                    byte *s2 = s;
                    int len, i;
                    byte hbuf[2];

                    if (pdfpsbuf < buflim && *pdfpsbuf == '<') { /* Dict opening "<<" - we don't care */
                        pdfpsbuf++;
                        continue;
                    }
                    while (pdfpsbuf < buflim && *pdfpsbuf != '>')
                        pdfpsbuf++;
                    len = pdfpsbuf - s;
                    while (len % 2)
                        len--;
                    for (i = 0; i < len; i += 2) {
                        hbuf[0] = s[i];
                        hbuf[1] = s[i + 1];
                        *s2++ = (decodehex(hbuf[0]) << 4) | decodehex(hbuf[1]);
                    }
                    pdfpsbuf++; /* move past the trailing '>' */
                    code = pdf_ps_stack_push_string(cs, s, len >> 1);
                }
                break;
            case '>': /* For hex strings, this should be handled above */
                {
                    if (pdfpsbuf < buflim && *pdfpsbuf == '>') { /* Dict closing "<<" - we still don't care */
                        pdfpsbuf++;
                    }
                }
               break;
            case '[':;         /* begin array */
            case '{':;         /* begin executable array (mainly, FontBBox) */
                arraydepth++;
                code = pdf_ps_stack_push_arr_mark(cs);
                break;
            case ']':          /* end array */
            case '}':          /* end executable array */
                {
                    pdf_ps_stack_object_t *arr = NULL;
                    int i, size = pdf_ps_stack_count_to_mark(cs, PDF_PS_OBJ_ARR_MARK);

                    if (size > 0 && arraydepth > 0) {
                        arr = (pdf_ps_stack_object_t *) gs_alloc_bytes(cs->pdfi_ctx->memory, size * sizeof(pdf_ps_stack_object_t), "pdfi_pscript_interpret(pdf_ps_stack_object_t");
                        if (arr == NULL) {
                            code = gs_note_error(gs_error_VMerror);
                            /* clean up the stack, including the mark object */
                            (void)pdf_ps_stack_pop(cs, size + 1);
                            size = 0;
                        }
                        else {
                            for (i = 0; i < size; i++) {
                                memcpy(&(arr[(size - 1) - i]), cs->cur, sizeof(*cs->cur));
                                if (pdf_ps_obj_has_type(cs->cur, PDF_PS_OBJ_ARRAY)) {
                                    pdf_ps_make_null(cs->cur);
                                }
                                (void)pdf_ps_stack_pop(cs, 1);
                            }
                            /* And pop the array mark */
                            (void)pdf_ps_stack_pop(cs, 1);
                        }
                    }
                    else {
                        /* And pop the array mark for an emtpy array */
                        (void)pdf_ps_stack_pop(cs, 1);
                    }
                    code = pdf_ps_stack_push_array(cs, arr, size > 0 ? size : 0);
                    arraydepth--;
                    if (arraydepth < 0)
                        arraydepth = 0;
                }
                break;
            case '.':
            case '-':
            case '+':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':{
                    bool is_float = false;
                    int len;
                    byte *n = --pdfpsbuf, *numbuf;

                    while (pdfpsbuf < buflim && !pdf_ps_end_number_object((int)*pdfpsbuf)) {
                        if (*pdfpsbuf == '.' || *pdfpsbuf == 'e')
                            is_float = true;
                        pdfpsbuf++;
                    }
                    len = pdfpsbuf - n;
                    if (len == 1 && *n == '-') {
                        /* Not a number, might be an operator */
                        pdfpsbuf = n + 1;
                        goto retry_as_oper;
                    }
                    numbuf = gs_alloc_bytes(cs->pdfi_ctx->memory, len + 1, "ps pdf number buffer");
                    if (numbuf == NULL) {
                        code = gs_note_error(gs_error_VMerror);
                    }
                    else {
                        memcpy(numbuf, n, len);
                        numbuf[len] = '\0';
                        if (is_float) {
                            float f = (float)atof((const char *)numbuf);

                            code = pdf_ps_stack_push_float(cs, f);
                        }
                        else {
                            int i = atoi((const char *)numbuf);

                            code = pdf_ps_stack_push_int(cs, i);
                        }
                        gs_free_object(cs->pdfi_ctx->memory, numbuf, "ps pdf number buffer");
                    }
                } break;
            case ' ':
            case '\f':
            case '\t':
            case char_CR:
            case char_EOL:
            case char_NULL:
                break;
            default:
              retry_as_oper:{
                    byte *n = --pdfpsbuf;
                    int len, i;
                    int (*opfunc)(gs_memory_t *mem, pdf_ps_ctx_t *stack, byte *buf, byte *bufend) = NULL;
                    pdf_ps_oper_list_t *ops = cs->ops;

                    while (pdfpsbuf < buflim && !pdf_ps_end_object((int)*pdfpsbuf))
                        pdfpsbuf++;

                    if (arraydepth == 0) {
                        len = pdfpsbuf - n;
                        for (i = 0; ops[i].opname != NULL; i++) {
                            if (len == ops[i].opnamelen && !memcmp(n, ops[i].opname, len)) {
                                opfunc = ops[i].oper;
                                break;
                            }
                        }

                        if (opfunc) {
                            code = (*opfunc) (cs->pdfi_ctx->memory, cs, pdfpsbuf, buflim);
                            if (code > 0) {
                                pdfpsbuf += code;
                                code = 0;
                            }
                        }
                    }
                }
                break;
        }
    }
    if ((stackdepth = pdf_ps_stack_count(cs)) > 0) {
        pdf_ps_stack_pop(cs, stackdepth);
    }
    return code;
}

static inline bool pdf_ps_name_cmp(pdf_ps_stack_object_t *obj, const char *namestr)
{
    byte *d = NULL;
    int l1, l2;

    if (namestr) {
        l2 = strlen(namestr);
    }

    if (obj->type == PDF_PS_OBJ_NAME) {
        d = obj->val.name;
        l1 = obj->size;
    }
    else if (obj->type == PDF_PS_OBJ_STRING) {
        d = obj->val.name;
        l1 = obj->size;
    }
    if (d != NULL && namestr != NULL && l1 == l2) {
        return memcmp(d, namestr, l1) == 0 ? true : false;
    }
    return false;
}

static int
ps_font_def_func(gs_memory_t *memory, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    int code = 0, code2 = 0;
    ps_font_interp_private *priv = (ps_font_interp_private *) s->client_data;

    if ((code = pdf_ps_stack_count(s)) < 2) {
        return pdf_ps_stack_pop(s, code);
    }

    if (pdf_ps_obj_has_type(&s->cur[-1], PDF_PS_OBJ_NAME)) {
        // <MASK>
    }

    code2 = pdf_ps_stack_pop(s, 2);
    if (code < 0)
        return code;
    else
        return code2;
}

static int
ps_font_true_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    (void)mem;
    return pdf_ps_stack_push_boolean(s, true);
}

static int
ps_font_false_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    (void)mem;
    return pdf_ps_stack_push_boolean(s, false);
}

static int
ps_font_dict_begin_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    (void)mem;
    return pdf_ps_stack_push_dict_mark(s);
}

static int
ps_font_dict_end_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    int dsize = pdf_ps_stack_count_to_mark(s, PDF_PS_OBJ_DICT_MARK);

    (void)mem;
    if (dsize >= 0)
        return pdf_ps_stack_pop(s, dsize + 1);  /* Add one for the mark object */
    else
        return 0;
}

static stream *
push_eexec_filter(gs_memory_t *mem, byte *buf, byte *bufend)
{
    stream *fs, *ffs = NULL;
    stream *sstrm;
    stream_exD_state *st;
    byte *strbuf;

    sstrm = file_alloc_stream(mem, "push_eexec_filter(buf stream)");
    if (sstrm == NULL)
        return NULL;

    /* Because of streams <shrug!> we advance the buffer one byte */
    buf++;
    sread_string(sstrm, buf, bufend - buf);
    sstrm->close_at_eod = false;

    fs = s_alloc(mem, "push_eexec_filter(fs)");
    strbuf = gs_alloc_bytes(mem, 4096, "push_eexec_filter(buf)");
    st = gs_alloc_struct(mem, stream_exD_state, s_exD_template.stype, "push_eexec_filter(st)");
    if (fs == NULL || st == NULL || strbuf == NULL) {
        sclose(sstrm);
        gs_free_object(mem, sstrm, "push_eexec_filter(buf stream)");
        gs_free_object(mem, fs, "push_eexec_filter(fs)");
        gs_free_object(mem, st, "push_eexec_filter(st)");
        goto done;
    }
    memset(st, 0x00, sizeof(stream_exD_state));

    s_std_init(fs, strbuf, 69, &s_filter_read_procs, s_mode_read);
    st->memory = mem;
    st->templat = &s_exD_template;
    fs->state = (stream_state *) st;
    fs->procs.process = s_exD_template.process;
    fs->strm = sstrm;
    (*s_exD_template.set_defaults) ((stream_state *) st);
    st->cstate = 55665;
    st->binary = -1;
    st->lenIV = 4;
    st->keep_spaces = true;
    (*s_exD_template.init) ((stream_state *) st);
    fs->close_at_eod = false;
    ffs = fs;
  done:
    return ffs;
}

static void
pop_eexec_filter(gs_memory_t *mem, stream *s)
{
    stream *src = s->strm;
    byte *b = s->cbuf;

    sclose(s);
    gs_free_object(mem, s, "pop_eexec_filter(s)");
    gs_free_object(mem, b, "pop_eexec_filter(b)");
    if (src)
        sclose(src);
    gs_free_object(mem, src, "pop_eexec_filter(strm)");
}

/* We decode the eexec data in place */
static int
ps_font_eexec_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    stream *strm;
    int c;

    strm = push_eexec_filter(mem, buf, bufend);
    while (1) {
        c = sgetc(strm);
        if (c < 0)
            break;
        *buf = (byte) c;
        buf++;
    }
    pop_eexec_filter(mem, strm);

    return 0;
}

/* Normally, for us, "array" is a NULL op.
 *The exception is when the name /Subrs is two objects
 *down from the top of the stack, then we can use this call
 *to record how many subrs we expect, and allocate space for them
 */
static int
ps_font_array_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    ps_font_interp_private *priv = (ps_font_interp_private *) s->client_data;
    int code = 0;

    if (pdf_ps_stack_count(s) < 2) {
        return pdf_ps_stack_pop(s, 1);
    }
    if (pdf_ps_obj_has_type(&s->cur[-1], PDF_PS_OBJ_NAME) &&
        pdf_ps_obj_has_type(&s->cur[0], PDF_PS_OBJ_INTEGER) &&
        !memcmp(s->cur[-1].val.name, PDF_PS_OPER_NAME_AND_LEN("Subrs"))) {

        if (s->cur[0].val.i > 0) {
            if (priv->u.t1.Subrs != NULL) {
                int i;
                for (i = 0; i < priv->u.t1.NumSubrs; i++) {
                    gs_free_object(mem, priv->u.t1.Subrs[i].data, "ps_font_array_func(Subrs[i])");
                }
                gs_free_object(mem, priv->u.t1.Subrs, "ps_font_array_func(Subrs)");
            }

            priv->u.t1.Subrs = (gs_string *) gs_alloc_bytes(mem, s->cur[0].val.i *sizeof(gs_string), "ps_font_array_func(Subrs)");
            if (priv->u.t1.Subrs == NULL) {
                return_error(gs_error_VMerror);
            }
            memset(priv->u.t1.Subrs, 0x00, s->cur[0].val.i * sizeof(gs_string));
        }
        priv->u.t1.NumSubrs = s->cur[0].val.i;
        code = pdf_ps_stack_pop(s, 1);
    }
    else if (pdf_ps_obj_has_type(&s->cur[-1], PDF_PS_OBJ_NAME) &&
             pdf_ps_obj_has_type(&s->cur[0], PDF_PS_OBJ_INTEGER) &&
             !memcmp(s->cur[-1].val.name, PDF_PS_OPER_NAME_AND_LEN("Encoding"))) {
        /* We're defining a custom encoding array */
        pdf_ps_stack_object_t *arr = NULL;
        int size = s->cur[0].val.i;

        if (size > 0) {
            arr = (pdf_ps_stack_object_t *) gs_alloc_bytes(mem, size *sizeof(pdf_ps_stack_object_t), "ps_font_array_func(encoding array)");
            if (arr != NULL) {
                code = pdf_ps_stack_pop(s, 1);
                if (code < 0) {
                    gs_free_object(mem, arr, "ps_font_array_func(encoding array)");
                }
                else {
                    int i;

                    for (i = 0; i < size; i++) {
                        pdf_ps_make_name(&arr[i], (byte *) notdefnamestr, strlen(notdefnamestr));
                    }
                    code = pdf_ps_stack_push_array(s, arr, size);
                }
            }
            else {
                code = gs_note_error(gs_error_VMerror);
            }
        }
    }
    return code;
}

/* Normally, for us, "dict" is a NULL op.
 *The exception is when the name /CharStrings is two objects
 *down from the top of the stack, then we can use this call
 *to record how many charstrings we expect, and allocate space for them
 */
static int
ps_font_dict_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    ps_font_interp_private *priv = (ps_font_interp_private *) s->client_data;

    if (pdf_ps_stack_count(s) < 2) {
        return pdf_ps_stack_pop(s, 1);
    }
    if (pdf_ps_obj_has_type(&s->cur[-1], PDF_PS_OBJ_NAME) &&
        pdf_ps_obj_has_type(&s->cur[0], PDF_PS_OBJ_INTEGER) &&
        !memcmp(s->cur[-1].val.name, PDF_PS_OPER_NAME_AND_LEN("CharStrings"))) {
        int code;
        pdf_dict *d = NULL;

        if (priv->u.t1.CharStrings == NULL) {
            code = pdfi_dict_alloc(s->pdfi_ctx, s->cur[0].val.i, &d);
            if (code < 0) {
                priv->u.t1.CharStrings = NULL;
                (void)pdf_ps_stack_pop(s, 1);
                return code;
            }

            priv->u.t1.CharStrings = d;
            pdfi_countup(priv->u.t1.CharStrings);
        }
    }
    return pdf_ps_stack_pop(s, 1);
}

static int
pdf_ps_pop2_oper_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    return pdf_ps_stack_pop(s, 2);
}

static int
pdf_ps_pop4_oper_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    return pdf_ps_stack_pop(s, 4);
}

static int
pdf_ps_standardencoding_oper_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    return pdf_ps_stack_push_name(s, (byte *) "StandardEncoding", 16);
}

/* { string currentfile exch readstring pop } */
static int
pdf_ps_RD_oper_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    ps_font_interp_private *priv = (ps_font_interp_private *) s->client_data;
    int code;
    int size = 0;

    if (pdf_ps_stack_count(s) >= 1) {
        if (priv->u.t1.Subrs != NULL && priv->u.t1.CharStrings == NULL) {
            if (pdf_ps_obj_has_type(&s->cur[0], PDF_PS_OBJ_INTEGER) &&
                pdf_ps_obj_has_type(&s->cur[-1], PDF_PS_OBJ_INTEGER)) {
                int inx = s->cur[-1].val.i;

                size = s->cur[0].val.i;
                buf++;
                if (buf + size < bufend) {
                    priv->u.t1.Subrs[inx].data =
                        gs_alloc_bytes(mem, size, "pdf_ps_RD_oper_func(subr string)");
                    if (priv->u.t1.Subrs[inx].data == NULL) {
                        (void)pdf_ps_stack_pop(s, 2);
                        return_error(gs_error_VMerror);
                    }
                    memcpy(priv->u.t1.Subrs[inx].data, buf, size);
                    priv->u.t1.Subrs[inx].size = size;
                }
            }
        }
        else if (priv->u.t1.CharStrings != NULL) {
            if (pdf_ps_obj_has_type(&s->cur[0], PDF_PS_OBJ_INTEGER) &&
                pdf_ps_obj_has_type(&s->cur[-1], PDF_PS_OBJ_NAME)) {
                pdf_string *str = NULL;
                pdf_obj *key = NULL;
                bool key_known;

                size = s->cur[0].val.i;
                buf++;
                code = pdfi_name_alloc(s->pdfi_ctx, (byte *) s->cur[-1].val.name, s->cur[-1].size, &key);
                if (code < 0) {
                    (void)pdf_ps_stack_pop(s, 2);
                    return code;
                }
                pdfi_countup(key);

                if (buf + size < bufend) {
                    code = pdfi_dict_known_by_key(s->pdfi_ctx, priv->u.t1.CharStrings, (pdf_name *)key, &key_known);
                    if (code >=0 && key_known != true) {
                         code = pdfi_object_alloc(s->pdfi_ctx, PDF_STRING, size, (pdf_obj **) &str);
                         if (code < 0) {
                             pdfi_countdown(key);
                             (void)pdf_ps_stack_pop(s, 2);
                             return code;
                         }
                         pdfi_countup(str);
                         memcpy(str->data, buf, size);

                         code = pdfi_dict_put_obj(s->pdfi_ctx, priv->u.t1.CharStrings, key, (pdf_obj *) str);
                         if (code < 0) {
                            pdfi_countdown(str);
                            pdfi_countdown(key);
                            (void)pdf_ps_stack_pop(s, 2);
                            return code;
                        }
                    }
                }
                pdfi_countdown(str);
                pdfi_countdown(key);
            }
        }
        code = pdf_ps_stack_pop(s, 2);
        return code < 0 ? code : size + 1;
    }
    return 0;
}

static int
pdf_ps_put_oper_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    int code;

    if ((code = pdf_ps_stack_count(s)) < 4) {
        return 0;
    }

    if (pdf_ps_obj_has_type(&s->cur[-3], PDF_PS_OBJ_NAME) &&
        !memcmp(s->cur[-3].val.name, PDF_PS_OPER_NAME_AND_LEN("Encoding")) &&
        pdf_ps_obj_has_type(&s->cur[-2], PDF_PS_OBJ_ARRAY) &&
        pdf_ps_obj_has_type(&s->cur[-1], PDF_PS_OBJ_INTEGER) &&
        pdf_ps_obj_has_type(&s->cur[0], PDF_PS_OBJ_NAME)) {
        if (s->cur[-1].val.i >= 0 && s->cur[-1].val.i < s->cur[-2].size) {
            pdf_ps_make_name(&s->cur[-2].val.arr[s->cur[-1].val.i], s->cur[0].val.name, s->cur[0].size);
        }
    }

    code = pdf_ps_stack_pop(s, 2);
    return code;
}

static pdf_ps_oper_list_t ps_font_oper_list[] = {
    {PDF_PS_OPER_NAME_AND_LEN("RD"), pdf_ps_RD_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("-|"), pdf_ps_RD_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("|"), pdf_ps_put_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("findresource"), clear_stack_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("dict"), ps_font_dict_func},
    {PDF_PS_OPER_NAME_AND_LEN("begin"), ps_font_dict_begin_func},
    {PDF_PS_OPER_NAME_AND_LEN("end"), ps_font_dict_end_func},
    {PDF_PS_OPER_NAME_AND_LEN("pop"), ps_pdf_null_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("def"), ps_font_def_func},
    {PDF_PS_OPER_NAME_AND_LEN("dup"), ps_pdf_null_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("defineresource"), clear_stack_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("definefont"), clear_stack_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("readonly"), ps_pdf_null_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("true"), ps_font_true_func},
    {PDF_PS_OPER_NAME_AND_LEN("false"), ps_font_false_func},
    {PDF_PS_OPER_NAME_AND_LEN("eexec"), ps_font_eexec_func},
    {PDF_PS_OPER_NAME_AND_LEN("array"), ps_font_array_func},
    {PDF_PS_OPER_NAME_AND_LEN("known"), pdf_ps_pop_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("if"), pdf_ps_pop_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("ifelse"), pdf_ps_pop2_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("for"), pdf_ps_pop4_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("put"), pdf_ps_put_oper_func},
    {PDF_PS_OPER_NAME_AND_LEN("StandardEncoding"), pdf_ps_standardencoding_oper_func},
    {NULL, 0, NULL}
};

int
pdfi_read_ps_font(pdf_context *ctx, pdf_dict *font_dict, byte *fbuf, int fbuflen, ps_font_interp_private *ps_font_priv)
{
    int code = 0;
    pdf_ps_ctx_t ps_font_ctx;

    code = pdfi_pscript_stack_init(ctx, ps_font_oper_list, ps_font_priv, &ps_font_ctx);
    if (code < 0)
        goto error_out;

    code = pdfi_pscript_interpret(&ps_font_ctx, fbuf, fbuflen);
    pdfi_pscript_stack_finit(&ps_font_ctx);
    /* We have several files that have a load of garbage data in the stream after the font is defined,
       and that can end up in a stackoverflow error, even though we have a complete font. Override it
       and let the Type 1 specific code decide for itself if it can use the font.
     */
    if (code == gs_error_stackoverflow)
        code = 0;

    return code;
  error_out:
    code = gs_error_invalidfont;
    return code;
}
