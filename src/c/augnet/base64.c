/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/base64.h"

static const char rcsid[] = "$Id:$";

#include "augutil/var.h"

#include "augsys/defs.h" /* AUG_MAXLINE */
#include "augsys/errinfo.h"
#include "augsys/string.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>       /* ENOMEM */
#include <stdlib.h>      /* malloc() */

struct aug_base64_ {
    aug_base64cb_t cb_;
    struct aug_var var_;
    char buf_[AUG_MAXLINE];
	char save_[3];
	unsigned char saved_;
	size_t pos_;
	int (*append_)(struct aug_base64_*, const char*, size_t);
	int (*end_)(struct aug_base64_*);
};

/*
   Extract from RFC 1521:

                         Table 1: The Base64 Alphabet

   Value Encoding  Value Encoding  Value Encoding  Value Encoding
        0 A            17 R            34 i            51 z
        1 B            18 S            35 j            52 0
        2 C            19 T            36 k            53 1
        3 D            20 U            37 l            54 2
        4 E            21 V            38 m            55 3
        5 F            22 W            39 n            56 4
        6 G            23 X            40 o            57 5
        7 H            24 Y            41 p            58 6
        8 I            25 Z            42 q            59 7
        9 J            26 a            43 r            60 8
       10 K            27 b            44 s            61 9
       11 L            28 c            45 t            62 +
       12 M            29 d            46 u            63 /
       13 N            30 e            47 v
       14 O            31 f            48 w         (pad) =
       15 P            32 g            49 x
       16 Q            33 h            50 y
*/

#define NUL_ (64)
#define INV_ (65)

