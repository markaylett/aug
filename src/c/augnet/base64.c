/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/base64.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errinfo.h"
#include "augsys/string.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>       /* ENOMEM */
#include <stdlib.h>      /* malloc() */

struct aug_base64_ {
    aug_base64cb_t cb_;
    aug_object* user_;
    char buf_[AUG_MAXLINE];
	size_t pos_;
	int save_[3];
	unsigned saved_;
	int (*append_)(struct aug_base64_*, const char*, size_t);
	int (*finish_)(struct aug_base64_*);
};

/* Each set of three bytes are encodes as four: three sets of eight bits are
   split into four sets of six bits:

   xxxxxxxx yyyyyyyy zzzzzzzz -> xxxxxx xxyyyy yyyyzz zzzzzz

   64 distinct values can be represented by 6 bits, these values are encoded
   as follows:
 */

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

#define ENCODE_(x) (encoder_[x])
#define ENCODE0_(x) ENCODE_(((x) & 0xFC) >> 2)
#define ENCODE1_(x, y) ENCODE_((((x) & 0x03) << 4) | (((y) & 0xF0) >> 4))
#define ENCODE2_(x, y) ENCODE_((((x) & 0x0F) << 2) | (((y) & 0xC0) >> 6))
#define ENCODE3_(x) ENCODE_((x) & 0x3F)

#define INV_ (-1)
#define END_ (-2)

static const int decoder_[256] = {

    END_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /*\0....... */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_,   62, INV_, INV_, INV_,   63, /* ...+.../ */
      52,   53,   54,   55,   56,   57,   58,   59, /* 01234567 */
      60,   61, INV_, INV_, INV_, END_, INV_, INV_, /* 89...=.. */
    INV_,    0,    1,    2,    3,    4,    5,    6, /* .ABCDEFG */
       7,    8,    9,   10,   11,   12,   13,   14, /* HIJKLMNO */
      15,   16,   17,   18,   19,   20,   21,   22, /* PQRSTUVW */
      23,   24,   25, INV_, INV_, INV_, INV_, INV_, /* XYZ..... */
    INV_,   26,   27,   28,   29,   30,   31,   32, /* .abcdefg */
      33,   34,   35,   36,   37,   38,   39,   40, /* hijklmno */
      41,   42,   43,   44,   45,   46,   47,   48, /* pqrstuvw */
      49,   50,   51, INV_, INV_, INV_, INV_, INV_, /* xyz..... */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_, /* ........ */
    INV_, INV_, INV_, INV_, INV_, INV_, INV_, INV_  /* ........ */
};

#define DECODE_(x) (decoder_[(int)(x)])
#define DECODE0_(x, y) (((x) << 2) | (((y) & 0x30) >> 4))
#define DECODE1_(x, y) ((((x) & 0x0F) << 4) | (((y) & 0x3C) >> 2))
#define DECODE2_(x, y) ((((x) & 0x03) << 6) | ((y) & 0x3F))

typedef int aug_chunk_t[4];

static void
seterrinvalid_(const char* file, int line, char ch)
{
    aug_seterrinfo(NULL, file, line, AUG_SRCLOCAL, AUG_EPARSE,
                   AUG_MSG("invalid character '%c'"), ch);
}

static void
seterralign_(const char* file, int line)
{
    aug_seterrinfo(NULL, file, line, AUG_SRCLOCAL, AUG_EENDOF,
                   AUG_MSG("misaligned base64 sequence"));
}

static int
flush_(aug_base64_t base64)
{
    size_t pos = base64->pos_;
    base64->pos_ = 0;
    base64->buf_[pos] = '\0';
    return base64->cb_(base64->user_, base64->buf_, pos);
}

static int
putch_(aug_base64_t base64, char ch)
{
    /* Add character to buffer.  Flush first if buffer space is exhausted. */

    if (sizeof(base64->buf_) - 1 <= base64->pos_ + 1 && -1 == flush_(base64))
        return -1;
    base64->buf_[base64->pos_++] = ch;
    return 0;
}

