/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_TYPES_H
#define AUGUTIL_TYPES_H

#define AUG_LEXESCAPE  0x01
#define AUG_LEXLABEL   0x02
#define AUG_LEXNEWLINE 0x04

/** Start from -2 to avoid confusion with ERROR and EOF. */

#define AUG_TOKPHRASE (-2)
#define AUG_TOKLABEL  (-3)
#define AUG_TOKWORD   (-4)
#define AUG_TOKRTRIM  (-5)

struct aug_words {
    void (*out_)(void*, int);
    void* arg_;
    void (*fn_)(struct aug_words*, int);
    unsigned flags_;
};

#endif /* AUGUTIL_TYPES_H */
