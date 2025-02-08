SIZED_STRING* yr_re_ast_extract_literal(
    RE_AST* re_ast)
{
  SIZED_STRING* string;
  RE_NODE* child;

  int size = 0;

  if (re_ast->root_node->type == RE_NODE_LITERAL)
  {
    size = 1;
  }
  else if (re_ast->root_node->type == RE_NODE_CONCAT)
  {
    child = re_ast->root_node->children_tail;

    while (child != NULL && child->type == RE_NODE_LITERAL)
    {
      size++;
      child = child->prev_sibling;
    }

    if (child != NULL)
      return NULL;
  }
  else
  {
    return NULL;
  }

  string = (SIZED_STRING*) yr_malloc(sizeof(SIZED_STRING) + size);

  if (string == NULL)
    return NULL;

  string->length = size;
  string->flags = 0;

  if (re_ast->root_node->type == RE_NODE_LITERAL)
  {
    string->c_string[0] = re_ast->root_node->value;
  }
  else
  {
    child = re_ast->root_node->children_tail;

    while (child != NULL)
    {
      string->c_string[--size] = child->value;
      child = child->prev_sibling;
    }
  }

  string->c_string[string->length] = '\0';

  return string;
}