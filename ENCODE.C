#include "common.h"

static	int	header_out(), putenc1(), putenc2(), putcfill();
static	void	notcom2t();

static	int	pos, length;

void	encode(inf, inchar1, inchar2, otf, len, comshare)
FILE	*inf;
register FILE	*otf;
int	len, inchar1, inchar2;
char	comshare; /* boolean */
{
	register int	c;
	register char	i = 3, j = 2;
	int	t = 0;
	long	osize;
	int	(*putenc)();

	putenc = comshare ? putenc2 : putenc1;
	osize = (long)header_out(otf, comshare);
	length = len;
	if(pos >= length) (putc('\n', otf), osize += 2, pos = 0);

	while(c = j ? (--j ? inchar1 : inchar2) : getc(inf), c != EOF){
		switch(--i){
		case 0:
			osize += (long)putenc((c >> 6) | t, otf);
			osize += (long)putenc(c, otf);
			i = 3;
			break;
		case 1:
			osize += (long)putenc((c >> 4) | t, otf);
			t = c << 2;
			break;
		default:
			osize += (long)putenc(c >> 2, otf);
			t = c << 4;
			break;
		}
	}
	if(i != 3) osize += (long)putenc(t, otf);

	if(comshare) (putc('\n', otf), osize += 2, pos = 0);
	osize += (long)putcfill(':', otf);
	if(pos) (putc('\n', otf), osize += 2);
#ifdef DEBUG
	fprintf(stderr, "output %ld bytes\n", osize);
#endif
	if(osize > MAXOSIZE){
		fputs("Warning: output is too big, so it may works incorrectly\n",
		      stderr);
	}
}

static	int	putenc2(c, otf)
register int	c;
register FILE	*otf;
{
	c &= 0x3f;
	if((c += 0x40) == 0x7f) c = 0x3f;
	
	return putcfill(c, otf);
}

static	int	putenc1(c, otf)
register int	c;
register FILE	*otf;
{
	c &= 0x3f, c += 0x40;
	if('Z' < c && c < 'a' && c != '_') c = '.' + '`' - c;
	else
	if('z' < c || c < 'A') c = ('4' + '@' - c) & 0x3f;

	return putcfill(c, otf);
}

static	int	putcfill(c, otf)
register int	c;
register FILE	*otf;
{
	putc(c, otf);
	if(++pos >= length){
		putc('\n', otf), pos = 0;
		return 3; /* 1 for c, 2 for "\r\n" */
	} else return 1;
}

static	void	ldout(l, f)
unsigned long	l;
FILE	*f; /* output long integer to file */
{
	int	c;

	c = (int)(l % 10L), l /= 10L;
	if(l) ldout(l, f);
	putc(c + '0', f);
}

static	int	header_out(otf, comshare)
FILE	*otf;
char	comshare; /* boolean */
{
	extern	char	*comtdat1[], *comtdat2[];
	register char	**pp, *p;
	int	retsize = 0, l;

	for(pp = comshare ? comtdat2 : comtdat1; ; pp++){
		if(comshare && pp[1] == NULL){
			fputs("  --- Insert lines here. File size must be <=",
			      otf);
			ldout(MAXOSIZE, otf);
			fputs(" bytes.\n", otf);
		}
		fputs(p = *pp, otf);
		retsize += (l = strlen(p));
		if(l && p[l - 1] == '\n') retsize++;
		 /* '\n'はデータの最後にしか来ないことを前提とする */
		if(pp[1] == NULL){
			pos = l;
			return retsize;
		}
	}
}

