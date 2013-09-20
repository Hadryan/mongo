/**
*    Copyright (C) 2012 10gen Inc.
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
*/

#pragma once

#include <string>
#include <vector>

#include "mongo/base/disallow_copying.h"
#include "mongo/base/status.h"
#include "mongo/db/auth/action_set.h"
#include "mongo/db/auth/action_type.h"
#include "mongo/db/auth/authorization_manager.h"
#include "mongo/db/auth/authz_session_external_state.h"
#include "mongo/db/auth/privilege.h"
#include "mongo/db/auth/user_name.h"
#include "mongo/db/auth/user_set.h"
#include "mongo/db/namespacestring.h"

namespace mongo {

    /**
     * Contains all the authorization logic for a single client connection.  It contains a set of
     * the users which have been authenticated, as well as a set of privileges that have been
     * granted to those users to perform various actions.
     *
     * An AuthorizationSession object is present within every mongo::ClientBasic object.
     *
     * Predicate methods for checking authorization may in the worst case acquire read locks
     * on the admin database.
     */
    class AuthorizationSession {
        MONGO_DISALLOW_COPYING(AuthorizationSession);
    public:

        // Takes ownership of the externalState.
        explicit AuthorizationSession(AuthzSessionExternalState* externalState);
        ~AuthorizationSession();

        AuthorizationManager& getAuthorizationManager();

        // Should be called at the beginning of every new request.  This performs the checks
        // necessary to determine if localhost connections should be given full access.
        // TODO: try to eliminate the need for this call.
        void startRequest();

        /**
         * Adds the User identified by "UserName" to the authorization session, acquiring privileges
         * for it in the process.
         */
        Status addAndAuthorizeUser(const UserName& userName);

        // Returns the authenticated user with the given name.  Returns NULL
        // if no such user is found.
        // The user remains in the _authenticatedUsers set for this AuthorizationSession,
        // and ownership of the user stays with the AuthorizationManager
        User* lookupUser(const UserName& name);

        // Gets an iterator over the names of all authenticated users stored in this manager.
        UserSet::NameIterator getAuthenticatedUserNames();

        // Returns a string representing all logged-in users on the current session.
        // WARNING: this string will contain NUL bytes so don't call c_str()!
        std::string getAuthenticatedUserNamesToken();

        // Removes any authenticated principals whose authorization credentials came from the given
        // database, and revokes any privileges that were granted via that principal.
        void logoutDatabase(const std::string& dbname);

        // Adds the internalSecurity user to the set of authenticated users.
        // Used to grant internal threads full access.
        void grantInternalAuthorization();

        // Checks if this connection has the privileges necessary to perform the given query on the
        // given namespace.
        Status checkAuthForQuery(const NamespaceString& ns, const BSONObj& query);

        // Checks if this connection has the privileges necessary to perform a getMore on the given
        // cursor in the given namespace.
        Status checkAuthForGetMore(const NamespaceString& ns, long long cursorID);

        // Checks if this connection has the privileges necessary to perform the given update on the
        // given namespace.
        Status checkAuthForUpdate(const NamespaceString& ns,
                                  const BSONObj& query,
                                  const BSONObj& update,
                                  bool upsert);

        // Checks if this connection has the privileges necessary to insert the given document
        // to the given namespace.  Correctly interprets inserts to system.indexes and performs
        // the proper auth checks for index building.
        Status checkAuthForInsert(const NamespaceString& ns, const BSONObj& document, bool buildingSystemUsersIndex=false);

        // Checks if this connection has the privileges necessary to perform a delete on the given
        // namespace.
        Status checkAuthForDelete(const NamespaceString& ns, const BSONObj& query);

        // Returns true if this session is authorized for the given Privilege.
        //
        // Contains all the authorization logic including handling things like the localhost
        // exception.
        bool isAuthorizedForPrivilege(const Privilege& privilege);

        // Like isAuthorizedForPrivilege, above, except returns true if the session is authorized
        // for all of the listed privileges.
        bool isAuthorizedForPrivileges(const vector<Privilege>& privileges);

        // Utility function for isAuthorizedForPrivilege(Privilege(resource, action)).
        bool isAuthorizedForActionsOnResource(const ResourcePattern& resource, ActionType action);

        // Utility function for isAuthorizedForPrivilege(Privilege(resource, actions)).
        bool isAuthorizedForActionsOnResource(const ResourcePattern& resource,
                                              const ActionSet& actions);

        // Utility function for
        // isAuthorizedForActionsOnResource(ResourcePattern::forExactNamespace(ns), action).
        bool isAuthorizedForActionsOnNamespace(const NamespaceString& ns, ActionType action);

        // Utility function for
        // isAuthorizedForActionsOnResource(ResourcePattern::forExactNamespace(ns), actions).
        bool isAuthorizedForActionsOnNamespace(const NamespaceString& ns, const ActionSet& actions);

    private:

        // Checks if this connection is authorized for the given Privilege, ignoring whether or not
        // we should even be doing authorization checks in general.  Note: this may acquire a read
        // lock on the admin database (to update out-of-date user privilege information).
        bool _isAuthorizedForPrivilege(const Privilege& privilege);

        scoped_ptr<AuthzSessionExternalState> _externalState;

        // All Users who have been authenticated on this connection
        UserSet _authenticatedUsers;
    };

} // namespace mongo