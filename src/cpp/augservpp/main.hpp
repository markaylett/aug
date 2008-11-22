/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERVPP_MAIN_HPP
#define AUGSERVPP_MAIN_HPP

#include "augservpp/config.hpp"

#include "augserv/main.h"

namespace aug {

    /**
     * On Windows, the Service Manager calls the service entry point on a
     * separate thread - automatic variables on the main thread's stack will
     * not be visible from the service thread.  A shallow copy of the service
     * structure is, therefore, performed by aug_main().
     */

    inline int
    main(int argc, char* argv[], const struct aug_serv& serv)
    {
        return aug_main(argc, argv, &serv);
    }
}

#endif // AUGSERVPP_MAIN_HPP
