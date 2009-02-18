# Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>
#
# This file is part of Aug written by Mark Aylett.
#
# Aug is released under the GPL with the additional exemption that compiling,
# linking, and/or using OpenSSL is allowed.
#
# Aug is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# Aug is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

module Log
    def self.crit(s)
        AugRb.writelog(AugRb::LOGCRIT, s)
    end
    def self.error(s)
        AugRb.writelog(AugRb::LOGERROR, s)
    end
    def self.warn(s)
        AugRb.writelog(AugRb::LOGWARN, s)
    end
    def self.notice(s)
        AugRb.writelog(AugRb::LOGNOTICE, s)
    end
    def self.info(s)
        AugRb.writelog(AugRb::LOGINFO, s)
    end
    def self.debug(s)
        AugRb.writelog(AugRb::LOGDEBUG, s)
    end
end
