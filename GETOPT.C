/*LINTLIBRARY*/

/***********************************************************************
	getopt.c  ‚±‚Ìgetopt.c‚ÍPublic Domain‚Å‚·
***********************************************************************/

#include <stdio.h>
#include <string.h>

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

static	void	errdisp(cmd, as)
char	*cmd, *as;
{
#ifdef LSI_C
	static	char	crtail[ ] = {0, '\r', '\n', 0};
#else
#  define _puterrmes(s) fputs((s), stderr)
	static	char	crtail[ ] = {0, '\n', 0};
#endif

	if(opterr){
		_puterrmes(cmd), _puterrmes(as),
		*crtail = optopt, _puterrmes(crtail);
	}
}

int	getopt(ac, av, opts)
int	ac;
char	**av, *opts;
{
	static	char	*curopt = NULL;
	register char	*cp;

	if(curopt == NULL || !*curopt){
		if(optind >= ac || *(curopt = av[optind]) != '-' || !curopt[1])
			return EOF;
		if(!strcmp("-", ++curopt))
			return (optind++, EOF);
	}
	if(':' == (optopt = *curopt++) || NULL == (cp = strchr(opts, optopt))){
		errdisp(*av, ": unknown option, -");
		if(!*curopt) optind++;
		return '?';
	}
	if(*++cp == ':'){
		optind++;
		if(*curopt){
			optarg = curopt;
			curopt = NULL;
		} else {
			if(optind >= ac){
				errdisp(*av, ": argument missing for -");
				return '?';
			} else {
				optarg = av[optind++];
			}
			 /* now *curopt == '\0' */
		}
	} else {
		optarg = NULL;
		if(!*curopt) optind++;
	}
	return optopt;
}

#ifdef DEBUG
main(ac, av)
int	ac;
char	**av;
{
	int	c;

	opterr = 1;
	while(EOF != (c = getopt(ac, av, "ab:cd:"))){
		printf("ret %c, optopt %c, optarg %s, optind %d\n", c,
			optopt, optarg == NULL ? "(NULL)" : optarg, optind);
	}
	for(ac -= optind, av += optind; ac; ac--, av++)
		printf("arg: '%s'\n", *av);
}
#endif

