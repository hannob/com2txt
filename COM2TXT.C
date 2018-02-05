#include "common.h"

extern	void	encode(), decode();

static	void	usage()
{
	fputs("com2txt Ver 1.11\n", stderr);
	fputs("Usage: com2txt [-w len] [-r] orig[.com] new[.com]\n",
	      stderr);
	exit(1);
}

#define istermarg(s) (*(s) == '-' && !(s)[1])
#define conststrlen(s) (sizeof(s) - 1)

static	void	name_unixnize(s) /* sは上書きされる */
register char	*s;
{
	for(; *s; s++){
		if(iskanji(*s) && s[1]){
			s++;
			continue;
		}
		if(*s == '\\') *s = '/';
		else if(is_upper(*s)) *s = tolower(*s);
	}
}

static	FILE	*comfopen(name, mode)
register char	*name, *mode;
{
	char	buf[MAXPATHLEN + 1];
	static	char	ext[] = ".com";
	int	l;
	FILE	*retf;

	if(istermarg(name)){
		retf = (*mode == 'r' ? stdin : stdout);
#ifndef UNIX
		if(mode[1] == 'b') fsetbin(retf);
#endif
		 /* 2文字目しかチェックしていない */
		return retf;
	}
	l = strlen(name);
	if(l + conststrlen(ext) <= MAXPATHLEN &&
	   (l < conststrlen(ext) || strcmp(ext, name + l - conststrlen(ext)))){
		strcpy(buf, name), strcpy(buf + l, ext);
		if(NULL != (retf = fopen(buf, mode))) return retf;
	}
	return fopen(name, mode);
}

static	FILE	*comfopen_chk(name, mode)
register char	*name;
char	*mode;
{
	register FILE	*retf;

	if(NULL == (retf = comfopen(name, mode))){
		fputs(name, stderr), fputs(": Can't open\n", stderr);
		exit(1);
	}
	return retf;
}

void	main(ac, av)
int	ac;
register char	*av[];
{
	extern	int	optind;
	extern	char	*optarg;
	register int	c;
	char	*infnm, *otfnm;
	char	infnmbuf[MAXPATHLEN + 1], otfnmbuf[MAXPATHLEN + 1];
	int	len = DEFAULT_LEN;
	char	reverse = 0, comshare = 0; /* boolean */
	FILE	*inf, *otf;
	int	d;

	while(EOF != (c = getopt(ac, av, "w:rc"))){
		switch(c){
		case 'w':
			len = atoi(optarg); break;
		case 'r':
			reverse = 1; break;
		case 'c':
			comshare = 1; break;
		default:
			usage();
		}
	}
	if(av += optind, ac -= optind, ac != 2) usage();

	if(strlen(av[0]) <= MAXPATHLEN){
		strcpy(infnm = infnmbuf, av[0]), name_unixnize(infnm);
	} else infnm = av[0];
	if(strlen(av[1]) <= MAXPATHLEN){
		strcpy(otfnm = otfnmbuf, av[1]), name_unixnize(otfnm);
	} else otfnm = av[1];
	
#ifdef UNIX
#  define R_BIN "r"
#  define W_BIN "w"
#else
#  define R_BIN "rb"
#  define W_BIN "wb"
#endif
	if(!reverse){
		inf = comfopen_chk(infnm, R_BIN);
		if(EOF != (c = getc(inf))) d = getc(inf);
		if(c == 'M' && d == 'Z'){
			fputs("Warning: '", stderr), fputs(infnm, stderr),
			fputs("' seems to be an EXE file!\n", stderr);
		}
		otf = comfopen_chk(otfnm, "w");
		encode(inf, c, d, otf, len, comshare);
	} else {
		decode(inf = comfopen_chk(infnm, "r"),
		       otf = comfopen_chk(otfnm, W_BIN));
	}
	if(fflush(otf) || ferror(otf)){
		fputs("Write error. (Disk full?)\n", stderr);
		exit(1);
	}
#ifndef UNIX
	tscopy(inf, otf); /* fflush(otf)より後でないといけない */
#endif
	exit(0);
}