static const char encoder_[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

static const char decoder_[128] = {

    NUL_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /*\0....... */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_,   62, INV_, INV_, INV_,   63, /* ...+.../ */
      52,   53,   54,   55,   56,   57,   58,   59, /* 01234567 */
      60,   61, INV_, INV_, INV_, NUL_, INV_, INV_, /* 89...=.. */
    INV_,    0,    1,    2,    3,    4,    5,    6, /* .ABCDEFG */
       7,    8,    9,   10,   11,   12,   13,   14, /* HIJKLMNO */
      15,   16,   17,   18,   19,   20,   21,   22, /* PQRSTUVW */
      23,   24,   25, INV_, INV_, INV_, INV_, INV_, /* XYZ..... */
    INV_,   26,   27,   28,   29,   30,   31,   32, /* .abcdefg */
      33,   34,   35,   36,   37,   38,   39,   40, /* hijklmno */
      41,   42,   43,   44,   45,   46,   47,   48, /* pqrstuvw */
      49,   50,   51, INV_, INV_, INV_, INV_, INV_  /* xyz..... */
};

#define RETERROR_ (-1)
#define RETENDOF_ (-2)
#define ENCODE_(x) (encoder_[(int)x])
#define DECODE_(x) (isascii(x) ? decoder_[(int)x] : INV_)

typedef char aug_chunk_t[4];

static int
flush_(aug_base64_t base64)
{
    size_t pos = base64->pos_;
    base64->pos_ = 0;

    /* Notify callback function, if set. */

    if (!base64->cb_)
        return 0;

    base64->buf_[pos] = '\0';
    return base64->cb_(&base64->var_, base64->buf_, pos);
}

static int
putdecode_(aug_base64_t base64, char ch)
{
    if (sizeof(base64->buf_) <= base64->pos_ + 1 && -1 == flush_(base64))
        return -1;
    base64->buf_[base64->pos_++] = ch;
    return 0;
}

static int
aligndecode_(aug_base64_t base64, const char* src, size_t len)
{
    /* This function will return one of the following:
       a) The number of characters used to perform the re-alignment.
       b) RETERROR_ in the case of an error condition.
       c) RETENDOF_ if the alignment could not be perform due to lack of
       available characters in the input buffer. */

    char ch;
    char* save = base64->save_;

    size_t align, i = 0;
    aug_chunk_t chunk;

    /* Copy any previously saved characters to local chunk. */

    switch (base64->saved_) {
    case 3:
        chunk[2] = save[2];
    case 2:
        chunk[1] = save[1];
    case 1:
        chunk[0] = save[0];
        break;
    default:
        return 0;
    }

    /* Attempt to read remaining characters required to complete the
       alignment. */

    align = sizeof(chunk) - base64->saved_;
    switch (base64->saved_) {
    case 1:
        if (!len--)
            return RETENDOF_;

        ch = src[i++];
        switch (chunk[1] = DECODE_(ch)) {
        case NUL_: case INV_:
            return RETERROR_;
        }

        save[1] = chunk[1];
        base64->saved_ = 2;

    case 2:
        if (!len--)
            return RETENDOF_;

        ch = src[i++];
        switch (chunk[2] = DECODE_(ch)) {
        case NUL_:
            if (-1 == putdecode_(base64, ((chunk[0] << 2) | (chunk[1] >> 4))))
                return RETERROR_;

            base64->saved_ = 0;
            return RETENDOF_;

        case INV_:
            return RETERROR_;
        }

        save[2] = chunk[2];
        base64->saved_ = 3;

    case 3:
        if (!len--)
            return RETENDOF_;

        ch = src[i++];
        switch (chunk[3] = DECODE_(ch)) {
        case NUL_:
            if (-1 == putdecode_(base64, ((chunk[0] << 2) | (chunk[1] >> 4)))
                || -1 == putdecode_(base64,
                                    ((chunk[1] << 4) | (chunk[2] >> 2))))
                return RETERROR_;

            base64->saved_ = 0;
            return RETENDOF_;

        case INV_:
            return RETERROR_;
        }
    }

    if (-1 == putdecode_(base64, ((chunk[0] << 2) | (chunk[1] >> 4)))
        || -1 == putdecode_(base64, ((chunk[1] << 4) | (chunk[2] >> 2)))
        || -1 == putdecode_(base64, ((chunk[2] << 6) | chunk[3])))
        return RETERROR_;

    base64->saved_ = 0;
    return align;
}

static int
decodeappend_(aug_base64_t base64, const char* src, size_t len)
{
    char ch;
    size_t align, i = 0;
    aug_chunk_t chunk;

    switch (align = aligndecode_(base64, src, len)) {
    case RETERROR_:
        return -1;
    case RETENDOF_:
        return 0;
    case 0:
        break;
    default:
        src += align;
        len -= align;
    }

    while (len--) {

        ch = src[i++];
        switch (chunk[0] = DECODE_(ch)) {
        case NUL_: case INV_:
            return -1;
        }

        if (!len--)
            goto save;

        ch = src[i++];
        switch (chunk[1] = DECODE_(ch)) {
        case NUL_: case INV_:
            return -1;
        }

        if (!len--)
            goto save;

        ch = src[i++];
        switch (chunk[2] = DECODE_(ch)) {
        case NUL_:
            if (-1 == putdecode_(base64, ((chunk[0] << 2) | (chunk[1] >> 4))))
                return -1;
            return 0;

        case INV_:
            return -1;
        }

        if (!len--)
            goto save;

        ch = src[i++];
        switch (chunk[3] = DECODE_(ch)) {
        case NUL_:
            if (-1 == putdecode_(base64, ((chunk[0] << 2) | (chunk[1] >> 4)))
                || -1 == putdecode_(base64,
                                    ((chunk[1] << 4) | (chunk[2] >> 2))))
                return -1;
            return 0;

        case INV_:
            return -1;
        }

        if (-1 == putdecode_(base64, ((chunk[0] << 2) | (chunk[1] >> 4)))
            || -1 == putdecode_(base64, ((chunk[1] << 4) | (chunk[2] >> 2)))
            || -1 == putdecode_(base64, ((chunk[2] << 6) | chunk[3])))
            return -1;
    }
    return 0;

 save:
    switch (base64->saved_ = i % 4) {
    case 3:
        base64->save_[2] = chunk[2];
    case 2:
        base64->save_[1] = chunk[1];
    case 1:
        base64->save_[0] = chunk[0];
    }
    return 0;
}

static int
decodeend_(aug_base64_t base64)
{
    return base64->pos_ ? flush_(base64) : 0;
}

static int
putencode_(aug_base64_t base64, aug_chunk_t chunk)
{
    if (sizeof(base64->buf_) <= base64->pos_ + sizeof(chunk)
        && -1 == flush_(base64))
        return -1;

    memcpy(base64->buf_ + base64->pos_, chunk, sizeof(chunk));
    base64->pos_ += sizeof(chunk);
    return 0;
}

static int
alignencode_(aug_base64_t base64, const char* src, size_t len)
{
    /* This function will return one of the following:
       a) The number of characters used to perform the re-alignment.
       b) RETERROR_ in the case of an error condition.
       c) RETENDOF_ if the alignment could not be perform due to lack of
       available characters in the input buffer. */

    const char* save = base64->save_;
    aug_chunk_t chunk;

    switch (base64->saved_) {
    case 2:
        if (!len)
            return RETENDOF_;

        chunk[0] = ENCODE_(save[0] >> 2);
        chunk[1] = ENCODE_(((save[0] & 0x03) << 4) | (save[1] >> 4));
        chunk[2] = ENCODE_(((save[1] & 0x0F) << 2) | (src[0] >> 6));
        chunk[3] = ENCODE_(src[0] & 0x3F);

        base64->saved_ = 0;
        if (-1 == putencode_(base64, chunk))
            return RETERROR_;

        return 1;

    case 1:
        switch (len) {
        case 0:
            return RETENDOF_;

        case 1:
            base64->save_[1] = src[0];
            base64->saved_ = 2;
            return RETENDOF_;

        default:
            chunk[0] = ENCODE_(save[0] >> 2);
            chunk[1] = ENCODE_(((save[0] & 0x03) << 4) | (src[0] >> 4));
            chunk[2] = ENCODE_(((src[0] & 0x0F) << 2) | (src[1] >> 6));
            chunk[3] = ENCODE_(src[1] & 0x3F);

            base64->saved_ = 0;
            if (-1 == putencode_(base64, chunk))
                return RETERROR_;

            return 2;
        }
    }
    return 0;
}

static int
encodeappend_(aug_base64_t base64, const char* src, size_t len)
{
    size_t align, whole, part, i;

    switch (align = alignencode_(base64, src, len)) {
    case RETERROR_:
        return -1;
    case RETENDOF_:
        return 0;
    case 0:
        break;
    default:
        src += align;
        len -= align;
    }

    whole = len / 3, part = len % 3;
    for (i = 0; i < whole; ++i, src += 3) {

        aug_chunk_t chunk;
        chunk[0] = ENCODE_(src[0] >> 2);
        chunk[1] = ENCODE_(((src[0] & 0x03) << 4) | (src[1] >> 4));
        chunk[2] = ENCODE_(((src[1] & 0x0F) << 2) | (src[2] >> 6));
        chunk[3] = ENCODE_(src[2] & 0x3F);

        if (-1 == putencode_(base64, chunk))
            return -1;
    }

    switch (part) {
    case 2:
        base64->save_[1] = src[1];
        /* Fallthrough. */
    case 1:
        base64->save_[0] = src[0];
        base64->saved_ = part;
    }
    return 0;
}

static int
encodeend_(aug_base64_t base64)
{
    const char* save = base64->save_;
    aug_chunk_t chunk;

    switch (base64->saved_) {
    case 2:
        chunk[0] = ENCODE_(save[0] >> 2);
        chunk[1] = ENCODE_(((save[0] & 0x03) << 4) | (save[1] >> 4));
        chunk[2] = ENCODE_(((save[1] & 0x0F) << 2));
        chunk[3] = '=';

        base64->saved_ = 0;
        if (-1 == putencode_(base64, chunk))
            return -1;
        break;

    case 1:
        chunk[0] = ENCODE_(save[0] >> 2);
        chunk[1] = ENCODE_(((save[0] & 0x03) << 4));
        chunk[2] = '=';
        chunk[3] = '=';

        base64->saved_ = 0;
        if (-1 == putencode_(base64, chunk))
            return -1;
        break;
    }
    return base64->pos_ ? flush_(base64) : 0;
}

AUGNET_API aug_base64_t
aug_createbase64(enum aug_base64mode mode, aug_base64cb_t cb,
                 const struct aug_var* var)
{
    aug_base64_t base64 = malloc(sizeof(struct aug_base64_));

    if (!base64) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    base64->cb_ = cb;
    aug_setvar(&base64->var_, var);

    base64->saved_ = 0;
    base64->pos_ = 0;

    switch (mode) {
    case AUG_DECODE64:
        base64->append_ = decodeappend_;
        base64->end_ = decodeend_;
        break;

    case AUG_ENCODE64:
        base64->append_ = encodeappend_;
        base64->end_ = encodeend_;
        /* Ensure buffer is at least as big as a single chunk plus null
           terminator. */
        assert(sizeof(aug_chunk_t) < size);
        break;

    default:
        base64->append_ = NULL;
        base64->end_ = NULL;
    }

    return base64;
}

AUGNET_API int
aug_destroybase64(aug_base64_t base64)
{
    aug_destroyvar(&base64->var_);
    free(base64);
    return 0;
}

AUGNET_API int
aug_appendbase64(aug_base64_t base64, const char* src, size_t len)
{
    if (-1 == base64->append_(base64, src, len)) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

AUGNET_API int
aug_endbase64(aug_base64_t base64)
{
    if (-1 == base64->end_(base64)) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}
