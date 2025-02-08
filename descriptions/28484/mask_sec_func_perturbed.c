static void cil_post_fc_fill_data(struct fc_data *fc, const char *filepath)
{
	size_t c = 0;
	fc->meta = 0;
	fc->stem_len = 0;
	fc->str_len = 0;
	
	while (filepath[c] != '\0') {
		switch (filepath[c]) {
		case '.':
		case '^':
		case '$':
		case '?':
		case '*':
		case '+':
		case '|':
		case '[':
		case '(':
		case '{':
			fc->meta = 1;
			break;
		// <MASK>
			/* FALLTHRU */
		default:
			if (!fc->meta) {
				fc->stem_len++;
			}
			break;
		}
		fc->str_len++;
		c++;
	}
}