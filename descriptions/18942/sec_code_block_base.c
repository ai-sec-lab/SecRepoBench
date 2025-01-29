{
        // Free all the loop variable identifiers and set loop_depth to 0. This
        // is ok even if we have nested loops. If an error occurs while parsing
        // the inner loop, it will be propagated to the outer loop anyways, so
        // it's safe to do this cleanup while processing the error for the
        // inner loop. If the error is ERROR_LOOP_NESTING_LIMIT_EXCEEDED the
        // value of loop_depth at this point is YR_MAX_LOOP_NESTING, for that
        // reason we use min(loop_depth, YR_MAX_LOOP_NESTING - 1) as the upper
        // bound for i. Using i < loop_depth as the condition (instead of <=)
        // is not an option because when loop_depth < YR_MAX_LOOP_NESTING we
        // want to clean up all loops including the current one, represented
        // by the current value of loop_depth.

        for (int i = 0;
             i <= yr_min(compiler->loop_depth,YR_MAX_LOOP_NESTING - 1);
             i++)
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

        fail_if_error(result);

        // This loop uses 3 internal variables besides the ones explicitly
        // defined by the user.
        compiler->loop[compiler->loop_depth].vars_internal_count = 3;

        // Initialize the number of variables, this number will be incremented
        // as variable declaration are processed by for_variables.
        compiler->loop[compiler->loop_depth].vars_count = 0;

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_CLEAR_M, var_frame + 0, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_CLEAR_M, var_frame + 1, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_POP_M, var_frame + 2, NULL, NULL));
      }
#line 2898 "grammar.c" /* yacc.c:1663  */
    break;

  case 80:
#line 1315 "grammar.y" /* yacc.c:1663  */
    {
        YR_LOOP_CONTEXT* loop_ctx = &compiler->loop[compiler->loop_depth];
        YR_FIXUP* fixup;

        uint8_t* loop_start_addr;
        void* jmp_arg_addr;

        int var_frame = _yr_compiler_get_var_frame(compiler);

        fail_if_error(yr_parser_emit(
            yyscanner, OP_ITER_NEXT, &loop_start_addr));

        // For each variable generate an instruction that pops the value from
        // the stack and store it into one memory slot starting at var_frame + 3
        // because the first 3 slots in the frame are for the internal variables.

        for (int i = 0; i < loop_ctx->vars_count; i++)
        {
          fail_if_error(yr_parser_emit_with_arg(
              yyscanner, OP_POP_M, var_frame + 3 + i, NULL, NULL));
        }

        fail_if_error(yr_parser_emit_with_arg_reloc(
            yyscanner,
            OP_JTRUE_P,
            0,
            NULL,
            &jmp_arg_addr));

        // Push a new fixup entry in the fixup stack so that the jump
        // destination is set once we know it.

        fixup = (YR_FIXUP*) yr_malloc(sizeof(YR_FIXUP));

        if (fixup == NULL)
          fail_if_error(ERROR_INSUFFICIENT_MEMORY);

        fixup->address = jmp_arg_addr;
        fixup->next = compiler->fixup_stack_head;
        compiler->fixup_stack_head = fixup;

        loop_ctx->addr = loop_start_addr;
        compiler->loop_depth++;
      }
#line 2947 "grammar.c" /* yacc.c:1663  */
    break;

  case 81:
#line 1360 "grammar.y" /* yacc.c:1663  */
    {
        YR_FIXUP* fixup;
        uint8_t* pop_addr;
        int var_frame;

        compiler->loop_depth--;

        loop_vars_cleanup(compiler->loop_depth);

        var_frame = _yr_compiler_get_var_frame(compiler);

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_ADD_M, var_frame + 0, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_INCR_M, var_frame + 1, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_PUSH_M, var_frame + 2, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg_reloc(
            yyscanner,
            OP_JUNDEF_P,
            compiler->loop[compiler->loop_depth].addr,
            NULL,
            NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_PUSH_M, var_frame + 0, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_PUSH_M, var_frame + 2, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg_reloc(
            yyscanner,
            OP_JL_P,
            compiler->loop[compiler->loop_depth].addr,
            NULL,
            NULL));

        fail_if_error(yr_parser_emit(
            yyscanner, OP_POP, &pop_addr));

        // Pop from the stack the fixup entry containing the jump's address
        // that needs to be fixed.

        fixup = compiler->fixup_stack_head;
        compiler->fixup_stack_head = fixup->next;

        // Fix the jump's target address.
        *(void**)(fixup->address) = (void*)(pop_addr);

        yr_free(fixup);

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_PUSH_M, var_frame + 0, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_PUSH_M, var_frame + 2, NULL, NULL));

        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_SWAPUNDEF, var_frame + 1, NULL, NULL));

        fail_if_error(yr_parser_emit(
            yyscanner, OP_INT_GE, NULL));
      }
#line 3018 "grammar.c" /* yacc.c:1663  */
    break;

  case 82:
