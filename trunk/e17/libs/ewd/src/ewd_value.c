/* ewd_value.c

Copyright (C) 2001 Christopher Rosendahl    <smugg@fatelabs.com>
                   Nathan Ingersoll         <ningerso@d.umn.edu>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <Ewd.h>

const unsigned int ewd_prime_table[] = { 17, 31, 61, 127, 257, 509, 1021,
	2053, 4093, 8191, 16381, 32771, 65537, 131071, 262147, 524287, 1048573,
	2097143, 4194301, 8388617, 16777213 
};

/**
 * ewd_direct_hash - just casts the key to an unsigned int
 * @key: the key to return compute a hash value
 *
 * Returns the key cast to an unsigned int.
 */
unsigned int ewd_direct_hash(void *key)
{
	return ((unsigned int) key);
}

/**
 * ewd_str_hash - compute the hash value of a string
 * @key: a pointer to the string to compute a hash value
 *
 * Returns a computed hash value for @key.
 */
unsigned int ewd_str_hash(void *key)
{
	int i;
	unsigned int value = 0;
	char *k = (char *) key;

	if (!k)
		return 0;

	for (i = 0; k[i] != '\0'; i++) {
		value ^= ((unsigned int) k[i] << ((i * 5) %
					(sizeof(unsigned int) * 8)));
	}

	if (!strcmp(k, "abash"))
		fprintf(stderr, "abash hashed to %u\n", value);

	return value;
}

/**
 * ewd_direct_compare - perform a direct comparison of two keys values
 * @key1: the first key to compare
 * @key2: the second key to compare
 *
 * Return a strcmp style value to indicate the larger key
 */
int ewd_direct_compare(void *key1, void *key2)
{
	unsigned int k1, k2;

	k1 = (unsigned int) key1;
	k2 = (unsigned int) key2;

	if (k1 > k2)
		return 1;

	if (k1 < k2)
		return -1;

	return 0;
}

/**
 * ewd_direct_compare - perform a string comparison of two keys values
 * @key1: the first key to compare
 * @key2: the second key to compare
 *
 * Return a strcmp style value to indicate the larger key
 */
int ewd_str_compare(void *key1, void *key2)
{
	char *k1, *k2;

	if (!key1 || !key2)
		return ewd_direct_compare(key1, key2);
	else if (key1 == key2)
		return 0;

	k1 = (char *) key1;
	k2 = (char *) key2;

	return strcmp(k1, k2);
}
