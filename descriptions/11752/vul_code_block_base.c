(!child->greedy &&
         child->type == RE_NODE_RANGE_ANY &&
        (child->start > YR_STRING_CHAINING_THRESHOLD ||
         child->end > YR_STRING_CHAINING_THRESHOLD))
    {
      result = yr_re_ast_create(remainder_re_ast);

      if (result != ERROR_SUCCESS)
        return result;

      concat = yr_re_node_create(RE_NODE_CONCAT);

      if (concat == NULL)
        return ERROR_INSUFFICIENT_MEMORY;

      concat->children_head = re_ast->root_node->children_head;
      concat->children_tail = child->prev_sibling;

      re_ast->root_node->children_head = child->next_sibling;

      if (child->prev_sibling != NULL)
        child->prev_sibling->next_sibling = NULL;

      if (child->next_sibling != NULL)
        child->next_sibling->prev_sibling = NULL;

      *min_gap = child->start;
      *max_gap = child->end;

      (*result_re_ast)->root_node = re_ast->root_node;
      (*result_re_ast)->flags = re_ast->flags;
      (*remainder_re_ast)->root_node = concat;
      (*remainder_re_ast)->flags = re_ast->flags;

      yr_re_node_destroy(child);

      return ERROR_SUCCESS;
    }