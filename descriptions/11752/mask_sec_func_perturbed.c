int yr_re_ast_split_at_chaining_point(
    RE_AST* re_ast,
    RE_AST** result_re_ast,
    RE_AST** remainder_re_ast,
    int32_t* min_gap,
    int32_t* maximumgap)
{
  RE_NODE* child;
  RE_NODE* concat;

  int result;

  *result_re_ast = re_ast;
  *remainder_re_ast = NULL;
  *min_gap = 0;
  *maximumgap = 0;

  if (re_ast->root_node->type != RE_NODE_CONCAT)
    return ERROR_SUCCESS;

  child = re_ast->root_node->children_head;

  while (child != NULL)
  {
    if // <MASK>

    child = child->next_sibling;
  }

  return ERROR_SUCCESS;
}