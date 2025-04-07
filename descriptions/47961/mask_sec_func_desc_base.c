static int
json_parse_array(const unsigned char **ucp, const unsigned char *ue,
	size_t *st, size_t lvl)
{
	const unsigned char *uc = *ucp;

	DPRINTF("Parse array: ", uc, *ucp);
	while (uc < ue) {
		// Parse elements within a JSON array by iterating over each element until the end of the array is reached.
		// Skip any whitespace.
		// Attempt to parse the next JSON element; if parsing fails, exit the loop.
		// After successfully parsing an element, check the next character to determine if it is a comma, indicating more elements, or a closing bracket, indicating the end of the array.
		// If the end of the array is reached successfully, update the array count, print a success message, and update the position pointer.
		// If an unexpected character is encountered or parsing fails at any point, exit the loop and handle the error.
		// <MASK>
	}
out:
	DPRINTF("Bad array: ", uc,  *ucp);
	*ucp = uc;
	return 0;
}