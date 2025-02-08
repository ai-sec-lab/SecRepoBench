static struct expr *
make_cmp(struct expr_context *ctx,
         const struct expr_field *field, enum expr_relop r,
         struct expr_constant_set *cs)
{
    struct expr *e = NULL;

    if (!type_check(ctx, field, cs)) {
        goto exit;
    }

    if (r != EXPR_R_EQ && r != EXPR_R_NE) {
        if (cs->in_curlies) {
            lexer_error(ctx->lexer, "Only == and != operators may be used "
                        "with value sets.");
            goto exit;
        }
        if (field->symbol->level == EXPR_L_NOMINAL ||
            field->symbol->level == EXPR_L_BOOLEAN) {
            lexer_error(ctx->lexer, "Only == and != operators may be used "
                        "with %s field %s.",
                        expr_level_to_string(field->symbol->level),
                        field->symbol->name);
            goto exit;
        }
        if (!cs->n_values) {
            lexer_error(ctx->lexer, "Only == and != operators may be used "
                        "to compare a field against an empty value set.");
            goto exit;
        }
        if (cs->values[0].masked) {
            lexer_error(ctx->lexer, "Only == and != operators may be used "
                        "with masked constants.  Consider using subfields "
                        "instead (e.g. eth.src[0..15] > 0x1111 in place of "
                        "eth.src > 00:00:00:00:11:11/00:00:00:00:ff:ff).");
            goto exit;
        }
    }

    if (field->symbol->level == EXPR_L_NOMINAL) {
        if (field->symbol->predicate) {
            ovs_assert(field->symbol->width > 0);
            for (size_t i = 0; i < cs->n_values; i++) {
                const union mf_subvalue *value = &cs->values[i].value;
                bool positive = (value->integer & htonll(1)) != 0;
                positive ^= r == EXPR_R_NE;
                positive ^= ctx->not;
                if (!positive) {
                    const char *name = field->symbol->name;
                    lexer_error(ctx->lexer,
                                "Nominal predicate %s may only be tested "
                                "positively, e.g. `%s' or `%s == 1' but not "
                                "`!%s' or `%s == 0'.",
                                name, name, name, name, name);
                    goto exit;
                }
            }
        } else if (r != (ctx->not ? EXPR_R_NE : EXPR_R_EQ)) {
            lexer_error(ctx->lexer, "Nominal field %s may only be tested for "
                        "equality (taking enclosing `!' operators into "
                        "account).", field->symbol->name);
            goto exit;
        }
    }

    if (!cs->n_values) {
        e = expr_create_boolean(r == EXPR_R_NE);
        goto exit;
    }
    e = make_cmp__(field, r, &cs->values[0]);
    for (size_t i = 1; i < cs->n_values; i++) {
        e = expr_combine(r == EXPR_R_EQ ? EXPR_T_OR : EXPR_T_AND,
                         e, make_cmp__(field, r, &cs->values[i]));
    }
exit:
    expr_constant_set_destroy(cs);
    return e;
}