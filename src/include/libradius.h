#ifndef LIBRADIUS_H
#define LIBRADIUS_H

/*
 * libradius.h	Structures and prototypes
 *		for the radius library.
 *
 * Version:	$Id$
 *
 */

#include "autoconf.h"

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_STDINT_H
#include <stdint.h>
#endif

#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#include <stdio.h>

/*
 *  Check for inclusion of <time.h>, versus <sys/time.h>
 *  Taken verbatim from the autoconf manual.
 */
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "radius.h"
#include "token.h"

#ifdef SIZEOF_UNSIGNED_INT
#if SIZEOF_UNSIGNED_INT != 4
#error FATAL: sizeof(unsigned int) != 4
#endif
#endif

#define EAP_START               2

#define AUTH_VECTOR_LEN		16
#define CHAP_VALUE_LENGTH       16
#define MAX_STRING_LEN		254	/* RFC2138: string 0-253 octets */

#define PW_AUTH_UDP_PORT                1645
#define PW_ACCT_UDP_PORT                1646

#ifdef _LIBRADIUS
#  define AUTH_HDR_LEN		20
#  define VENDORPEC_USR		429
#  define VENDOR(x)		(x >> 16)
#  define DEBUG			if (librad_debug) printf
#  define debug_pair(vp)	do { if (librad_debug) { \
					putchar('\t'); \
					vp_print(stdout, vp); \
					putchar('\n'); \
				     } \
				} while(0)
#endif

typedef struct dict_attr {
	char			name[40];
	int			attr;
	int			type;
	int			vendor;
	struct dict_attr	*next;
} DICT_ATTR;

typedef struct dict_value {
	char			name[40];
	char			attrname[40];
	int			attr;
	int			value;
	struct dict_value	*next;
} DICT_VALUE;

typedef struct dict_vendor {
	char			vendorname[40];
	int			vendorpec;
	struct dict_vendor	*next;
} DICT_VENDOR;

typedef struct value_pair {
	char			name[40];
	int			attribute;
	int			type;
	int			length; /* of strvalue */
	uint32_t		lvalue;
	LRAD_TOKEN		operator;
	int			addport;
	uint8_t			strvalue[MAX_STRING_LEN];
	struct value_pair	*next;
} VALUE_PAIR;

/*
 *	vector:		Request authenticator from access-request packet
 *			Put in there by rad_decode, and must be put in the
 *			response RADIUS_PACKET as well before calling rad_send
 *
 *	verified:	Filled in by rad_decode for accounting-request packets
 *
 *	data,data_len:	Used between rad_recv and rad_decode.
 */
typedef struct radius_packet {
	int			sockfd;
	uint32_t		src_ipaddr;
	uint32_t		dst_ipaddr;
	u_short			src_port;
	u_short			dst_port;
	int			id;
	int			code;
	uint8_t			vector[AUTH_VECTOR_LEN];
	time_t			timestamp;
	int			verified;
	uint8_t			*data;
	int			data_len;
	VALUE_PAIR		*vps;
} RADIUS_PACKET;

/*
 *	Printing functions.
 */
void		librad_safeprint(char *in, int inlen, char *out, int outlen);
int     vp_prints_value(char *out, int outlen, VALUE_PAIR *vp,int delimitst);
int     vp_prints(char *out, int outlen, VALUE_PAIR *vp);
void		vp_print(FILE *, VALUE_PAIR *);
void		vp_printlist(FILE *, VALUE_PAIR *);
#define		fprint_attr_val vp_print

/*
 *	Dictionary functions.
 */
int		dict_addvendor(const char *name, int value);
int		dict_addattr(const char *name, int vendor, int type, int value);
int		dict_addvalue(const char *namestr, char *attrstr, int value);
int		dict_init(const char *dir, const char *fn);
DICT_ATTR	*dict_attrbyvalue(int attr);
DICT_ATTR	*dict_attrbyname(const char *attr);
DICT_VALUE	*dict_valbyattr(int attr, int val);
DICT_VALUE	*dict_valbyname(int attr, const char *val);
int		dict_vendorname(const char *name);

/*
 *  Compatibility
 */
#define dict_vendorcode
#define dict_vendorpec


#if 1 /* FIXME: compat */
#define dict_attrget	dict_attrbyvalue
#define dict_attrfind	dict_attrbyname
#define dict_valfind	dict_valbyname
/*#define dict_valget	dict_valbyattr almost but not quite*/
#endif

/* md5.c */

void		librad_md5_calc(u_char *, u_char *, u_int);

/* hmac.c */

void lrad_hmac_md5(const unsigned char *text, int text_len,
		   const unsigned char *key, int key_len,
		   unsigned char *digest);

/* radius.c */
int		rad_send(RADIUS_PACKET *, const RADIUS_PACKET *, const char *secret);
RADIUS_PACKET	*rad_recv(int fd);
int		rad_decode(RADIUS_PACKET *packet, RADIUS_PACKET *original, const char *secret);
RADIUS_PACKET	*rad_alloc(int newvector);
void		rad_free(RADIUS_PACKET **);
int		rad_pwencode(char *encpw, int *len, const char *secret, const char *vector);
int		rad_pwdecode(char *encpw, int len, const char *secret, const char *vector);
int		rad_chap_encode(RADIUS_PACKET *packet, char *output, int id, VALUE_PAIR *password);

/* valuepair.c */
VALUE_PAIR	*paircreate(int attr, int type);
void		pairfree(VALUE_PAIR **);
VALUE_PAIR	*pairfind(VALUE_PAIR *, int);
void		pairdelete(VALUE_PAIR **, int);
void		pairadd(VALUE_PAIR **, VALUE_PAIR *);
VALUE_PAIR	*paircopy(VALUE_PAIR *vp);
VALUE_PAIR	*paircopy2(VALUE_PAIR *vp, int attr);
void		pairmove(VALUE_PAIR **to, VALUE_PAIR **from);
void		pairmove2(VALUE_PAIR **to, VALUE_PAIR **from, int attr);
VALUE_PAIR	*pairmake(const char *attribute, const char *value, int operator);
VALUE_PAIR	*pairread(char **ptr, int *eol);
LRAD_TOKEN	userparse(char *buffer, VALUE_PAIR **first_pair);

/*
 *	Error functions.
 */
#ifdef _LIBRADIUS
void		librad_log(const char *, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 1, 2)))
#endif
;
#endif
void		librad_perror(const char *, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 1, 2)))
#endif
;
extern char	librad_errstr[];
extern int	librad_dodns;
extern int	librad_debug;

/*
 *	Several handy miscellaneous functions.
 */
char *		ip_hostname (char *buf, size_t buflen, uint32_t ipaddr);
uint32_t	ip_getaddr (const char *);
char *		ip_ntoa(char *, uint32_t);
uint32_t	ip_addr(const char *);
char		*strNcpy(char *dest, const char *src, int n);
void		rad_lowercase(char *str);
void		rad_rmspace(char *str);

#ifdef ASCEND_BINARY
/* filters.c */
int		filterBinary(VALUE_PAIR *pair, const char *valstr);
void		print_abinary(VALUE_PAIR *vp, u_char *buffer, int len);
#endif /*ASCEND_BINARY*/

#ifdef HAVE_LOCAL_SNPRINTF
#include <stdarg.h>
int snprintf(char *str, size_t count, const char *fmt, ...);
int vsnprintf(char *str, size_t count, const char *fmt, va_list arg);
#endif

#endif /*LIBRADIUS_H*/
