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

  string->c_string[size] = '\0';

  return string;