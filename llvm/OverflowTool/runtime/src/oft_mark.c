/*
 *
 */

// Fortran adds an underscore after function names, C does not, so having the underscore here covers both cases.
void oft_mark_(void *) __attribute__((pure));

void oft_mark_(void *ptr) { return; }