#line 1427 "grammar.y" /* yacc.c:1663  */
    {
        int result = ERROR_SUCCESS;
        int var_frame = _yr_compiler_get_var_frame(compiler);;
        uint8_t* addr;

        if (compiler->loop_depth == YR_MAX_LOOP_NESTING)
          result = ERROR_LOOP_NESTING_LIMIT_EXCEEDED;

        if (compiler->loop_for_of_var_index != -1)
          result = ERROR_NESTED_FOR_OF_LOOP;

        fail_if_error(result);

        yr_parser_emit_with_arg(
            yyscanner, OP_CLEAR_M, var_frame + 1, NULL, NULL);

        yr_parser_emit_with_arg(
            yyscanner, OP_CLEAR_M, var_frame + 2, NULL, NULL);

        // Pop the first string.
        yr_parser_emit_with_arg(
            yyscanner, OP_POP_M, var_frame, &addr, NULL);

        compiler->loop_for_of_var_index = var_frame;
        compiler->loop[compiler->loop_depth].vars_internal_count = 3;
        compiler->loop[compiler->loop_depth].vars_count = 0;
        compiler->loop[compiler->loop_depth].addr = addr;
        compiler->loop_depth++;
      }
#line 3052 "grammar.c" /* yacc.c:1663  */
    break;

  case 83:
#line 1457 "grammar.y" /* yacc.c:1663  */
    {
        int var_frame = 0;

        compiler->loop_depth--;
        compiler->loop_for_of_var_index = -1;

        loop_vars_cleanup(compiler->loop_depth);

        var_frame = _yr_compiler_get_var_frame(compiler);

        // Increment counter by the value returned by the
        // boolean expression (0 or 1). If the boolean expression
        // returned UNDEFINED the OP_ADD_M won't do anything.

        yr_parser_emit_with_arg(
            yyscanner, OP_ADD_M, var_frame + 1, NULL, NULL);

        // Increment iterations counter.
        yr_parser_emit_with_arg(
            yyscanner, OP_INCR_M, var_frame + 2, NULL, NULL);

        // If next string is not undefined, go back to the
        // beginning of the loop.
        yr_parser_emit_with_arg_reloc(
            yyscanner,
            OP_JNUNDEF,
            compiler->loop[compiler->loop_depth].addr,
            NULL,
            NULL);

        // Pop end-of-list marker.
        yr_parser_emit(yyscanner, OP_POP, NULL);

        // At this point the loop quantifier (any, all, 1, 2,..)
        // is at top of the stack. Check if the quantifier is
        // undefined (meaning "all") and replace it with the
        // iterations counter in that case.
        yr_parser_emit_with_arg(
            yyscanner, OP_SWAPUNDEF, var_frame + 2, NULL, NULL);

        // Compare the loop quantifier with the number of
        // expressions evaluating to true.
        yr_parser_emit_with_arg(
            yyscanner, OP_PUSH_M, var_frame + 1, NULL, NULL);

        yr_parser_emit(yyscanner, OP_INT_LE, NULL);

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3106 "grammar.c" /* yacc.c:1663  */
    break;

  case 84:
#line 1507 "grammar.y" /* yacc.c:1663  */
    {
        yr_parser_emit(yyscanner, OP_OF, NULL);

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3116 "grammar.c" /* yacc.c:1663  */
    break;

  case 85:
#line 1513 "grammar.y" /* yacc.c:1663  */
    {
        yr_parser_emit(yyscanner, OP_NOT, NULL);

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3126 "grammar.c" /* yacc.c:1663  */
    break;

  case 86:
#line 1519 "grammar.y" /* yacc.c:1663  */
    {
        YR_FIXUP* fixup;
        void* jmp_destination_addr;

        fail_if_error(yr_parser_emit_with_arg_reloc(
            yyscanner,
            OP_JFALSE,
            0,          // still don't know the jump destination
            NULL,
            &jmp_destination_addr));

        // create a fixup entry for the jump and push it in the stack
        fixup = (YR_FIXUP*) yr_malloc(sizeof(YR_FIXUP));

        if (fixup == NULL)
          fail_if_error(ERROR_INSUFFICIENT_MEMORY);

        fixup->address = jmp_destination_addr;
        fixup->next = compiler->fixup_stack_head;
        compiler->fixup_stack_head = fixup;
      }
#line 3152 "grammar.c" /* yacc.c:1663  */
    break;

  case 87:
#line 1541 "grammar.y" /* yacc.c:1663  */
    {
        YR_FIXUP* fixup;
        uint8_t* nop_addr;

        fail_if_error(yr_parser_emit(yyscanner, OP_AND, NULL));

        // Generate a do-nothing instruction (NOP) in order to get its address
        // and use it as the destination for the OP_JFALSE. We can not simply
        // use the address of the OP_AND instruction +1 because we can't be
        // sure that the instruction following the OP_AND is going to be in
        // the same arena page. As we don't have a reliable way of getting the
        // address of the next instruction we generate the OP_NOP.

        fail_if_error(yr_parser_emit(yyscanner, OP_NOP, &nop_addr));

        fixup = compiler->fixup_stack_head;
        *(void**)(fixup->address) = (void*) nop_addr;
        compiler->fixup_stack_head = fixup->next;
        yr_free(fixup);

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3179 "grammar.c" /* yacc.c:1663  */
    break;

  case 88:
#line 1564 "grammar.y" /* yacc.c:1663  */
    {
        YR_FIXUP* fixup;
        void* jmp_destination_addr;

        fail_if_error(yr_parser_emit_with_arg_reloc(
            yyscanner,
            OP_JTRUE,
            0,         // still don't know the jump destination
            NULL,
            &jmp_destination_addr));

        fixup = (YR_FIXUP*) yr_malloc(sizeof(YR_FIXUP));

        if (fixup == NULL)
          fail_if_error(ERROR_INSUFFICIENT_MEMORY);

        fixup->address = jmp_destination_addr;
        fixup->next = compiler->fixup_stack_head;
        compiler->fixup_stack_head = fixup;
      }
#line 3204 "grammar.c" /* yacc.c:1663  */
    break;

  case 89:
#line 1585 "grammar.y" /* yacc.c:1663  */
    {
        YR_FIXUP* fixup;
        uint8_t* nop_addr;

        fail_if_error(yr_parser_emit(yyscanner, OP_OR, NULL));

        // Generate a do-nothing instruction (NOP) in order to get its address
        // and use it as the destination for the OP_JFALSE. We can not simply
        // use the address of the OP_OR instruction +1 because we can't be
        // sure that the instruction following the OP_AND is going to be in
        // the same arena page. As we don't have a reliable way of getting the
        // address of the next instruction we generate the OP_NOP.

        fail_if_error(yr_parser_emit(yyscanner, OP_NOP, &nop_addr));

        fixup = compiler->fixup_stack_head;
        *(void**)(fixup->address) = (void*)(nop_addr);
        compiler->fixup_stack_head = fixup->next;
        yr_free(fixup);

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3231 "grammar.c" /* yacc.c:1663  */
    break;

  case 90:
#line 1608 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_reduce_operation(
            yyscanner, "<", (yyvsp[-2].expression), (yyvsp[0].expression)));

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3242 "grammar.c" /* yacc.c:1663  */
    break;

  case 91:
#line 1615 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_reduce_operation(
            yyscanner, ">", (yyvsp[-2].expression), (yyvsp[0].expression)));

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3253 "grammar.c" /* yacc.c:1663  */
    break;

  case 92:
#line 1622 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_reduce_operation(
            yyscanner, "<=", (yyvsp[-2].expression), (yyvsp[0].expression)));

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3264 "grammar.c" /* yacc.c:1663  */
    break;

  case 93:
