
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file newyork.hpp

    \fullpath
    Include/ql/Calendars/%newyork.hpp
    \brief New York calendar

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 14:02:31  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.8  2001/08/31 15:23:45  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.7  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.6  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.5  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.4  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.3  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_newyork_calendar_h
#define quantlib_newyork_calendar_h

#include "ql/Calendars/westerncalendar.hpp"

namespace QuantLib {

    namespace Calendars {

        //! New York calendar
        /*! Holidays:
            <ul>
            <li>Saturdays</li>
            <li>Sundays</li>
            <li>New Year's Day, January 1st (possibly moved to Monday if
                actually on Sunday, or to Friday if on Saturday)</li>
            <li>Martin Luther King's birthday, third Monday in January</li>
            <li>Washington's birthday, third Monday in February</li>
            <li>Memorial Day, last Monday in May</li>
            <li>Independence Day, July 4th (moved to Monday if Sunday or Friday
                if Saturday)</li>
            <li>Labor Day, first Monday in September</li>
            <li>Columbus Day, second Monday in October</li>
            <li>Veteran's Day, November 11th (moved to Monday if Sunday or
                Friday if Saturday)</li>
            <li>Thanksgiving Day, fourth Thursday in November</li>
            <li>Christmas, December 25th (moved to Monday if Sunday or Friday if
                Saturday)</li>
            </ul>
        */
        class NewYork : public WesternCalendar {
          public:
            NewYork() {}
            std::string name() const { return "NewYork"; }
            bool isBusinessDay(const Date&) const;
        };

    }

}


#endif
