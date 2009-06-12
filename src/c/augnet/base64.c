/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define AUGNET_BUILD
#include "augnet/base64.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include <assert.h>
#include <ctype.h>

struct aug_base64_ {
    aug_mpool* mpool_;
    aug_base64cb_t cb_;
    aug_object* ob_;
    char buf_[AUG_MAXLINE];
	size_t pos_;
	int save_[3];
	unsigned saved_;
	aug_result (*append_)(struct aug_base64_*, const char*, size_t);
	aug_result (*finish_)(struct aug_base64_*);
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
    aug_seterrinfo(aug_tlerr, file, line, "aug", AUG_EPARSE,
                   AUG_MSG("invalid character [%c]"), ch);
}

static void
seterralign_(const char* file, int line)
{
    aug_seterrinfo(aug_tlerr, file, line, "aug", AUG_EENDOF,
                   AUG_MSG("misaligned base64 sequence"));
}

static aug_result
flush_(aug_base64_t base64)
{
    size_t pos = base64->pos_;
    base64->pos_ = 0;
    base64->buf_[pos] = '\0';
    return base64->cb_(base64->buf_, pos, base64->ob_);
}

static aug_result
putch_(aug_base64_t base64, char ch)
{
    /* Add character to buffer.  Flush first if buffer space is exhausted. */

    if (sizeof(base64->buf_) - 1 <= base64->pos_ + 1)
        aug_verify(flush_(base64));

    base64->buf_[base64->pos_++] = ch;
    return AUG_SUCCESS;
}

static aug_rsize
decodealign_(aug_base64_t base64, const char* src, size_t len)
{
    /* This function will return one of the following:

       a) The number of characters consumed in the alignment.

       b) AUG_FAILNONE if the alignment could not be perform due to lack of
          available characters in the source buffer.

       c) AUG_FAILERROR on error.
    */

    char in;
    int align, out;
    int* save = base64->save_;
    aug_chunk_t chunk = { 0 }; /* Suppress warnings. */

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
        return AUG_ZERO;
    }

    /* Attempt to read remaining characters required to complete the
       alignment. */

    align = 4 - base64->saved_;
    switch (base64->saved_) {
    case 1:
        if (!len--)
            return AUG_FAILNONE;

        in = *src++;
        switch (out = DECODE_(in)) {
        case END_:
            seterralign_(__FILE__, __LINE__);
            return AUG_FAILERROR;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return AUG_FAILERROR;
        }

        save[1] = chunk[1] = out;
        base64->saved_ = 2;

        /* Fallthrough. */

    case 2:
        if (!len--)
            return AUG_FAILNONE;

        in = *src++;
        switch (out = DECODE_(in)) {
        case END_:
            aug_verify(putch_(base64, DECODE0_(chunk[0], chunk[1])));
            base64->saved_ = 0;
            return AUG_FAILNONE;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return AUG_FAILERROR;
        }

        save[2] = chunk[2] = out;
        base64->saved_ = 3;

        /* Fallthrough. */

    case 3:
        if (!len--)
            return AUG_FAILNONE;

        in = *src++;
        switch (out = DECODE_(in)) {
        case END_:
            aug_verify(putch_(base64, DECODE0_(chunk[0], chunk[1])));
            aug_verify(putch_(base64, DECODE1_(chunk[1], chunk[2])));
            base64->saved_ = 0;
            return AUG_FAILNONE;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return AUG_FAILERROR;
        }

        chunk[3] = out;
    }

    aug_verify(putch_(base64, DECODE0_(chunk[0], chunk[1])));
    aug_verify(putch_(base64, DECODE1_(chunk[1],  chunk[2])));
    aug_verify(putch_(base64, DECODE2_(chunk[2], chunk[3])));
    base64->saved_ = 0;

    return AUG_MKRESULT(align);
}

static aug_result
decodeappend_(aug_base64_t base64, const char* src, size_t len)
{
    aug_rsize align;
    char in;
    int i = 0, out;
    aug_chunk_t chunk = { 0 }; /* Suppress warnings. */

    if (AUG_ISFAIL(align = decodealign_(base64, src, len))) {

        if (AUG_ISNONE(align))
            return AUG_SUCCESS;

        return align;
    }

    src += AUG_RESULT(align);
    len -= AUG_RESULT(align);

    while (len--) {

        in = src[i++];
        switch (out = DECODE_(in)) {
        case END_:
            seterralign_(__FILE__, __LINE__);
            return AUG_FAILERROR;
        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return AUG_FAILERROR;
        }

        chunk[0] = out;
        if (!len--)
            goto save;

        in = src[i++];
        switch (out = DECODE_(in)) {
        case END_:
            seterralign_(__FILE__, __LINE__);
            return AUG_FAILERROR;
        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return AUG_FAILERROR;
        }

        chunk[1] = out;
        if (!len--)
            goto save;

        in = src[i++];
        switch (out = DECODE_(in)) {
        case END_:
            aug_verify(putch_(base64, DECODE0_(chunk[0], chunk[1])));
            return AUG_SUCCESS;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return AUG_FAILERROR;
        }

        chunk[2] = out;
        if (!len--)
            goto save;

        in = src[i++];
        switch (out = DECODE_(in)) {
        case END_:
            aug_verify(putch_(base64, DECODE0_(chunk[0], chunk[1])));
            aug_verify(putch_(base64, DECODE1_(chunk[1], chunk[2])));
            return AUG_SUCCESS;

        case INV_:
            seterrinvalid_(__FILE__, __LINE__, in);
            return AUG_FAILERROR;
        }

        chunk[3] = out;

        aug_verify(putch_(base64, DECODE0_(chunk[0], chunk[1])));
        aug_verify(putch_(base64, DECODE1_(chunk[1], chunk[2])));
        aug_verify(putch_(base64, DECODE2_(chunk[2], chunk[3])));
    }
    return AUG_SUCCESS;

 save:
    switch (base64->saved_ = i % 4) {
    case 3:
        base64->save_[2] = chunk[2];
    case 2:
        base64->save_[1] = chunk[1];
    case 1:
        base64->save_[0] = chunk[0];
    }
    return AUG_SUCCESS;
}