#line 1629 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_reduce_operation(
            yyscanner, ">=", (yyvsp[-2].expression), (yyvsp[0].expression)));

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3275 "grammar.c" /* yacc.c:1663  */
    break;

  case 94:
#line 1636 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_reduce_operation(
            yyscanner, "==", (yyvsp[-2].expression), (yyvsp[0].expression)));

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3286 "grammar.c" /* yacc.c:1663  */
    break;

  case 95:
#line 1643 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_reduce_operation(
            yyscanner, "!=", (yyvsp[-2].expression), (yyvsp[0].expression)));

        (yyval.expression).type = EXPRESSION_TYPE_BOOLEAN;
      }
#line 3297 "grammar.c" /* yacc.c:1663  */
    break;

  case 96:
#line 1650 "grammar.y" /* yacc.c:1663  */
    {
        (yyval.expression) = (yyvsp[0].expression);
      }
#line 3305 "grammar.c" /* yacc.c:1663  */
    break;

  case 97:
#line 1654 "grammar.y" /* yacc.c:1663  */
    {
        (yyval.expression) = (yyvsp[-1].expression);
      }
#line 3313 "grammar.c" /* yacc.c:1663  */
    break;

  case 98:
#line 1662 "grammar.y" /* yacc.c:1663  */
    {
        int result = ERROR_SUCCESS;

        YR_LOOP_CONTEXT* loop_ctx = &compiler->loop[compiler->loop_depth];

        if (yr_parser_lookup_loop_variable(yyscanner, (yyvsp[0].c_string), NULL) >= 0)
        {
          yr_compiler_set_error_extra_info(compiler, (yyvsp[0].c_string));
          yr_free((yyvsp[0].c_string));

          result = ERROR_DUPLICATED_LOOP_IDENTIFIER;
        }

        fail_if_error(result);

        loop_ctx->vars[loop_ctx->vars_count++].identifier = (yyvsp[0].c_string);

        assert(loop_ctx->vars_count <= YR_MAX_LOOP_VARS);
      }
#line 3337 "grammar.c" /* yacc.c:1663  */
    break;

  case 99:
#line 1682 "grammar.y" /* yacc.c:1663  */
    {
        int result = ERROR_SUCCESS;

        YR_LOOP_CONTEXT* loop_ctx = &compiler->loop[compiler->loop_depth];

        if (loop_ctx->vars_count == YR_MAX_LOOP_VARS)
        {
          yr_compiler_set_error_extra_info(compiler, "too many loop variables");
          yr_free((yyvsp[0].c_string));

          result = ERROR_SYNTAX_ERROR;
        }
        else if (yr_parser_lookup_loop_variable(yyscanner, (yyvsp[0].c_string), NULL) >= 0)
        {
          yr_compiler_set_error_extra_info(compiler, (yyvsp[0].c_string));
          yr_free((yyvsp[0].c_string));

          result = ERROR_DUPLICATED_LOOP_IDENTIFIER;
        }

        fail_if_error(result);

        loop_ctx->vars[loop_ctx->vars_count++].identifier = (yyvsp[0].c_string);
      }
