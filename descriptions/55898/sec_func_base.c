static int
sampled_data_continue(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    gs_sampled_data_enum *penum = senum;
    gs_function_Sd_params_t * params =
            (gs_function_Sd_params_t *)&penum->pfn->params;
    int i, j, num_out = params->n;
    int code = 0;
    byte * data_ptr;
    double sampled_data_value_max = (double)((1 << params->BitsPerSample) - 1);
    int bps = bits2bytes(params->BitsPerSample), stack_depth_adjust = 0;

    /*
     * Check to make sure that the procedure produced the correct number of
     * values.  If not, move the stack back to where it belongs and abort
     */
    if (num_out + O_STACK_PAD + penum->o_stack_depth != ref_stack_count(&o_stack)) {
        stack_depth_adjust = ref_stack_count(&o_stack) - penum->o_stack_depth;

        if (stack_depth_adjust < 0) {
            /*
             * If we get to here then there were major problems.  The function
             * removed too many items off of the stack.  We had placed extra
             * (unused) stack stack space to allow for this but the function
             * exceeded even that.  Data on the stack may have been lost.
             * The only thing that we can do is move the stack pointer back and
             * hope.
             */
            push(-stack_depth_adjust);
            esp -= estack_storage;
            return_error(gs_error_undefinedresult);
        }
    }

    /* Save data from the given function */
    data_ptr = cube_ptr_from_index(params, penum->indexes);
    for (i=0; i < num_out; i++) {
        ulong cv;
        double value;
        double rmin = params->Range[2 * i];
        double rmax = params->Range[2 * i + 1];

        code = real_param(op + i - num_out + 1, &value);
        if (code < 0) {
            esp -= estack_storage;
            return code;
        }
        if (value < rmin)
            value = rmin;
        else if (value > rmax)
            value = rmax;
        value = (value - rmin) / (rmax - rmin);		/* Convert to 0 to 1.0 */
        cv = (int) (value * sampled_data_value_max + 0.5);
        for (j = 0; j < bps; j++)
            data_ptr[bps * i + j] = (byte)(cv >> ((bps - 1 - j) * 8));	/* MSB first */
    }

    pop(num_out); /* Move op to base of result values */

    /* From here on, we have to use ref_stack_pop() rather than pop()
       so that it handles stack extension blocks properly, before calling
       sampled_data_sample() which also uses the op stack.
     */
    /* Check if we are done collecting data. */
    if (increment_cube_indexes(params, penum->indexes)) {
        if (stack_depth_adjust == 0)
            ref_stack_pop(&o_stack, O_STACK_PAD);	    /* Remove spare stack space */
        else
            ref_stack_pop(&o_stack, stack_depth_adjust - num_out);
        /* Execute the closing procedure, if given */
        code = 0;
        if (esp_finish_proc != 0)
            code = esp_finish_proc(i_ctx_p);

        return code;
    } else {
        if (stack_depth_adjust) {
            stack_depth_adjust -= num_out;
            if ((O_STACK_PAD - stack_depth_adjust) < 0) {
                stack_depth_adjust = -(O_STACK_PAD - stack_depth_adjust);
                check_op(stack_depth_adjust);
                ref_stack_pop(&o_stack, stack_depth_adjust);
            }
            else {
                check_ostack(O_STACK_PAD - stack_depth_adjust);
                ref_stack_push(&o_stack, O_STACK_PAD - stack_depth_adjust);
                for (i=0;i<O_STACK_PAD - stack_depth_adjust;i++)
                    make_null(op - i);
            }
        }
    }

    /* Now get the data for the next location */

    return sampled_data_sample(i_ctx_p);
}