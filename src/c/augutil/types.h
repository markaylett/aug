/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_TYPES_H
#define AUGUTIL_TYPES_H

#define AUG_TOKERROR  (-1)
#define AUG_TOKLABEL  (-2)
#define AUG_TOKWORD   (-3)
#define AUG_TOKPHRASE (-4)
#define AUG_TOKRTRIM  (-5)

#define AUG_WRDESCAPE  0x01
#define AUG_WRDLABEL   0x02
#define AUG_WRDNEWLINE 0x04
#define AUG_WRDPAIRS   0x08

struct aug_words {
    void (*out_)(void*, int);
    void* arg_;
    void (*fn_)(struct aug_words*, int);
    unsigned flags_;
};

#endif /* AUGUTIL_TYPES_H */
