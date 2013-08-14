/** @file admin_access.h
 */

/**
*    Copyright (C) 2010 10gen Inc.
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*    As a special exception, the copyright holders give permission to link the
*    code of portions of this program with the OpenSSL library under certain
*    conditions as described in each individual source file and distribute
*    linked combinations including the program with the OpenSSL library. You
*    must comply with the GNU Affero General Public License in all respects
*    for all of the code used other than as permitted herein. If you modify
*    file(s) with this exception, you may extend this exception to your
*    version of the file(s), but you are not obligated to do so. If you do not
*    wish to do so, delete this exception statement from your version. If you
*    delete this exception statement from all source files in the program,
*    then also delete it here.
*/

#pragma once

#include "mongo/db/jsobj.h"
#include "mongo/db/auth/user_name.h"

namespace mongo {

    /*
     * An AdminAccess is an interface class used to determine if certain users have
     * privileges to a given resource.
     *
     */
    class AdminAccess {
    public:
        virtual ~AdminAccess() { }

        /** @return if there are any priviledge users. This should not
         *          block for long and throw if can't get a lock if needed.
         */
        virtual bool haveAdminUsers() const = 0;

        /** @return privileged user with this name. This should not block
         *          for long and throw if can't get a lock if needed
         */
        virtual BSONObj getAdminUser(const UserName& username) const = 0;
    };

    class NoAdminAccess : public AdminAccess {
    public:
        virtual ~NoAdminAccess() { }

        virtual bool haveAdminUsers() const { return false; }
        virtual BSONObj getAdminUser(const UserName& username) const { return BSONObj(); }
    };

}  // namespace mongo
