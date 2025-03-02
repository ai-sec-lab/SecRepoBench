#line 1207 "grammar.y" /* yacc.c:1663  */
    {
        // Free all the loop variable identifiers and set loop_depth to 0. This
        // is ok even if we have nested loops. If an error occurs while parsing
        // the inner loop, it will be propagated to the outer loop anyways, so
        // it's safe to do this cleanup while processing the error for the
        // inner loop.

        for (int i = 0; i <= compiler->loop_depth; i++)
        {
          loop_vars_cleanup(i);
        }

        compiler->loop_depth = 0;
        YYERROR;
      }
#line 2861 "grammar.c" /* yacc.c:1663  */
    break;

  case 79:
#line 1282 "grammar.y" /* yacc.c:1663  */
    {
        // var_frame is used for accessing local variables used in this loop.
        // All local variables are accessed using var_frame as a reference,
        // like var_frame + 0, var_frame + 1, etc. Here we initialize var_frame
        // with the correct value, which depends on the number of variables
        // defined by any outer loops.

        int var_frame = _yr_compiler_get_var_frame(compiler);
        int result = ERROR_SUCCESS;

        if (compiler->loop_depth == YR_MAX_LOOP_NESTING)
          result = ERROR_LOOP_NESTING_LIMIT_EXCEEDED;

        // This loop uses 3 internal variables besides the ones explicitly
        // defined by the user.
        compiler->loop[compiler->loop_depth].vars_internal_count = 3;

        // Initialize the number of variables, this number will be incremented
        // as variable declaration are processed by for_variables.
        compiler->loop[compiler->loop_depth].vars_count = 0;

        fail_if_error(result);

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_CLEAR_M, var_frame + 0, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_CLEAR_M, var_frame + 1, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_POP_M, var_frame + 2, NULL, NULL));
      }