#line 3366 "grammar.c" /* yacc.c:1663  */
    break;

  case 100:
#line 1710 "grammar.y" /* yacc.c:1663  */
    {
        YR_LOOP_CONTEXT* loop_ctx = &compiler->loop[compiler->loop_depth];

        // Initially we assume that the identifier is from a non-iterable type,
        // this will change later if it's iterable.
        int result = ERROR_WRONG_TYPE;

        if ((yyvsp[0].expression).type == EXPRESSION_TYPE_OBJECT)
        {
          switch((yyvsp[0].expression).value.object->type)
          {
            case OBJECT_TYPE_ARRAY:
              // If iterating an array the loop must define a single variable
              // that will hold the current item. If a different number of
              // variables were defined that's an error.
              if (loop_ctx->vars_count == 1)
              {
                loop_ctx->vars[0].type = EXPRESSION_TYPE_OBJECT;
                loop_ctx->vars[0].value.object = \
                    object_as_array((yyvsp[0].expression).value.object)->prototype_item;

                result = yr_parser_emit(yyscanner, OP_ITER_START_ARRAY, NULL);
              }
              else
              {
                yr_compiler_set_error_extra_info_fmt(
                    compiler,
                    "iterator for \"%s\" yields a single item on each iteration"
                    ", but the loop expects %d",
                    (yyvsp[0].expression).identifier,
                    loop_ctx->vars_count);

                result =  ERROR_SYNTAX_ERROR;
              }
              break;

            case OBJECT_TYPE_DICTIONARY:
              // If iterating a dictionary the loop must define exactly two
              // variables, one for the key and another for the value . If a
              // different number of variables were defined that's an error.
              if (loop_ctx->vars_count == 2)
              {
                loop_ctx->vars[0].type = EXPRESSION_TYPE_STRING;
                loop_ctx->vars[0].value.sized_string = NULL;
                loop_ctx->vars[1].type = EXPRESSION_TYPE_OBJECT;
                loop_ctx->vars[1].value.object = \
                    object_as_array((yyvsp[0].expression).value.object)->prototype_item;

                result = yr_parser_emit(yyscanner, OP_ITER_START_DICT, NULL);
              }
              else
              {
                yr_compiler_set_error_extra_info_fmt(
                    compiler,
                    "iterator for \"%s\" yields a key,value pair item on each iteration",
                    (yyvsp[0].expression).identifier);

                result =  ERROR_SYNTAX_ERROR;
              }
              break;
          }
        }

        if (result == ERROR_WRONG_TYPE)
        {
          yr_compiler_set_error_extra_info_fmt(
              compiler,
              "identifier \"%s\" is not iterable",
              (yyvsp[0].expression).identifier);
        }

        fail_if_error(result);
      }
#line 3444 "grammar.c" /* yacc.c:1663  */
    break;

  case 101:
#line 1784 "grammar.y" /* yacc.c:1663  */
    {
        int result = ERROR_SUCCESS;

        YR_LOOP_CONTEXT* loop_ctx = &compiler->loop[compiler->loop_depth];

        if (loop_ctx->vars_count == 1)
        {
          loop_ctx->vars[0].type = EXPRESSION_TYPE_INTEGER;
          loop_ctx->vars[0].value.integer = UNDEFINED;
        }
        else
        {
          yr_compiler_set_error_extra_info_fmt(
              compiler,
              "iterator yields an integer on each iteration "
              ", but the loop expects %d",
              loop_ctx->vars_count);

          result =  ERROR_SYNTAX_ERROR;
        }

        fail_if_error(result);
      }
#line 3472 "grammar.c" /* yacc.c:1663  */
    break;

  case 102:
#line 1812 "grammar.y" /* yacc.c:1663  */
    {
        // $2 contains the number of integers in the enumeration
        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_PUSH, (yyvsp[-1].integer), NULL, NULL));

        fail_if_error(yr_parser_emit(
            yyscanner, OP_ITER_START_INT_ENUM, NULL));
      }
#line 3485 "grammar.c" /* yacc.c:1663  */
    break;

  case 103:
#line 1821 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_emit(
            yyscanner, OP_ITER_START_INT_RANGE, NULL));
      }
#line 3494 "grammar.c" /* yacc.c:1663  */
    break;

  case 104:
#line 1830 "grammar.y" /* yacc.c:1663  */
    {
        int result = ERROR_SUCCESS;

        if ((yyvsp[-3].expression).type != EXPRESSION_TYPE_INTEGER)
        {
          yr_compiler_set_error_extra_info(
              compiler, "wrong type for range's lower bound");
          result = ERROR_WRONG_TYPE;
        }

        if ((yyvsp[-1].expression).type != EXPRESSION_TYPE_INTEGER)
        {
          yr_compiler_set_error_extra_info(
              compiler, "wrong type for range's upper bound");
          result = ERROR_WRONG_TYPE;
        }

        fail_if_error(result);
      }
#line 3518 "grammar.c" /* yacc.c:1663  */
    break;

  case 105:
#line 1854 "grammar.y" /* yacc.c:1663  */
    {
        int result = ERROR_SUCCESS;

        if ((yyvsp[0].expression).type != EXPRESSION_TYPE_INTEGER)
        {
          yr_compiler_set_error_extra_info(
              compiler, "wrong type for enumeration item");
          result = ERROR_WRONG_TYPE;
        }

        fail_if_error(result);

        (yyval.integer) = 1;
      }
#line 3537 "grammar.c" /* yacc.c:1663  */
    break;

  case 106:
#line 1869 "grammar.y" /* yacc.c:1663  */
    {
        int result = ERROR_SUCCESS;

        if ((yyvsp[0].expression).type != EXPRESSION_TYPE_INTEGER)
        {
          yr_compiler_set_error_extra_info(
              compiler, "wrong type for enumeration item");
          result = ERROR_WRONG_TYPE;
        }

        fail_if_error(result);

        (yyval.integer) = (yyvsp[-2].integer) + 1;
      }
#line 3556 "grammar.c" /* yacc.c:1663  */
    break;

  case 107:
#line 1888 "grammar.y" /* yacc.c:1663  */
    {
        // Push end-of-list marker
        yr_parser_emit_with_arg(yyscanner, OP_PUSH, UNDEFINED, NULL, NULL);
      }
#line 3565 "grammar.c" /* yacc.c:1663  */
    break;

  case 109:
#line 1894 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_PUSH, UNDEFINED, NULL, NULL));

        fail_if_error(yr_parser_emit_pushes_for_strings(
            yyscanner, "$*"));
      }
#line 3577 "grammar.c" /* yacc.c:1663  */
    break;

  case 112:
#line 1912 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_emit_pushes_for_strings(yyscanner, (yyvsp[0].c_string));
        yr_free((yyvsp[0].c_string));

        fail_if_error(result);
      }
#line 3588 "grammar.c" /* yacc.c:1663  */
    break;

  case 113:
#line 1919 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_emit_pushes_for_strings(yyscanner, (yyvsp[0].c_string));
        yr_free((yyvsp[0].c_string));

        fail_if_error(result);
      }
#line 3599 "grammar.c" /* yacc.c:1663  */
    break;

  case 114:
#line 1930 "grammar.y" /* yacc.c:1663  */
    {
        (yyval.integer) = FOR_EXPRESSION_ANY;
      }
#line 3607 "grammar.c" /* yacc.c:1663  */
    break;

  case 115:
#line 1934 "grammar.y" /* yacc.c:1663  */
    {
        yr_parser_emit_with_arg(yyscanner, OP_PUSH, UNDEFINED, NULL, NULL);
        (yyval.integer) = FOR_EXPRESSION_ALL;
      }
#line 3616 "grammar.c" /* yacc.c:1663  */
    break;

  case 116:
#line 1939 "grammar.y" /* yacc.c:1663  */
    {
        yr_parser_emit_with_arg(yyscanner, OP_PUSH, 1, NULL, NULL);
        (yyval.integer) = FOR_EXPRESSION_ANY;
      }
#line 3625 "grammar.c" /* yacc.c:1663  */
    break;

  case 117:
#line 1948 "grammar.y" /* yacc.c:1663  */
    {
        (yyval.expression) = (yyvsp[-1].expression);
      }
#line 3633 "grammar.c" /* yacc.c:1663  */
    break;

  case 118:
#line 1952 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_emit(
            yyscanner, OP_FILESIZE, NULL));

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = UNDEFINED;
      }
#line 3645 "grammar.c" /* yacc.c:1663  */
    break;

  case 119:
#line 1960 "grammar.y" /* yacc.c:1663  */
    {
        yywarning(yyscanner,
            "Using deprecated \"entrypoint\" keyword. Use the \"entry_point\" "
            "function from PE module instead.");

        fail_if_error(yr_parser_emit(
            yyscanner, OP_ENTRYPOINT, NULL));

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = UNDEFINED;
      }
#line 3661 "grammar.c" /* yacc.c:1663  */
    break;

  case 120:
#line 1972 "grammar.y" /* yacc.c:1663  */
    {
        check_type((yyvsp[-1].expression), EXPRESSION_TYPE_INTEGER, "intXXXX or uintXXXX");

        // _INTEGER_FUNCTION_ could be any of int8, int16, int32, uint8,
        // uint32, etc. $1 contains an index that added to OP_READ_INT results
        // in the proper OP_INTXX opcode.

        fail_if_error(yr_parser_emit(
            yyscanner, (uint8_t) (OP_READ_INT + (yyvsp[-3].integer)), NULL));

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = UNDEFINED;
      }
#line 3679 "grammar.c" /* yacc.c:1663  */
    break;

  case 121:
#line 1986 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_emit_with_arg(
            yyscanner, OP_PUSH, (yyvsp[0].integer), NULL, NULL));

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = (yyvsp[0].integer);
      }
#line 3691 "grammar.c" /* yacc.c:1663  */
    break;

  case 122:
#line 1994 "grammar.y" /* yacc.c:1663  */
    {
        fail_if_error(yr_parser_emit_with_arg_double(
            yyscanner, OP_PUSH, (yyvsp[0].double_), NULL, NULL));

        (yyval.expression).type = EXPRESSION_TYPE_FLOAT;
      }