static aug_result
decodefinish_(aug_base64_t base64)
{
    return base64->pos_ ? flush_(base64) : AUG_SUCCESS;
}

static aug_result
putchunk_(aug_base64_t base64, aug_chunk_t chunk)
{
    /* Flush if buffer overflow. */

    if (sizeof(base64->buf_) <= base64->pos_ + 4)
        aug_verify(flush_(base64));

    /* Copy chunk to buffer. */

    base64->buf_[base64->pos_ + 0] = chunk[0];
    base64->buf_[base64->pos_ + 1] = chunk[1];
    base64->buf_[base64->pos_ + 2] = chunk[2];
    base64->buf_[base64->pos_ + 3] = chunk[3];
    base64->pos_ += 4;
    return AUG_SUCCESS;
}

static aug_rsize
encodealign_(aug_base64_t base64, const char* src, size_t len)
{
    /* This function will return one of the following:

       a) The number of characters consumed in the alignment.

       b) AUG_FAILNONE if the alignment could not be perform due to lack of
       available characters in the source buffer.

       c) AUG_FAILERROR on error.
    */

    const int* save = base64->save_;
    aug_chunk_t chunk;

    switch (base64->saved_) {
    case 2:
        if (!len)
            return AUG_FAILNONE;

        chunk[0] = ENCODE0_(save[0]);
        chunk[1] = ENCODE1_(save[0], save[1]);
        chunk[2] = ENCODE2_(save[1], src[0]);
        chunk[3] = ENCODE3_(src[0]);

        base64->saved_ = 0;
        aug_verify(putchunk_(base64, chunk));

        return AUG_MKRESULT(1);

    case 1:
        switch (len) {
        case 0:
            return AUG_FAILNONE;

        case 1:
            base64->save_[1] = src[0];
            base64->saved_ = 2;
            return AUG_FAILNONE;

        default:
            chunk[0] = ENCODE0_(save[0]);
            chunk[1] = ENCODE1_(save[0], src[0]);
            chunk[2] = ENCODE2_(src[0], src[1]);
            chunk[3] = ENCODE3_(src[1]);

            base64->saved_ = 0;
            aug_verify(putchunk_(base64, chunk));

            return AUG_MKRESULT(2);
        }
    }
    return AUG_ZERO;
}

static aug_result
encodeappend_(aug_base64_t base64, const char* src, size_t len)
{
    aug_rsize align;
    size_t whole, i;
    unsigned part;

    if (AUG_ISFAIL(align = encodealign_(base64, src, len))) {

        if (AUG_ISNONE(align))
            return AUG_SUCCESS;

        return align;
    }

    src += AUG_RESULT(align);
    len -= AUG_RESULT(align);

    /* For each aligned chunk. */

    whole = len / 3, part = (unsigned)(len % 3);
    for (i = 0; i < whole; ++i, src += 3) {

        aug_chunk_t chunk;
        chunk[0] = ENCODE0_(src[0]);
        chunk[1] = ENCODE1_(src[0], src[1]);
        chunk[2] = ENCODE2_(src[1], src[2]);
        chunk[3] = ENCODE3_(src[2]);

        aug_verify(putchunk_(base64, chunk));
    }

    switch (part) {
    case 2:
        base64->save_[1] = src[1];
        /* Fallthrough. */
    case 1:
        base64->save_[0] = src[0];
        base64->saved_ = part;
    }
    return AUG_SUCCESS;
}

static aug_result
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
        aug_verify(putchunk_(base64, chunk));
        break;

    case 1:
        chunk[0] = ENCODE0_(save[0]);
        chunk[1] = ENCODE1_(save[0], 0);
        chunk[2] = '=';
        chunk[3] = '=';

        base64->saved_ = 0;
        aug_verify(putchunk_(base64, chunk));
        break;
    }
    return base64->pos_ ? flush_(base64) : AUG_SUCCESS;
}

AUGNET_API aug_base64_t
aug_createbase64(aug_mpool* mpool, int mode, aug_base64cb_t cb,
                 aug_object* ob)
{
    aug_base64_t base64;
    if (!(base64 = aug_allocmem(mpool, sizeof(struct aug_base64_))))
        return NULL;

    base64->mpool_ = mpool;
    base64->cb_ = cb;
    if ((base64->ob_ = ob))
        aug_retain(ob);

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

    aug_retain(mpool);
    return base64;
}

AUGNET_API void
aug_destroybase64(aug_base64_t base64)
{
    aug_mpool* mpool = base64->mpool_;
    if (base64->ob_)
        aug_release(base64->ob_);
    aug_freemem(mpool, base64);
    aug_release(mpool);
}

AUGNET_API aug_result
aug_appendbase64(aug_base64_t base64, const char* src, size_t len)
{
    return base64->append_(base64, src, len);
}

AUGNET_API aug_result
aug_finishbase64(aug_base64_t base64)
{
    return base64->finish_(base64);
}
