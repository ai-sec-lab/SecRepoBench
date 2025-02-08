static struct expr *
make_cmp(struct expr_context *ctx,
         const struct expr_field *f, enum expr_relop r,
         struct expr_constant_set *cs)
{
    struct expr *e = NULL;

    if (!type_check(ctx, f, cs)) {
        goto exit;
    }

    if (r != EXPR_R_EQ && r != EXPR_R_NE) {
        if (cs->in_curlies) {
            lexer_error(ctx->lexer, "Only == and != operators may be used "
                        "with value sets.");
            goto exit;
        }
        if (f->symbol->level == EXPR_L_NOMINAL ||
            f->symbol->level == EXPR_L_BOOLEAN) {
            lexer_error(ctx->lexer, "Only == and != operators may be used "
                        "with %s field %s.",
                        expr_level_to_string(f->symbol->level),
                        f->symbol->name);
            goto exit;
        }
        // <MASK>
    }

    if (f->symbol->level == EXPR_L_NOMINAL) {
        if (f->symbol->predicate) {
            ovs_assert(f->symbol->width > 0);
            for (size_t i = 0; i < cs->n_values; i++) {
                const union mf_subvalue *value = &cs->values[i].value;
                bool positive = (value->integer & htonll(1)) != 0;
                positive ^= r == EXPR_R_NE;
                positive ^= ctx->not;
                if (!positive) {
                    const char *name = f->symbol->name;
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
                        "account).", f->symbol->name);
            goto exit;
        }
    }

    if (!cs->n_values) {
        e = expr_create_boolean(r == EXPR_R_NE);
        goto exit;
    }
    e = make_cmp__(f, r, &cs->values[0]);
    for (size_t i = 1; i < cs->n_values; i++) {
        e = expr_combine(r == EXPR_R_EQ ? EXPR_T_OR : EXPR_T_AND,
                         e, make_cmp__(f, r, &cs->values[i]));
    }
exit:
    expr_constant_set_destroy(cs);
    return e;
}