void	decode(inf, otf)
FILE	*inf;
register FILE	*otf;
 /*
  前提とするフォーマット:
     comshareモードでない場合
	ヘッダ
		1行目 ("TXPPP" 又は "T_OOWW3=") + テキストデータ
		  ("TXPPP" は旧バージョンのもの)
		2行目 長さQSTRLENのテキストデータ + テキストデータ
		ヘッダの最後 2行目の先頭QSTRLENバイトより後で最初に
			     "__" が出るところ
	encodeされたデータ
		テキスト (':'で終わり 旧バージョンは'='で終わり)
     comshareモードの場合
	ヘッダ
		1行目 ":=%%%%" + テキストデータ
		2行目 長さQSTRLENのテキストデータ + テキストデータ
		ヘッダの最後 2行目の先頭QSTRLENバイトより後で最初に "!!"
			     が出るところ(そこまでの各行は ":" で始まる)
			     + 改行 + 任意のデータ + 改行 + "*"
	encodeされたデータ
		テキスト (':'で終わり)
     いずれの場合もタブは含まれていないこと
 */
{
#define SEE1 8
#define SEE2 8 /* とりあえず8のままにしとこう */

	extern	char	*comtdat1[], *comtdat2[];
	static	char	oldh[] = {'T','X','P','P','P'};
	char	buf[SEE1 > SEE2 ? SEE1 : SEE2]; /* 先頭1文字は別途getchar */
	register int	c, t = 0;
	char	comshare = 0 /* boolean */, i;
	int	l, prev;

	c = getc(inf);
	if(c == *comtdat1[0] || c == oldh[0]){
		if(NULL == fgets(buf, SEE1, inf) ||
		   strncmp(*comtdat1 + 1, buf, SEE1 - 1) &&
		   strncmp(oldh + 1, buf, sizeof(oldh) - 1)){
			notcom2t();
		}
	} else
	if(c == *comtdat2[0]){
		comshare = 1;
		if(NULL == fgets(buf, SEE2, inf) ||
		   strncmp(*comtdat2 + 1, buf, SEE2 - 1))
			notcom2t();
	} else notcom2t();

	while('\n' != (c = getc(inf)))
		if(!is_print(c) && c != '\r') notcom2t();
	for(l = QSTRLEN; c = getc(inf), l--; ) if(!is_print(c)) notcom2t();

	prev = '\0';
	while((c = getc(inf)) != (comshare ? '#' : '_') || prev != c){
		if(!is_print(c) && c != '\r' && c != '\n' ||
		   comshare && prev == '\n' && c != ':')
			notcom2t();
		prev = c;
	}
	if(comshare){
		while('\n' != (c = getc(inf)))
			if(c != '\r') notcom2t();
		for(;;){
			if('*' == (c = getc(inf))) break;
			while(c != '\n'){
				if(EOF == (c = getc(inf))) notcom2t();
			}
		}
	}

	i = 4;
	while(':' != (c = getc(inf)) && c != '='){
		if(c == '\n' || c == '\r') continue;
		if(!is_print(c)) notcom2t();

		if(!comshare){
			if(c <= '3') c = '`' + '.' - c;
			else if(c <= '9') c = '4' - c;
		}
		c &= 0x3f;

		switch(--i){
		case 0:
			i = 4;
			putc(t | c, otf); break;
		case 1:
			putc(t | (c >> 2), otf); t = c << 6; break;
		case 2:
			putc(t | (c >> 4), otf); t = c << 4; break;
		default:
			t = c << 2; break;
		}
	}
#if 0 /* -cの逆変換を考慮してやめた */
	while('\n' == (c = getc(inf)) || '\r' == c);
	if(c != EOF) fputs("Warning: trailing garbage is ignored.\n", stderr);
#endif
}

static	void	notcom2t()
{
	fputs("Input is not seems to be textized by com2txt.\n", stderr);
	exit(1);
}

#ifndef UNIX
void	tscopy(sf, df)
FILE	*sf, *df;
{
	union	REGS	in, out;

	in . x . ax = 0x5700;
	in . x . bx = fileno(sf);
	intdos(&in, &out);
	if(out . x . cflag) return;
	out . x . ax = 0x5701;
	out . x . bx = fileno(df);
	intdos(&out, &in);
}
#endif
