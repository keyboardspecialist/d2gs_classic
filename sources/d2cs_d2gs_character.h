#ifndef INCLUDED_D2CS_D2GS_CHARACTER_H
#define INCLUDED_D2CS_D2GS_CHARACTER_H

#ifdef D2CS
# include "common/bn_type.h"
#endif

#ifdef D2GS
# include "bn_types.h"
#pragma pack(push, pack01, 1)
#endif 

#define D2CHARINFO_MAGICWORD		0x12345678
#define D2CHARINFO_VERSION			0x00010000
#define D2CHARINFO_PORTRAIT_PADSIZE	30
#define D2CHARINFO_PORTRAIT_MODERN_SIZE 34
#define D2CHARINFO_PORTRAIT_LEGACY_BASE_SIZE 44
#define D2CHARINFO_PORTRAIT_STORAGE_SIZE 64

typedef struct
{
	bn_int		magicword;		/* static for check */
	bn_int		version;		/* charinfo file version */
	bn_int		create_time;	/* character creation time */
	bn_int		last_time;		/* character last access time */
	bn_int		checksum;
	bn_int		total_play_time;
	bn_int		reserved[6];
	unsigned char	charname[MAX_CHARNAME_LEN];
	unsigned char	account[MAX_ACCTNAME_LEN];
	unsigned char	realmname[MAX_REALMNAME_LEN];
} t_d2charinfo_header;

typedef struct
{
	bn_int		experience;
	bn_int		charstatus;
	bn_int		charlevel;
	bn_int		charclass;
} t_d2charinfo_summary;

typedef struct
{
        bn_short        header;	/* 0x84 0x80 */
        bn_byte         gfx[11];
        bn_byte         chclass;
        bn_byte         color[11];
        bn_byte         level;
        bn_byte         status;
        bn_byte         u1[3];
        bn_byte         ladder;
        bn_byte         u2[2];
		bn_byte         end;	/* 0x00 */
		bn_byte         legacy_tail[D2CHARINFO_PORTRAIT_PADSIZE];
} t_d2charinfo_portrait;

typedef struct
{
	t_d2charinfo_header		header;
	t_d2charinfo_portrait	portrait;
	t_d2charinfo_summary	summary;
} t_d2charinfo_file;

_Static_assert(sizeof(t_d2charinfo_header) == 112, "Character-info header ABI changed");
_Static_assert(sizeof(t_d2charinfo_portrait) == 64, "Character portrait ABI changed");
_Static_assert(sizeof(t_d2charinfo_file) == 192, "Character-info file ABI changed");

#ifdef D2GS
#pragma pack(pop, pack01)
#endif

#endif