#line 3702 "grammar.c" /* yacc.c:1663  */
    break;

  case 123:
#line 2001 "grammar.y" /* yacc.c:1663  */
    {
        SIZED_STRING* sized_string;

        int result = yr_arena_write_data(
            compiler->sz_arena,
            (yyvsp[0].sized_string),
            (yyvsp[0].sized_string)->length + sizeof(SIZED_STRING),
            (void**) &sized_string);

        yr_free((yyvsp[0].sized_string));

        if (result == ERROR_SUCCESS)
          result = yr_parser_emit_with_arg_reloc(
              yyscanner,
              OP_PUSH,
              sized_string,
              NULL,
              NULL);

        fail_if_error(result);

        (yyval.expression).type = EXPRESSION_TYPE_STRING;
        (yyval.expression).value.sized_string = sized_string;
      }
#line 3731 "grammar.c" /* yacc.c:1663  */
    break;

  case 124:
#line 2026 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_reduce_string_identifier(
            yyscanner, (yyvsp[0].c_string), OP_COUNT, UNDEFINED);

        yr_free((yyvsp[0].c_string));

        fail_if_error(result);

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = UNDEFINED;
      }
#line 3747 "grammar.c" /* yacc.c:1663  */
    break;

  case 125:
#line 2038 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_reduce_string_identifier(
            yyscanner, (yyvsp[-3].c_string), OP_OFFSET, UNDEFINED);

        yr_free((yyvsp[-3].c_string));

        fail_if_error(result);

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = UNDEFINED;
      }
#line 3763 "grammar.c" /* yacc.c:1663  */
    break;

  case 126:
#line 2050 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_emit_with_arg(
            yyscanner, OP_PUSH, 1, NULL, NULL);

        if (result == ERROR_SUCCESS)
          result = yr_parser_reduce_string_identifier(
              yyscanner, (yyvsp[0].c_string), OP_OFFSET, UNDEFINED);

        yr_free((yyvsp[0].c_string));

        fail_if_error(result);

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = UNDEFINED;
      }
#line 3783 "grammar.c" /* yacc.c:1663  */
    break;

  case 127:
#line 2066 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_reduce_string_identifier(
            yyscanner, (yyvsp[-3].c_string), OP_LENGTH, UNDEFINED);

        yr_free((yyvsp[-3].c_string));

        fail_if_error(result);

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = UNDEFINED;
      }
#line 3799 "grammar.c" /* yacc.c:1663  */
    break;

  case 128:
#line 2078 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_emit_with_arg(
            yyscanner, OP_PUSH, 1, NULL, NULL);

        if (result == ERROR_SUCCESS)
          result = yr_parser_reduce_string_identifier(
              yyscanner, (yyvsp[0].c_string), OP_LENGTH, UNDEFINED);

        yr_free((yyvsp[0].c_string));

        fail_if_error(result);

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = UNDEFINED;
      }
#line 3819 "grammar.c" /* yacc.c:1663  */
    break;

  case 129:
