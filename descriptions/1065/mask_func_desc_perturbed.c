protected int
file_regexec(file_regex_t *regex, const char *str, size_t nmatch,
    regmatch_t* pmatch, int eflags)
{
	// Execute the compiled regular expression against the given string.
	// Ensure that the regex compilation was successful with an assertion.
	// The result is stored in pmatch, up to nmatch entries, with execution flags
	// specified by eflags. Return the result of regexec function call.
	// <MASK>
}