static int
decodealign_(aug_base64_t base64, const char* src, size_t len)
{
    /* This function will return one of the following:
       a) The number of characters consumed in the alignment.
       b) END_ if the alignment could not be perform due to lack of available
       characters in the source buffer.
       c) on error.
    */

    char in;
    int align, out;
    int* save = base64->save_;
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

    align = 4 - base64->saved_;
    switch (base64->saved_) {
    case 1:
        if (!len--)
            return END_;

        in = *src++;
        switch (out = DECODE_(in)) {
        case END_:
            seterralign_(__FILE__, __LINE__);
            return -1;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return -1;
        }

        save[1] = chunk[1] = out;
        base64->saved_ = 2;

        /* Fallthrough. */

    case 2:
        if (!len--)
            return END_;

        in = *src++;
        switch (out = DECODE_(in)) {
        case END_:
            if (-1 == putch_(base64, DECODE0_(chunk[0], chunk[1])))
                return -1;

            base64->saved_ = 0;
            return END_;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return -1;
        }

        save[2] = chunk[2] = out;
        base64->saved_ = 3;

        /* Fallthrough. */

    case 3:
        if (!len--)
            return END_;

        in = *src++;
        switch (out = DECODE_(in)) {
        case END_:
            if (-1 == putch_(base64, DECODE0_(chunk[0], chunk[1]))
                || -1 == putch_(base64, DECODE1_(chunk[1], chunk[2])))
                return -1;

            base64->saved_ = 0;
            return END_;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return -1;
        }

        chunk[3] = out;
    }

    if (-1 == putch_(base64, DECODE0_(chunk[0], chunk[1]))
        || -1 == putch_(base64, DECODE1_(chunk[1],  chunk[2]))
        || -1 == putch_(base64, DECODE2_(chunk[2], chunk[3])))
        return -1;

    base64->saved_ = 0;
    return align;
}