#line 2094 "grammar.y" /* yacc.c:1663  */
    {
        int result = ERROR_SUCCESS;

        if ((yyvsp[0].expression).type == EXPRESSION_TYPE_OBJECT)
        {
          result = yr_parser_emit(
              yyscanner, OP_OBJ_VALUE, NULL);

          switch((yyvsp[0].expression).value.object->type)
          {
            case OBJECT_TYPE_INTEGER:
              (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
              (yyval.expression).value.integer = UNDEFINED;
              break;
            case OBJECT_TYPE_FLOAT:
              (yyval.expression).type = EXPRESSION_TYPE_FLOAT;
              break;
            case OBJECT_TYPE_STRING:
              (yyval.expression).type = EXPRESSION_TYPE_STRING;
              (yyval.expression).value.sized_string = NULL;
              break;
            default:
              // In a primary expression any identifier that corresponds to an
              // object must be of type integer, float or string. If "foobar" is
              // either a function, structure, dictionary or array you can not
              // use it as:
              //   condition: foobar
              yr_compiler_set_error_extra_info_fmt(
                  compiler,
                  "wrong usage of identifier \"%s\"",
                  (yyvsp[0].expression).identifier);
              result = ERROR_WRONG_TYPE;
          }
        }
        else
        {
          (yyval.expression) = (yyvsp[0].expression);
        }

        fail_if_error(result);
      }
#line 3865 "grammar.c" /* yacc.c:1663  */
    break;

  case 130:
#line 2136 "grammar.y" /* yacc.c:1663  */
    {
        int result = ERROR_SUCCESS;

        check_type((yyvsp[0].expression), EXPRESSION_TYPE_INTEGER | EXPRESSION_TYPE_FLOAT, "-");

        if ((yyvsp[0].expression).type == EXPRESSION_TYPE_INTEGER)
        {
          (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
          (yyval.expression).value.integer = ((yyvsp[0].expression).value.integer == UNDEFINED) ?
              UNDEFINED : -((yyvsp[0].expression).value.integer);
          result = yr_parser_emit(yyscanner, OP_INT_MINUS, NULL);
        }
        else if ((yyvsp[0].expression).type == EXPRESSION_TYPE_FLOAT)
        {
          (yyval.expression).type = EXPRESSION_TYPE_FLOAT;
          result = yr_parser_emit(yyscanner, OP_DBL_MINUS, NULL);
        }

        fail_if_error(result);
      }
#line 3890 "grammar.c" /* yacc.c:1663  */
    break;

  case 131:
#line 2157 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_reduce_operation(
            yyscanner, "+", (yyvsp[-2].expression), (yyvsp[0].expression));

        if ((yyvsp[-2].expression).type == EXPRESSION_TYPE_INTEGER &&
            (yyvsp[0].expression).type == EXPRESSION_TYPE_INTEGER)
        {
          int64_t i1 = (yyvsp[-2].expression).value.integer;
          int64_t i2 = (yyvsp[0].expression).value.integer;

          if (!IS_UNDEFINED(i1) && !IS_UNDEFINED(i2) &&
              (
                (i2 > 0 && i1 > INT64_MAX - i2) ||
                (i2 < 0 && i1 < INT64_MIN - i2)
              ))
          {
            yr_compiler_set_error_extra_info_fmt(
                compiler, "%" PRId64 " + %" PRId64, i1, i2);

            result = ERROR_INTEGER_OVERFLOW;
          }
          else
          {
            (yyval.expression).value.integer = OPERATION(+, i1, i2);
            (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
          }
        }
        else
        {
          (yyval.expression).type = EXPRESSION_TYPE_FLOAT;
        }

        fail_if_error(result);
      }
#line 3929 "grammar.c" /* yacc.c:1663  */
    break;

  case 132:
#line 2192 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_reduce_operation(
            yyscanner, "-", (yyvsp[-2].expression), (yyvsp[0].expression));

        if ((yyvsp[-2].expression).type == EXPRESSION_TYPE_INTEGER &&
            (yyvsp[0].expression).type == EXPRESSION_TYPE_INTEGER)
        {
          int64_t i1 = (yyvsp[-2].expression).value.integer;
          int64_t i2 = (yyvsp[0].expression).value.integer;

          if (!IS_UNDEFINED(i1) && !IS_UNDEFINED(i2) &&
              (
                (i2 < 0 && i1 > INT64_MAX + i2) ||
                (i2 > 0 && i1 < INT64_MIN + i2)
              ))
          {
            yr_compiler_set_error_extra_info_fmt(
                compiler, "%" PRId64 " - %" PRId64, i1, i2);

            result = ERROR_INTEGER_OVERFLOW;
          }
          else
          {
            (yyval.expression).value.integer = OPERATION(-, i1, i2);
            (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
          }
        }
        else
        {
          (yyval.expression).type = EXPRESSION_TYPE_FLOAT;
        }

        fail_if_error(result);
      }
#line 3968 "grammar.c" /* yacc.c:1663  */
    break;

  case 133:
#line 2227 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_reduce_operation(
            yyscanner, "*", (yyvsp[-2].expression), (yyvsp[0].expression));

        if ((yyvsp[-2].expression).type == EXPRESSION_TYPE_INTEGER &&
            (yyvsp[0].expression).type == EXPRESSION_TYPE_INTEGER)
        {
          int64_t i1 = (yyvsp[-2].expression).value.integer;
          int64_t i2 = (yyvsp[0].expression).value.integer;

          if (!IS_UNDEFINED(i1) && !IS_UNDEFINED(i2) &&
              (
                i2 != 0 && llabs(i1) > INT64_MAX / llabs(i2)
              ))
          {
            yr_compiler_set_error_extra_info_fmt(
                compiler, "%" PRId64 " * %" PRId64, i1, i2);

            result = ERROR_INTEGER_OVERFLOW;
          }
          else
          {
            (yyval.expression).value.integer = OPERATION(*, i1, i2);
            (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
          }
        }
        else
        {
          (yyval.expression).type = EXPRESSION_TYPE_FLOAT;
        }

        fail_if_error(result);
      }
#line 4006 "grammar.c" /* yacc.c:1663  */
    break;

  case 134:
