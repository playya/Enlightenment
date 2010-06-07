enum
{
	UTFmax        = 4,            /* maximum bytes per rune */
	Runesync      = 0x80,         /* cannot represent part of a UTF sequence (<) */
	Runeself      = 0x80,         /* rune and UTF sequences are the same (<) */
	Runeerror     = 0xFFFD,       /* decoding error in UTF */
	Runemax       = 0x10FFFF,     /* maximum rune value */
};

enum
{
	Bit1    = 7,
	Bitx    = 6,
	Bit2    = 5,
	Bit3    = 4,
	Bit4    = 3,
	Bit5    = 2,

	T1      = ((1<<(Bit1+1))-1) ^ 0xFF,     /* 0000 0000 */
	Tx      = ((1<<(Bitx+1))-1) ^ 0xFF,     /* 1000 0000 */
	T2      = ((1<<(Bit2+1))-1) ^ 0xFF,     /* 1100 0000 */
	T3      = ((1<<(Bit3+1))-1) ^ 0xFF,     /* 1110 0000 */
	T4      = ((1<<(Bit4+1))-1) ^ 0xFF,     /* 1111 0000 */
	T5      = ((1<<(Bit5+1))-1) ^ 0xFF,     /* 1111 1000 */

	Rune1   = (1<<(Bit1+0*Bitx))-1,         /* 0000 0000 0111 1111 */
	Rune2   = (1<<(Bit2+1*Bitx))-1,         /* 0000 0111 1111 1111 */
	Rune3   = (1<<(Bit3+2*Bitx))-1,         /* 1111 1111 1111 1111 */
	Rune4   = (1<<(Bit4+3*Bitx))-1,
	/* 0001 1111 1111 1111 1111 1111 */

	Maskx   = (1<<Bitx)-1,                  /* 0011 1111 */
	Testx   = Maskx ^ 0xFF,                 /* 1100 0000 */

	Bad     = Runeerror,
};

int
chartorune(int *rune, char *str)
{
	int c, c1, c2, c3;
	long l;

	/*
	 * one character sequence
	 *      00000-0007F => T1
	 */
	c = *(unsigned char*)str;
	if(c < Tx) {
		*rune = c;
		return 1;
	}

	/*
	 * two character sequence
	 *      0080-07FF => T2 Tx
	 */
	c1 = *(unsigned char*)(str+1) ^ Tx;
	if(c1 & Testx)
		goto bad;
	if(c < T3) {
		if(c < T2)
			goto bad;
		l = ((c << Bitx) | c1) & Rune2;
		if(l <= Rune1)
			goto bad;
		*rune = l;
		return 2;
	}

	/*
	 * three character sequence
	 *      0800-FFFF => T3 Tx Tx
	 */
	c2 = *(unsigned char*)(str+2) ^ Tx;
	if(c2 & Testx)
		goto bad;
	if(c < T4) {
		l = ((((c << Bitx) | c1) << Bitx) | c2) & Rune3;
		if(l <= Rune2)
			goto bad;
		*rune = l;
		return 3;
	}

	/*
	 * four character sequence (21-bit value)
	 *      10000-1FFFFF => T4 Tx Tx Tx
	 */
	c3 = *(unsigned char*)(str+3) ^ Tx;
	if (c3 & Testx)
		goto bad;
	if (c < T5) {
		l = ((((((c << Bitx) | c1) << Bitx) | c2) << Bitx) | c3) & Rune4;
		if (l <= Rune3)
			goto bad;
		*rune = l;
		return 4;
	}
	/*
	 * Support for 5-byte or longer UTF-8 would go here, but
	 * since we don't have that, we'll just fall through to bad.
	 */

	/*
	 * bad decoding
	 */
bad:
	*rune = Bad;
	return 1;
}


int
runetochar(char *str, int *rune)
{
	/* Runes are signed, so convert to unsigned for range check. */
	unsigned long c;

	/*
	 * one character sequence
	 *      00000-0007F => 00-7F
	 */
	c = *rune;
	if(c <= Rune1) {
		str[0] = c;
		return 1;
	}

	/*
	 * two character sequence
	 *      0080-07FF => T2 Tx
	 */
	if(c <= Rune2) {
		str[0] = T2 | (c >> 1*Bitx);
		str[1] = Tx | (c & Maskx);
		return 2;
	}

	/*
	 * If the Rune is out of range, convert it to the error rune.
	 * Do this test here because the error rune encodes to three bytes.
	 * Doing it earlier would duplicate work, since an out of range
	 * Rune wouldn't have fit in one or two bytes.
	 */
	if (c > Runemax)
		c = Runeerror;

	/*
	 * three character sequence
	 *      0800-FFFF => T3 Tx Tx
	 */
	if (c <= Rune3) {
		str[0] = T3 |  (c >> 2*Bitx);
		str[1] = Tx | ((c >> 1*Bitx) & Maskx);
		str[2] = Tx |  (c & Maskx);
		return 3;
	}

	/*
	 * four character sequence (21-bit value)
	 *     10000-1FFFFF => T4 Tx Tx Tx
	 */
	str[0] = T4 | (c >> 3*Bitx);
	str[1] = Tx | ((c >> 2*Bitx) & Maskx);
	str[2] = Tx | ((c >> 1*Bitx) & Maskx);
	str[3] = Tx | (c & Maskx);
	return 4;
}


int
runelen(int c)
{
	char str[10];
	return runetochar(str, &c);
}

int
runenlen(int *r, int nrune)
{
	int nb, c;

	nb = 0;
	while (nrune--) {
		c = *r++;
		if (c <= Rune1)
			nb++;
		else if (c <= Rune2)
			nb += 2;
		else if (c <= Rune3)
			nb += 3;
		else /* assert(c <= Rune4) */
			nb += 4;
	}
	return nb;
}

int
fullrune(char *str, int n)
{
	if (n > 0) {
		int c = *(unsigned char*)str;
		if (c < Tx)
			return 1;
		if (n > 1) {
			if (c < T3)
				return 1;
			if (n > 2) {
				if (c < T4 || n > 3)
					return 1;
			}
		}
	}
	return 0;
}

