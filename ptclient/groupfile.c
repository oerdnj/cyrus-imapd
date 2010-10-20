/*
 * Copyright (c) 1994-2008 Carnegie Mellon University.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any legal
 *    details, please contact
 *      Carnegie Mellon University
 *      Center for Technology Transfer and Enterprise Creation
 *      4615 Forbes Avenue
 *      Suite 302
 *      Pittsburgh, PA  15213
 *      (412) 268-7393, fax: (412) 268-7395
 *      innovation@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <sys/syslog.h>
#include <grp.h>
#include "ptloader.h"
#include "util.h"

/* libconfig */
#include "libconfig.h"

/* libcyrus */
#include "auth_pts.h"
#include "strhash.h"
#include "xmalloc.h"

static struct group* fgetgrnam(const char* name)
{
    struct group *grp;
    FILE *groupfile;

    groupfile = fopen("/etc/imapd.group", "r");
    if (!groupfile) groupfile = fopen("/etc/group", "r");
    if (groupfile) {
       while ((grp = fgetgrent(groupfile))) {
         if (strcmp(grp->gr_name, name) == 0) {
           fclose(groupfile);
           return grp;
         }
       }
    }
    if (groupfile) fclose(groupfile);
    return NULL;
}

/*
 * Convert 'identifier' into canonical form.
 * Returns a pointer to a static buffer containing the canonical form
 * or NULL if 'identifier' is invalid.
 *
 * XXX If any of the characters marked with 0 are valid and are cropping up,
 * the right thing to do is probably to canonicalize the identifier to two
 * representations: one for getpwent calls and one for folder names.  The
 * latter canonicalizes to a MUTF7 representation.
 */
static char *ptsmodule_canonifyid(const char *identifier, size_t len)
{
    static char retbuf[81];
    char sawalpha;
    char *p;
    int username_tolower = 0;
    int i = 0;

    if(!len) len = strlen(identifier);
    if(len >= sizeof(retbuf)) return NULL;

    memcpy(retbuf, identifier, len);
    retbuf[len] = '\0';

    if (!strncmp(retbuf, "group:", 6))
        i = 6;

    /* Copy the string and look up values in the allowedchars array above.
     * If we see any we don't like, reject the string.
     * Lowercase usernames if requested.
     */
    username_tolower = config_getswitch(IMAPOPT_USERNAME_TOLOWER);
    sawalpha = 0;
    for(p = retbuf+i; *p; p++) {
        if (username_tolower && Uisupper(*p))
            *p = tolower((unsigned char)*p);
    }

    if (!sawalpha) return NULL;  /* has to be one alpha char */

    return retbuf;
}

static void myinit(void)
{
    return;
}

static struct auth_state *myauthstate(
        const char *identifier,
        size_t size,
        const char **reply,
        int *dsize
    )
{

    const char *canon_id;
    struct auth_state *newstate = NULL;
    int rc;

    canon_id = ptsmodule_canonifyid(identifier, size);
    if (canon_id) {
        *reply = "ptsmodule_canonifyid() failed";
        return NULL;
    }

    size = strlen(canon_id);

    *reply = NULL;
    newstate = fgetgrnam(canon_id);

    return newstate;
}

struct pts_module pts_groupfile =
{
    "groupfile",     /* The name of this module */

    &myinit,
    &myauthstate,
};
