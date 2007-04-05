/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/tmspec.h"
#include "augsys/defs.h"

AUG_RCSID("$Id:$");

#include <time.h>
#include <string.h> /* memcpy() */

#define FIXED_(x) (-1 != (x))

struct tmspec {
    int min_, hour_, mday_, mon_, wday_;
};

static const int MDAYS_[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static int
isleap_(int year)
{
    return year % 4 && (0 != year % 100 || 0 == year % 400);
}

static int
mdays_(int mon, int year)
{
    return 1 == mon && isleap_(year) ? 29 : MDAYS_[mon];
}

static int
mday_(const struct tm* tm)
{
    int last = mdays_(tm->tm_mon, tm->tm_year + 1900) - 1;
    return last < tm->tm_mday ? last : tm->tm_mday;
}

static void
nextmon_(struct tm* tm, const struct tmspec* tms)
{
    if (FIXED_(tms->mon_) || 0 == (tm->tm_mon = (tm->tm_mon + 1) % 12))
        ++tm->tm_year;
}

static void
nextmday_(struct tm* tm, const struct tmspec* tms)
{
    if (FIXED_(tms->mday_))
        nextmon_(tm, tms);
    else {
        int n = mdays_(tm->tm_mon, tm->tm_year + 1900);
        if (0 == (tm->tm_mday = (tm->tm_mday + 1) % n))
            nextmon_(tm, tms);
    }
}

static void
nextmhour_(struct tm* tm, const struct tmspec* tms)
{
    if (FIXED_(tms->hour_) || 0 == (tm->tm_hour = (tm->tm_hour + 1) % 24))
        nextmday_(tm, tms);
}

static void
nextmmin_(struct tm* tm, const struct tmspec* tms)
{
    if (FIXED_(tms->min_) || 0 == (tm->tm_min = (tm->tm_min + 1) % 60))
        nextmhour_(tm, tms);
}

static void
nextwday_(struct tm* tm, const struct tmspec* tms)
{
    tm->tm_mday += FIXED_(tms->wday_) ? 7 : 1;
}

static void
nextwhour_(struct tm* tm, const struct tmspec* tms)
{
    if (FIXED_(tms->hour_) || 0 == (tm->tm_hour = (tm->tm_hour + 1) % 24))
        nextwday_(tm, tms);
}

static void
nextwmin_(struct tm* tm, const struct tmspec* tms)
{
    if (FIXED_(tms->min_) || 0 == (tm->tm_min = (tm->tm_min + 1) % 60))
        nextwhour_(tm, tms);
}

static void
nextmtime_(struct tm* tm, const struct tmspec* tms)
{
    struct tm out = { 0 };

    if (FIXED_(tms->min_))
        out.tm_min = tms->min_;

    if (FIXED_(tms->hour_))
        out.tm_hour = tms->hour_;

    if (FIXED_(tms->mday_))
        out.tm_mday = tms->mday_;

    if (FIXED_(tms->mon_))
        out.tm_mon = tms->mon_;

    out.tm_year = tm->tm_year;

    if (FIXED_(tms->mon_)) {

        if (out.tm_mon != tm->tm_mon) {

            if (out.tm_mon < tm->tm_mon)
                ++out.tm_year;

            goto done;
        }

    } else
        out.tm_mon = tm->tm_mon;

    if (FIXED_(tms->mday_)) {

        int mday = mday_(&out);

        if (mday != tm->tm_mday) {

            if (mday < tm->tm_mday)
                nextmon_(&out, tms);

            goto done;
        }

    } else
        out.tm_mday = tm->tm_mday;

    if (FIXED_(tms->hour_)) {

        if (out.tm_hour != tm->tm_hour) {

            if (out.tm_hour < tm->tm_hour)
                nextmday_(&out, tms);

            goto done;
        }

    } else
        out.tm_hour = tm->tm_hour;

    if (FIXED_(tms->min_)) {

        if (out.tm_min != tm->tm_min) {

            if (out.tm_min < tm->tm_min)
                nextmhour_(&out, tms);

            goto done;
        }

    } else
        out.tm_min = tm->tm_min;

    if (out.tm_sec < tm->tm_sec)
        nextmmin_(&out, tms);

 done:
    out.tm_mday = mday_(&out);
    memcpy(tm, &out, sizeof(out));
}

static void
nextwtime_(struct tm* tm, const struct tmspec* tms)
{
    struct tm out = { 0 };

    if (FIXED_(tms->min_))
        out.tm_min = tms->min_;

    if (FIXED_(tms->hour_))
        out.tm_hour = tms->hour_;

    out.tm_mday = tm->tm_mday;
    out.tm_mon = tm->tm_mon;
    out.tm_year = tm->tm_year;

    if (FIXED_(tms->wday_)) {

        int wdays = (7 + tms->wday_ - tm->tm_wday) % 7;
        if (wdays) {

            out.tm_mday += wdays;
            goto done;
        }
    }

    if (FIXED_(tms->hour_)) {

        if (out.tm_hour != tm->tm_hour) {

            if (out.tm_hour < tm->tm_hour)
                nextwday_(&out, tms);

            goto done;
        }

    } else
        out.tm_hour = tm->tm_hour;

    if (FIXED_(tms->min_)) {

        if (out.tm_min != tm->tm_min) {

            if (out.tm_min < tm->tm_min)
                nextwhour_(&out, tms);

            goto done;
        }

    } else
        out.tm_min = tm->tm_min;

    if (out.tm_sec < tm->tm_sec)
        nextwmin_(&out, tms);

 done:
    memcpy(tm, &out, sizeof(out));
}

AUGUTIL_API struct tmspec*
aug_strtmspec(struct tmspec* tms, const char* s)
{
    char ch;
    int x = 0;
    tms->min_ = tms->hour_ = tms->mday_ = tms->mon_ = tms->wday_ = -1;
    while ('\0' != (ch = *s++)) {
        switch (ch) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            x *= 10;
            x += ch - '0';
            break;
        case 'H':
            tms->hour_ = x;
            x = 0;
            break;
        case 'M':
            tms->min_ = x;
            x = 0;
            break;
        case 'd':
            tms->mday_ = x;
            x = 0;
            break;
        case 'm':
            tms->mon_ = x;
            x = 0;
            break;
        case 'w':
            tms->wday_ = x;
            x = 0;
            break;
        default:
            return NULL;
        }
    }

    /* Convert month elements to zero-based indexes. */

    switch (tms->mday_) {
    case -1:
        break;
    case 0:
        return NULL;
    default:
        --tms->mday_;
    }

    switch (tms->mon_) {
    case -1:
        break;
    case 0:
        return NULL;
    default:
        --tms->mon_;
    }

    /* Treat 7 as Sunday. */

    if (7 == tms->wday_)
        tms->wday_ = 0;

    /* Validate ranges. */

    return tms->min_ < 60 && tms->hour_ < 24 && tms->mday_ < 31
        && tms->mon_ < 12 && tms->wday_ < 7 ? tms : NULL;
}

AUGUTIL_API struct tm*
aug_nexttime(struct tm* tm, const struct tmspec* tms)
{
    void (*fn)(struct tm*, const struct tmspec*) = NULL;

    if (FIXED_(tms->wday_)) {

        /* For simplicity, combined monthly and weekly specifications are not
           supported. */

        if (FIXED_(tms->mday_) || FIXED_(tms->mon_))
            return NULL;

        fn = nextwtime_;

    } else
        fn = nextmtime_;

    /* Avoid expiry time exactly matching current time. */

    if (0 == tm->tm_sec)
        tm->tm_sec = 1;

    /* Convert day-of-month to zero-based index. */

    --tm->tm_mday;
    fn(tm, tms);
    ++tm->tm_mday;
    return tm;
}
