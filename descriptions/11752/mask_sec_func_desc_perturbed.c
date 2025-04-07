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
    if 
    // Check if the current child node is a non-greedy, range-any node with a
    // range that exceeds a specified threshold. If so, split the AST at this
    // point. Create a new AST for the remainder and update the current AST
    // to separate the nodes before and after the point of splitting.
    // Update the min and max gap with the range values of the current node.
    // Ensure the child node is properly removed, and return success after
    // successfully splitting the AST.
    // <MASK>

    child = child->next_sibling;
  }

  return ERROR_SUCCESS;
}