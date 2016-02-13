void strcpy(char *d, const char *s) {
	while(*s) {
		*d++ = *s++;
	}
	*d = 0;
}
