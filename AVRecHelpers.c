#ifdef SUPPORT_LIBAVCODEC

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include "errno.h"
// inline is not available for the microsoft c compiler
#define inline __inline
#include "ffmpeg\\libavformat\\avformat.h"

#if (_MSC_VER < 1400)
int _get_output_format( void ){ return 0; } // for libmingwex.a
#endif

int strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *us1 = (const unsigned char *)s1,
			*us2 = (const unsigned char *)s2;

	while (tolower(*us1) == tolower(*us2++))
		if (*us1++ == '\0')
			return (0);
	return (tolower(*us1) - tolower(*--us2));
}

// From file.c changed to support URL_NOTRUNC
#define URL_NOTRUNC   4
extern URLProtocol *first_protocol;
int av_strstart(const char *str, const char *val, const char **ptr);
static int avrec_file_open(URLContext *h, const char *filename, int flags)
{
    int access;
    int fd;

    av_strstart(filename, "file:", &filename);

	if (flags & URL_NOTRUNC)
	{
		if (flags & URL_RDWR) {
			access = O_CREAT | O_RDWR;
		} else if (flags & URL_WRONLY) {
			access = O_CREAT | O_WRONLY;
		} else {
			access = O_RDONLY;
		}
	}
	else
	{
		if (flags & URL_RDWR) {
			access = O_CREAT | O_TRUNC | O_RDWR;
		} else if (flags & URL_WRONLY) {
			access = O_CREAT | O_TRUNC | O_WRONLY;
		} else {
			access = O_RDONLY;
		}
	}
    access |= O_BINARY;
    fd = _open(filename, access, 0666);
    if (fd < 0)
        return AVERROR(ENOENT);
    h->priv_data = (void *)(size_t)fd;
    return 0;
}

int avrec_url_open_protocol (URLContext **puc, struct URLProtocol *up,
                       const char *filename, int flags)
{
    URLContext *uc;
    int err;

    uc = av_malloc(sizeof(URLContext) + strlen(filename) + 1);
    if (!uc) {
        err = AVERROR(ENOMEM);
        goto fail;
    }
#if LIBAVFORMAT_VERSION_MAJOR >= 53
    uc->av_class = &urlcontext_class;
#endif
    uc->filename = (char *) &uc[1];
    strcpy(uc->filename, filename);
    uc->prot = up;
    uc->flags = flags;
    uc->is_streamed = 0; /* default = not streamed */
    uc->max_packet_size = 0; /* default: stream file */

	// Use No Truncation File Open
    //err = up->url_open(uc, filename, flags);
	err = avrec_file_open(uc, filename, flags);

    if (err < 0) {
        av_free(uc);
        *puc = NULL;
        return err;
    }

    //We must be carefull here as url_seek() could be slow, for example for http
    if(   (flags & (URL_WRONLY | URL_RDWR))
       || !strcmp(up->name, "file"))
        if(!uc->is_streamed && url_seek(uc, 0, SEEK_SET) < 0)
            uc->is_streamed= 1;
    *puc = uc;
    return 0;
 fail:
    *puc = NULL;
    return err;
}

int avrec_url_open(URLContext **puc, const char *filename, int flags)
{
    URLProtocol *up;
    const char *p;
    char proto_str[128], *q;

    p = filename;
    q = proto_str;
    while (*p != '\0' && *p != ':') {
        /* protocols can only contain alphabetic chars */
        if (!isalpha(*p))
            goto file_proto;
        if ((q - proto_str) < sizeof(proto_str) - 1)
            *q++ = *p;
        p++;
    }
    /* if the protocol has length 1, we consider it is a dos drive */
    if (*p == '\0' || (q - proto_str) <= 1) {
    file_proto:
        strcpy(proto_str, "file");
    } else {
        *q = '\0';
    }

    up = first_protocol;
    while (up != NULL) {
        if (!strcmp(proto_str, up->name))
            return avrec_url_open_protocol (puc, up, filename, flags);
        up = up->next;
    }
    *puc = NULL;
    return AVERROR(ENOENT);
}

int avrec_url_fopen(ByteIOContext **s, const char *filename, int flags)
{
    URLContext *h;
    int err;

    err = avrec_url_open(&h, filename, flags);
    if (err < 0)
        return err;
    err = url_fdopen(s, h); // Allocates ByteIOContext
    if (err < 0) {
        url_close(h);
        return err;
    }
    return 0;
}

#endif