static int
decodeappend_(aug_base64_t base64, const char* src, size_t len)
{
    char in;
    int align, i = 0, out;
    aug_chunk_t chunk;

    switch (align = decodealign_(base64, src, len)) {
    case END_:
        return 0;
    case -1:
        return -1;
    case 0:
        break;
    default:
        src += align;
        len -= align;
    }

    while (len--) {

        in = src[i++];
        switch (out = DECODE_(in)) {
        case END_:
            seterralign_(__FILE__, __LINE__);
            return -1;
        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return -1;
        }

        chunk[0] = out;
        if (!len--)
            goto save;

        in = src[i++];
        switch (out = DECODE_(in)) {
        case END_:
            seterralign_(__FILE__, __LINE__);
            return -1;
        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return -1;
        }

        chunk[1] = out;
        if (!len--)
            goto save;

        in = src[i++];
        switch (out = DECODE_(in)) {
        case END_:
            if (-1 == putch_(base64, DECODE0_(chunk[0], chunk[1])))
                return -1;
            return 0;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return -1;
        }

        chunk[2] = out;
        if (!len--)
            goto save;

        in = src[i++];
        switch (out = DECODE_(in)) {
        case END_:
            if (-1 == putch_(base64, DECODE0_(chunk[0], chunk[1]))
                || -1 == putch_(base64, DECODE1_(chunk[1], chunk[2])))
                return -1;
            return 0;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return -1;
        }

        chunk[3] = out;

        if (-1 == putch_(base64, DECODE0_(chunk[0], chunk[1]))
            || -1 == putch_(base64, DECODE1_(chunk[1], chunk[2]))
            || -1 == putch_(base64, DECODE2_(chunk[2], chunk[3])))
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
decodefinish_(aug_base64_t base64)
{
    return base64->pos_ ? flush_(base64) : 0;
}

static int
putchunk_(aug_base64_t base64, aug_chunk_t chunk)
{
    /* Flush if buffer overflow. */

    if (sizeof(base64->buf_) <= base64->pos_ + 4 && -1 == flush_(base64))
        return -1;

    /* Copy chunk to buffer. */

    base64->buf_[base64->pos_ + 0] = chunk[0];
    base64->buf_[base64->pos_ + 1] = chunk[1];
    base64->buf_[base64->pos_ + 2] = chunk[2];
    base64->buf_[base64->pos_ + 3] = chunk[3];
    base64->pos_ += 4;
    return 0;
}

static int
encodealign_(aug_base64_t base64, const char* src, size_t len)
{
    /* This function will return one of the following:
       a) The number of characters consumed in the alignment.
       b) END_ if the alignment could not be perform due to lack of available
       characters in the source buffer.
       c) -1 on error.
    */

    const int* save = base64->save_;
    aug_chunk_t chunk;

    switch (base64->saved_) {
    case 2:
        if (!len)
            return END_;

        chunk[0] = ENCODE0_(save[0]);
        chunk[1] = ENCODE1_(save[0], save[1]);
        chunk[2] = ENCODE2_(save[1], src[0]);
        chunk[3] = ENCODE3_(src[0]);

        base64->saved_ = 0;
        if (-1 == putchunk_(base64, chunk))
            return -1;

        return 1;

    case 1:
        switch (len) {
        case 0:
            return END_;

        case 1:
            base64->save_[1] = src[0];
            base64->saved_ = 2;
            return END_;

        default:
            chunk[0] = ENCODE0_(save[0]);
            chunk[1] = ENCODE1_(save[0], src[0]);
            chunk[2] = ENCODE2_(src[0], src[1]);
            chunk[3] = ENCODE3_(src[1]);

            base64->saved_ = 0;
            if (-1 == putchunk_(base64, chunk))
                return -1;

            return 2;
        }
    }
    return 0;
}

static int
encodeappend_(aug_base64_t base64, const char* src, size_t len)
{
    int align;
    size_t whole, i;
    unsigned part;

    switch (align = encodealign_(base64, src, len)) {
    case END_:
        return 0;
    case -1:
        return -1;
    case 0:
        break;
    default:
        src += align;
        len -= align;
    }

    /* For each aligned chunk. */

    whole = len / 3, part = (unsigned)(len % 3);
    for (i = 0; i < whole; ++i, src += 3) {

        aug_chunk_t chunk;
        chunk[0] = ENCODE0_(src[0]);
        chunk[1] = ENCODE1_(src[0], src[1]);
        chunk[2] = ENCODE2_(src[1], src[2]);
        chunk[3] = ENCODE3_(src[2]);

        if (-1 == putchunk_(base64, chunk))
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
encodefinish_(aug_base64_t base64)
{
    const int* save = base64->save_;
    aug_chunk_t chunk;

    switch (base64->saved_) {
    case 2:
        chunk[0] = ENCODE0_(save[0]);
        chunk[1] = ENCODE1_(save[0], save[1]);
        chunk[2] = ENCODE2_(save[1], 0);
        chunk[3] = '=';

        base64->saved_ = 0;
        if (-1 == putchunk_(base64, chunk))
            return -1;
        break;

    case 1:
        chunk[0] = ENCODE0_(save[0]);
        chunk[1] = ENCODE1_(save[0], 0);
        chunk[2] = '=';
        chunk[3] = '=';

        base64->saved_ = 0;
        if (-1 == putchunk_(base64, chunk))
            return -1;
        break;
    }
    return base64->pos_ ? flush_(base64) : 0;
}

AUGNET_API aug_base64_t
aug_createbase64(enum aug_base64mode mode, aug_base64cb_t cb,
                 aug_object* user)
{
    aug_base64_t base64 = malloc(sizeof(struct aug_base64_));
    if (!base64) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    base64->cb_ = cb;
    if ((base64->user_ = user))
        aug_incref(user);

    base64->saved_ = 0;
    base64->pos_ = 0;

    switch (mode) {
    case AUG_DECODE64:
        base64->append_ = decodeappend_;
        base64->finish_ = decodefinish_;
        break;

    case AUG_ENCODE64:
        base64->append_ = encodeappend_;
        base64->finish_ = encodefinish_;
        break;
    }

    return base64;
}

AUGNET_API int
aug_destroybase64(aug_base64_t base64)
{
    if (base64->user_)
        aug_decref(base64->user_);
    free(base64);
    return 0;
}

AUGNET_API int
aug_appendbase64(aug_base64_t base64, const char* src, size_t len)
{
    return base64->append_(base64, src, len);
}

AUGNET_API int
aug_finishbase64(aug_base64_t base64)
{
    return base64->finish_(base64);
}
