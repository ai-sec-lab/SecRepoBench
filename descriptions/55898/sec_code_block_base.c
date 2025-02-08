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