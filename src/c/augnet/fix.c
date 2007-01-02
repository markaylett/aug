/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/fix.h"

static const char rcsid[] = "$Id$";

#include "augutil/strbuf.h"
#include "augutil/var.h"

#include "augsys/errinfo.h"
#include "augsys/string.h"

#include <ctype.h>  /* isdigit() */
#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */

/* All fields (including those of data type data i.e. SecureData, RawData,
   SignatureData, XmlData, etc.) in a FIX message are terminated by a
   delimiter character.  The non-printing, ASCII "SOH" (#001, hex: 0x01,
   referred to in this document as <SOH>), is used for field termination.
   Messages are delimited by the "SOH" character following the CheckSum field.
   All messages begin with the "8=FIX.x.y<SOH>" string and terminate with
   "10=nnn<SOH>". */

/* General message format is composed of the standard header followed by the
   body followed by the standard trailer:

   The first three fields in the standard header are BeginString (tag #8)
   followed by BodyLength (tag #9) followed by MsgType (tag #35).

   The last field in the standard trailer is the CheckSum (tag #10). */

#define SOH_ "\01"
#define FIELD_SEP_ "="

#define BEGINSTRING_TAG_ "8"
#define BODYLENGTH_TAG_ "9"
#define CHECKSUM_TAG_ "10"

#define BEGINSTRING_PREFIX_ BEGINSTRING_TAG_ FIELD_SEP_
#define BODYLENGTH_PREFIX_ BODYLENGTH_TAG_ FIELD_SEP_
#define CHECKSUM_PREFIX_ CHECKSUM_TAG_ FIELD_SEP_

#define BEGINSTRING_PREFIX_SIZE_ (sizeof(BEGINSTRING_PREFIX_) - 1)
#define BODYLENGTH_PREFIX_SIZE_ (sizeof(BODYLENGTH_PREFIX_) - 1)
#define CHECKSUM_PREFIX_SIZE_ (sizeof(BODYLENGTH_PREFIX_) - 1)

#define BEGINSTRING_FIELD_ BEGINSTRING_TAG_ FIELD_SEP_ "FIX.x.y"
#define CHECKSUM_FIELD_ CHECKSUM_TAG_ FIELD_SEP_ "nnn"

#define HEAD_ BEGINSTRING_FIELD_ SOH_ BODYLENGTH_PREFIX_
#define TAIL_ CHECKSUM_FIELD_ SOH_

#define HEAD_SIZE_ (sizeof(HEAD_) - 1)
#define TAIL_SIZE_ (sizeof(TAIL_) - 1)

#define BUFSIZE_ 4096
#define ENDOF_ (-2)

/* Maximum number of digits in a 32 bit integer. */

#define MAX_DIGITS_ 10

struct aug_fixstream_ {
    aug_fixcb_t cb_;
    struct aug_var arg_;
    aug_strbuf_t strbuf_;
    size_t mlen_;
};

static int
fixtoui_(unsigned* dst, const char* buf, size_t size, char delim)
{
    const char* it, * end = buf + size;
    int digits;
    unsigned fact, value;

    for (it = buf; it != end; ++it)
        if (*it == delim)
            goto found;

    return 0;
 found:

    /* Verify the number of digits found does not exceed the maximum number of
       valid digits. */

    digits = it - buf;
    if (MAX_DIGITS_ < digits) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("too many integer digits '%d'"),
                       (int)digits);
        return -1;
    }

    /* Given the string "1234", the integer value would be calculated as
       follows:

       4 * 1 + 3 * 10 + 2 * 100 + 1 * 1000 = 1234
     */

    for (fact = 1, value = 0; it-- != buf; fact *= 10) {

        if (!isdigit(*it)) {
            aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                           AUG_MSG("non-digit '%c' in integer"), *it);
            return -1;
        }

        value += (*it - '0') * fact;
    }

    *dst = value;
    return digits;
}

static unsigned
getsize_(const char* buf, size_t size)
{
    /* TODO: Alter the fix parsing logic so that it does not assume a
       static header with a constant begin string value. */

    int digits;
    unsigned value;

    /* In order for the buffer to contain the body length, it must be able
       to hold, at least, the standard leader, a single digit and a
       delimiter character. */

    if (size < HEAD_SIZE_ + sizeof('0') + sizeof(*SOH_))
        return 0;

    /* The beginning of the body length value is located immediately after the
       standard leader. */

    if (0 >= (digits = fixtoui_(&value, buf + HEAD_SIZE_, size, *SOH_)))
        return digits; /* -1 or 0. */

    /* Return the total number of bytes in the message. */

    return HEAD_SIZE_ + digits + sizeof(*SOH_) + value + TAIL_SIZE_;
}

static int
getsum_(const char* buf, size_t size)
{
    char cu, ct, ch;
    aug_len_t sum;

    cu = buf[size - 2]; /* Units. */
    ct = buf[size - 3]; /* Tens. */
    ch = buf[size - 4]; /* Hundreds. */

    if (!isdigit(cu) || !isdigit(ct) || !isdigit(ch)) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("non-digit in checksum '%.3s'"),
                       buf + (size - 4));
        return -1;
    }

    sum = cu - '0';
    sum += (ct - '0') * 10;
    sum += (ch - '0') * 100;

    return sum;
}

AUGNET_API aug_fixstream_t
aug_createfixstream(size_t size, aug_fixcb_t cb, const struct aug_var* arg)
{
    aug_fixstream_t stream = malloc(sizeof(struct aug_fixstream_));
    aug_strbuf_t strbuf;

    if (!stream) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    if (!(strbuf = aug_createstrbuf(0 == size ? BUFSIZE_ : size))) {
        free(stream);
        return NULL;
    }

    stream->cb_ = cb;
    aug_setvar(&stream->arg_, arg);
    stream->strbuf_ = strbuf;
    stream->mlen_ = 0;
    return stream;
}

