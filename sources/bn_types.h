#ifndef INCLUDED_BN_TYPES_H
#define INCLUDED_BN_TYPES_H

#include <stdint.h>

/* basic bn types */
typedef uint8_t		bn_basic;
typedef uint8_t		bn_char;
typedef uint8_t		bn_byte;
typedef uint16_t	bn_short;
typedef uint32_t	bn_int;
typedef uint32_t	bn_long;


/* use network order or not? */
#ifdef USE_NBO
#define bn_htons(a)		(htons(a))
#define bn_ntohs(a)		(ntohs(a))
#define bn_htonl(a)		(htonl(a))
#define bn_ntohl(a)		(ntohl(a))
#else
#define bn_htons(a)		(a)
#define bn_ntohs(a)		(a)
#define bn_htonl(a)		(a)
#define bn_ntohl(a)		(a)
#endif


#endif /* INCLUDED_BN_TYPES_H */
