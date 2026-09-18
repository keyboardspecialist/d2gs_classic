#include <windows.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "d2gelib/d2server.h"

#define SPLIT_STRING_INIT_COUNT		32
#define	SPLIT_STRING_INCREASEMENT	32
extern char * * strtoarray(char const * str, char const * delim, int * count)
{
	int	i, n;
	size_t	index_size;
	int	in_delim, match;
	char	* temp, * result;
	size_t	* pindex;
	char	* pd;
	char	const * ps;
	size_t	* realloc_tmp;

	if (!str || !delim || !count) return NULL;

	temp=malloc(strlen(str)+1);
	if (!temp) return NULL;

	n = SPLIT_STRING_INIT_COUNT;
	pindex=malloc(sizeof(*pindex) * n);
	if (!pindex) {
		free(temp);
		return NULL;
	}

	*count=0;
	in_delim=1;
	ps=str;
	pd=temp;
	pindex[0]=0;
	while (*ps!='\0') {
		match=0;
		for (i=0; delim[i]!='\0'; i++) {
			if ( *ps == delim[i]) {
				match=1;
				if (!in_delim) {
					*pd = '\0';
					pd++;
					(*count)++;
					in_delim=1;
				}
				break;
			}
		}
		if (!match) {
			if (in_delim) {
				if (*count>=n) {
					n += SPLIT_STRING_INCREASEMENT;
					if (!(realloc_tmp=realloc(pindex, n * sizeof(*pindex)))) {
						free(pindex);
						free(temp);
						return NULL;
					}
					pindex=realloc_tmp;
				}
				pindex[*count]= pd-temp;
				in_delim = 0;
			}
			*pd = * ps;
			pd++;
		}
		ps++;
	}
	if (!in_delim) {
		*pd='\0';
		pd++;
		(*count)++;
	}
	index_size=*count * sizeof(char *);
	if (!index_size) {
		free(temp);
		free(pindex);
		return NULL;
	}
	result=malloc(pd-temp+index_size);
	if (!result) {
		free(temp);
		free(pindex);
		return NULL;
	}
	memcpy(result+index_size,temp,pd-temp);
	for (i=0; i< *count; i++)
		((char **)result)[i] = result + index_size + pindex[i];
	free(temp);
	free(pindex);
	return (char **) result;
}


extern char * * strtoargv(char const * str, int * count)
{
	unsigned int	n;
	size_t		index_size;
	char		* temp;
	int		i, j;
	size_t		* pindex;
	char		* result;
	size_t		* realloc_tmp;

	if (!str || !count) return NULL;
	temp=malloc(strlen(str)+1);
	if (!temp) return NULL;
	n = SPLIT_STRING_INIT_COUNT;
	pindex=malloc(n * sizeof(*pindex));
	if (!pindex) {
		free(temp);
		return NULL;
	}

	i=j=0;
	*count=0;
	while (str[i]) {
		while (str[i]==' ' || str[i]=='\t') i++;
		if (!str[i]) break;
		if ((unsigned int)(*count) >=n ) {
			n += SPLIT_STRING_INCREASEMENT;
			if (!(realloc_tmp=realloc(pindex, n * sizeof(*pindex)))) {
				free(pindex);
				free(temp);
				return NULL;
			}
			pindex=realloc_tmp;
		}
		pindex[*count]=j;
		(*count)++;
		if (str[i]=='"') {
			i++;
			while (str[i]) {
				if (str[i]=='\\') {
					i++;
					if (!str[i]) break;
				} else if (str[i]=='"') {
					i++;
					break;
				}
				temp[j++]=str[i++];
			}
		} else {
			while (str[i] && str[i] != ' ' && str[i] != '\t') {
				temp[j++]=str[i++];
			}
		}
		temp[j++]='\0';
	}
	index_size= *count * sizeof(char *);
	if (!index_size) {
		free(temp);
		free(pindex);
		return NULL;
	}
	result=malloc(j+index_size);
	if (!result) {
		free(temp);
		free(pindex);
		return NULL;
	}
	memcpy(result+index_size,temp,j);
	for (i=0; i< *count; i++)
		((char **)result)[i] = result + index_size + pindex[i];
	free(temp);
	free(pindex);
	return (char * *)result;
}


extern char * str_strip_affix(char * str, char const * affix)
{
	size_t i, end;

	if (!str) return NULL;
	if (!affix) return str;
	end = strlen(str);
	for (i=0; i<end && strchr(affix, str[i]); i++);
	while (end>i && strchr(affix, str[end-1])) end--;
	memmove(str, str+i, end-i);
	str[end-i]='\0';
	return str;
}

typedef struct
{
	char const	* src;
	char const	* dest;
} t_replace;

static t_replace ColorHackTable[] = {
	{"%white%",		D2COLOR_WHITE},
	{"%red%",		D2COLOR_RED},
	{"%green%",		D2COLOR_GREEN},
	{"%blue%",		D2COLOR_BLUE},
	{"%golden%",	D2COLOR_GOLDEN},
	{"%grey%",		D2COLOR_GREY},
	{"%black%",		D2COLOR_BLACK},
	{"%dyellow%",	D2COLOR_DARK_YELLOW},
	{"%orange%",	D2COLOR_ORANGE},
	{"%yellow%",	D2COLOR_YELLOW},
	{"%dgreen%",	D2COLOR_DARK_GREEN},
	{"%purple%",	D2COLOR_PURPLE},
	{"%ldgreen%",	D2COLOR_LIGHT_DARK_GREEN},
	{ NULL,			NULL}
};


extern char *string_color(char *str)
{
	char *src, *dest;
	t_replace	*tr;

	if (!str) return NULL;
	src = dest = str;
	while(*src!='\0')
	{
		if (*src!='%') {
			*dest++ = *src++;
			continue;
		}
		tr = ColorHackTable;
		while(tr->src!=NULL)
		{
			if (strnicmp(src, tr->src, strlen(tr->src))==0) {
				memcpy(dest, tr->dest, strlen(tr->dest));
				src += strlen(tr->src);
				dest += strlen(tr->dest);
				continue;
			}
			tr++;
		}
		*dest++ = *src++;
	}
	*dest = '\0';
	return str;
}