#line 2261 "grammar.y" /* yacc.c:1663  */
    {
        int result = yr_parser_reduce_operation(
            yyscanner, "\\", (yyvsp[-2].expression), (yyvsp[0].expression));

        if ((yyvsp[-2].expression).type == EXPRESSION_TYPE_INTEGER &&
            (yyvsp[0].expression).type == EXPRESSION_TYPE_INTEGER)
        {
          if ((yyvsp[0].expression).value.integer != 0)
          {
            (yyval.expression).value.integer = OPERATION(/, (yyvsp[-2].expression).value.integer, (yyvsp[0].expression).value.integer);
            (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
          }
          else
          {
            result = ERROR_DIVISION_BY_ZERO;
          }
        }
        else
        {
          (yyval.expression).type = EXPRESSION_TYPE_FLOAT;
        }

        fail_if_error(result);
      }
#line 4035 "grammar.c" /* yacc.c:1663  */
    break;

  case 135:
#line 2286 "grammar.y" /* yacc.c:1663  */
    {
        check_type((yyvsp[-2].expression), EXPRESSION_TYPE_INTEGER, "%");
        check_type((yyvsp[0].expression), EXPRESSION_TYPE_INTEGER, "%");

        fail_if_error(yr_parser_emit(yyscanner, OP_MOD, NULL));

        if ((yyvsp[0].expression).value.integer != 0)
        {
          (yyval.expression).value.integer = OPERATION(%, (yyvsp[-2].expression).value.integer, (yyvsp[0].expression).value.integer);
          (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        }
        else
        {
          fail_if_error(ERROR_DIVISION_BY_ZERO);
        }
      }
#line 4056 "grammar.c" /* yacc.c:1663  */
    break;

  case 136:
#line 2303 "grammar.y" /* yacc.c:1663  */
    {
        check_type((yyvsp[-2].expression), EXPRESSION_TYPE_INTEGER, "^");
        check_type((yyvsp[0].expression), EXPRESSION_TYPE_INTEGER, "^");

        fail_if_error(yr_parser_emit(yyscanner, OP_BITWISE_XOR, NULL));

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = OPERATION(^, (yyvsp[-2].expression).value.integer, (yyvsp[0].expression).value.integer);
      }
#line 4070 "grammar.c" /* yacc.c:1663  */
    break;

  case 137:
#line 2313 "grammar.y" /* yacc.c:1663  */
    {
        check_type((yyvsp[-2].expression), EXPRESSION_TYPE_INTEGER, "^");
        check_type((yyvsp[0].expression), EXPRESSION_TYPE_INTEGER, "^");

        fail_if_error(yr_parser_emit(yyscanner, OP_BITWISE_AND, NULL));

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = OPERATION(&, (yyvsp[-2].expression).value.integer, (yyvsp[0].expression).value.integer);
      }
#line 4084 "grammar.c" /* yacc.c:1663  */
    break;

  case 138:
#line 2323 "grammar.y" /* yacc.c:1663  */
    {
        check_type((yyvsp[-2].expression), EXPRESSION_TYPE_INTEGER, "|");
        check_type((yyvsp[0].expression), EXPRESSION_TYPE_INTEGER, "|");

        fail_if_error(yr_parser_emit(yyscanner, OP_BITWISE_OR, NULL));

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = OPERATION(|, (yyvsp[-2].expression).value.integer, (yyvsp[0].expression).value.integer);
      }
#line 4098 "grammar.c" /* yacc.c:1663  */
    break;

  case 139:
#line 2333 "grammar.y" /* yacc.c:1663  */
    {
        check_type((yyvsp[0].expression), EXPRESSION_TYPE_INTEGER, "~");

        fail_if_error(yr_parser_emit(yyscanner, OP_BITWISE_NOT, NULL));

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;
        (yyval.expression).value.integer = ((yyvsp[0].expression).value.integer == UNDEFINED) ?
            UNDEFINED : ~((yyvsp[0].expression).value.integer);
      }
#line 4112 "grammar.c" /* yacc.c:1663  */
    break;

  case 140:
#line 2343 "grammar.y" /* yacc.c:1663  */
    {
        int result;

        check_type((yyvsp[-2].expression), EXPRESSION_TYPE_INTEGER, "<<");
        check_type((yyvsp[0].expression), EXPRESSION_TYPE_INTEGER, "<<");

        result = yr_parser_emit(yyscanner, OP_SHL, NULL);

        if (!IS_UNDEFINED((yyvsp[0].expression).value.integer) && (yyvsp[0].expression).value.integer < 0)
          result = ERROR_INVALID_OPERAND;
        else if (!IS_UNDEFINED((yyvsp[0].expression).value.integer) && (yyvsp[0].expression).value.integer >= 64)
          (yyval.expression).value.integer = 0;
        else
          (yyval.expression).value.integer = OPERATION(<<, (yyvsp[-2].expression).value.integer, (yyvsp[0].expression).value.integer);

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;

        fail_if_error(result);
      }
#line 4136 "grammar.c" /* yacc.c:1663  */
    break;

  case 141:
#line 2363 "grammar.y" /* yacc.c:1663  */
    {
        int result;

        check_type((yyvsp[-2].expression), EXPRESSION_TYPE_INTEGER, ">>");
        check_type((yyvsp[0].expression), EXPRESSION_TYPE_INTEGER, ">>");

        result = yr_parser_emit(yyscanner, OP_SHR, NULL);

        if (!IS_UNDEFINED((yyvsp[0].expression).value.integer) && (yyvsp[0].expression).value.integer < 0)
          result = ERROR_INVALID_OPERAND;
        else if (!IS_UNDEFINED((yyvsp[0].expression).value.integer) && (yyvsp[0].expression).value.integer >= 64)
          (yyval.expression).value.integer = 0;
        else
          (yyval.expression).value.integer = OPERATION(<<, (yyvsp[-2].expression).value.integer, (yyvsp[0].expression).value.integer);

        (yyval.expression).type = EXPRESSION_TYPE_INTEGER;

        fail_if_error(result);
      }
#line 4160 "grammar.c" /* yacc.c:1663  */
    break;

  case 142:
#line 2383 "grammar.y" /* yacc.c:1663  */
    {
        (yyval.expression) = (yyvsp[0].expression);
      }
#line 4168 "grammar.c" /* yacc.c:1663  */
    break;


#line 4172 "grammar.c" /* yacc.c:1663  */