AUGNET_API int
aug_freefixstream(aug_fixstream_t stream)
{
    int ret = aug_freestrbuf(stream->strbuf_);
    aug_freevar(&stream->arg_);
    free(stream);
    return ret;
}

AUGNET_API ssize_t
aug_readfix(aug_fixstream_t stream, int fd, size_t size)
{
    ssize_t rlen, blen, mlen;
    const char* ptr;

    /* Return on error or end of file. */

    if (0 >= (rlen = aug_readstrbuf(fd, &stream->strbuf_, size)))
        return rlen;

    /* Total number of buffered bytes. */

    blen = aug_strbuflen(stream->strbuf_);
    ptr = aug_getstr(stream->strbuf_);

    if (0 != stream->mlen_)
        goto body;

    /* Header and body parts of the message are alternately parsed until the
       entire buffer has been consumed. */

    for (;;) {

        if (-1 == (mlen = getsize_(ptr, size)))
            return -1;

        /* Not enough of the message has been read to determine the total
           message length. */

        if (0 == mlen)
            break;

        /* Cache total message length. */

        stream->mlen_ = mlen;

    body:
        if (blen < stream->mlen_)
            break;

        stream->cb_(&stream->arg_, ptr, stream->mlen_);
        blen -= stream->mlen_;
        stream->mlen_ = 0;

        if (0 == blen) {
            aug_clearstrbuf(&stream->strbuf_);
            break;
        }

        aug_setstrbufsn(&stream->strbuf_, ptr + stream->mlen_, blen);
        ptr = aug_getstr(stream->strbuf_);
    }

    return rlen;
}

AUGNET_API int
aug_endfix(aug_fixstream_t stream)
{
    /* Test for any partially buffered messages. */

    int ret;
    size_t size = aug_strbuflen(stream->strbuf_);
    if (size) {

        aug_clearstrbuf(&stream->strbuf_);
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EIO,
                       AUG_MSG("fix stream not empty, '%d' bytes"),
                       (int)size);
        ret = -1;

    } else
        ret = 0;

    stream->mlen_ = 0;
    return ret;
}

AUGNET_API int
aug_checkfix(struct aug_fixstd_* fixstd, const char* buf, size_t size)
{
    const char* ptr = buf;
    int sum1, sum2;

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ^^
     */

    if (0 != memcmp(ptr, BEGINSTRING_PREFIX_, BEGINSTRING_PREFIX_SIZE_)) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("invalid beginstring tag"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ..^^^^^^^
     */

    ptr += BEGINSTRING_PREFIX_SIZE_;
    memcpy(fixstd->fixver_, ptr, AUG_FIXVERLEN);
    fixstd->fixver_[AUG_FIXVERLEN] = '\0';

    /* 8=FIX.4.2^9=5^35=D^10=181^
       .........^
     */

    ptr += AUG_FIXVERLEN;
    if (*SOH_ != *ptr++) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("beginstring field delimiter not found"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ..........^^
     */

    if (0 != memcmp(ptr, BODYLENGTH_PREFIX_, BODYLENGTH_PREFIX_SIZE_)) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("invalid bodylength tag"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       .............^
     */

    /* Move one character past the SOH delimiter. */

    /* Note: the SOH delimiter is known to exist - getsum_() was used when
       reading the message. */

    ptr += BODYLENGTH_PREFIX_SIZE_;
    while (*SOH_ != *ptr++)
        ;

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ..............^^^^^
     */

    /* Set body pointer and size. */

    fixstd->body_ = ptr;
    ptr = buf + size - TAIL_SIZE_;
    fixstd->size_ = ptr - fixstd->body_;

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ...................^^^
     */

    if (0 != memcmp(ptr, CHECKSUM_PREFIX_, CHECKSUM_PREFIX_SIZE_)) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("invalid checksum tag"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ...................      ^
     */

    if (*SOH_ != buf[size - 1]) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("checksum field delimiter not found"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ......................^^^.
     */

    /* Get sum contained within message. */

    if (-1 == (sum1 = getsum_(buf, size)))
        return -1;

    /* Calculate message checksum. */

    sum2 = aug_checksum(buf, size - TAIL_SIZE_);

    if (sum1 != sum2) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("invalid checksum '%03d', expected '%03d'"),
                       sum1, sum2);
        return -1;
    }

    return 0;
}

AUGNET_API aug_len_t
aug_checksum(const char* buf, size_t size)
{
    const char* end = buf + size;
    unsigned long sum = 0;
    for (; buf != end; ++buf)
        sum += *buf;
    return sum % 256;
}

AUGNET_API ssize_t
aug_fixfield(struct aug_fixfield_* field, const char* buf, size_t size)
{
    const char* it, * end = buf + size;
    int digits;
    unsigned tag;

    /* Extract tag value from buffer. */

    if (-1 == (digits = fixtoui_(&tag, buf, size, '=')))
        return -1;

    if (0 == digits) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("tag not found"));
        return -1;
    }

    field->tag_ = tag;

    it = buf + digits + sizeof(*SOH_);
    field->value_ = it;

    /* Locate field delimiter. */

    for (; it != end; ++it)
        if (*it == *SOH_)
            goto found;

    aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                   AUG_MSG("field '%d' delimiter not found"),
                   (int)field->tag_);
    return -1;
 found:

    field->size_ = it - field->value_;

    /* Return number of bytes consumed. */

    return (it - buf) + sizeof(*SOH_);
}
