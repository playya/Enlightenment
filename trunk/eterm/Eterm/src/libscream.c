/****************************************************************************
 * scream::libscream.c                         Azundris <scream@azundris.com>
 *
 * routines for terminal emulators to connect to screen and/or scream daemons.
 * libscream is a double-transparency layer -- it abstracts the backend
 * (screen or a replacement, locally or ssh-tunneled) to the front-end
 * (a terminal-emulation such as Eterm, konsole, or multi-gnome-terminal)
 * and vice versa.  several sessions can be open at once.
 *
 * Lesser GNU Public Licence applies.
 * Thread-safe:  untested
 *
 * 2002/04/19  Azundris  incept
 * 2002/05/04  Azundris  support for esoteric screens, thanks to Till
 * 2002/05/12  Azundris  edit display names, send statement, tab completion
 * 2002/05/13  Azundris  ssh tunnel through firewall
 * 2002/05/17  Azundris  supports systemwide screenrc (thanks mej)
 * 2002/05/18  Azundris  remote handling improved (thanks tillsan, tfing)
 * 2002/05/21  Azundris  code restructuring, basic tab tear-off
 * 2002/06/04  Azundris  advanced tab tear-off
 * 2002/06/05  Azundris  basic twin support
 ***************************************************************************/



#undef NS_DEBUG

#include <stdio.h>              /* stderr, fprintf, snprintf() */
#include <string.h>             /* bzero() */
#include <pwd.h>                /* getpwuid() */
#include <sys/types.h>          /* getpwuid() */
#include <sys/stat.h>           /* stat() */
#include <unistd.h>             /* getuid() */
#include <stdlib.h>             /* atoi() */
#include <netdb.h>              /* getservbyname() */
#include <netinet/in.h>         /* ntohs() */
#include <limits.h>             /* PATH_MAX */
#include <ctype.h>              /* isspace() */
#include <errno.h>              /* errno */

#include "config.h"
#include "feature.h"

/* use libast if we have it */
#ifdef DEBUG_ESCREEN
#  include <libast.h>
#else
#  define MALLOC(a) malloc(a)
#  define FREE(a) free(a)
#  define STRDUP(a) strdup(a)
#  ifdef NS_DEBUG
#    define D_ESCREEN(a)  fprintf(stderr,a);
#  else
#    define D_ESCREEN(a)
#  endif
#endif

#include "scream.h"             /* structs, defs, headers */
#include "screamcfg.h"          /* user-tunables */

#ifdef NS_HAVE_TWIN
#  include <Tw/Tw.h>
#  include <Tw/Tw_1.h>
#endif

#ifndef MAXPATHLEN
#  ifdef PATH_MAX
#    define MAXPATHLEN PATH_MAX
#  elif defined(MAX_PATHLEN)
#    define MAXPATHLEN MAX_PATHLEN
#  endif
#endif



/***************************************************************************/
/* module-global vars */
/**********************/



static long err_inhibit = 0;    /* bits. avoid telling same error twice. */
static _ns_sess *sa = NULL;     /* anchor for session list */
static _ns_hop *ha = NULL;      /* anchor for hop list */



/***************************************************************************/
/* forward declarations */
/************************/



static void ns_desc_hop(_ns_hop *, char *);
static int ns_parse_screenrc(_ns_sess *, char *, int);
static int ns_mov_screen_disp(_ns_sess *, int, int);
static _ns_sess *ns_dst_sess(_ns_sess **);
static int ns_inp_tab(void *, char *, size_t, size_t);



/****************************************************************************
           _     _     _ _          _                       
 _ __ ___ (_) __| | __| | | ___    | | __ _ _   _  ___ _ __ 
| '_ ` _ \| |/ _` |/ _` | |/ _ \   | |/ _` | | | |/ _ \ '__|
| | | | | | | (_| | (_| | |  __/   | | (_| | |_| |  __/ |   
|_| |_| |_|_|\__,_|\__,_|_|\___|   |_|\__,_|\__, |\___|_|   
                                            |___/

central abstraction layer

  this abstracts the front-end (terminal emulator) against the back-end
  (local or remote terminal server), and the back-end against the front-end:

  - front-end hands us an URL we attach to (without knowing about the backend)
  - CAL receives messages from back-end and calls the external function (efun)
    the front-end registered for this event
  - CAL functions are called from the front-end and send data fitting the
    session-type to the backend
*/



/* test if we have a valid callback for function-type "e".
  !p  a variable of the "_ns_efuns *" type.  will contain a pointer to
      an efun struct containing a function pointer to the requested function
      if such a struct exists, or NULL, if it doesn't exist
   s  a variable of the "_ns_sess *" type, or NULL (see ns_get_efuns())
   d  a variable of the "_nd_disp *" type, or NULL (see ns_get_efuns())
   e  the name of an element of "_ns_efuns"
  !<- conditional execution of next (compound-) statement (which would
      normally be (p)->(e)(...), the call of the function e).
 */
#define NS_EFUN_EXISTS(p,s,d,e) (((p)=ns_get_efuns((s),(d)))&&((p)->e))



/***************************************************************************/
/* constructors/destructors */
/****************************/



/* ns_free
   free a string (or whatever) */

static void
*
ns_free(char **x)
{
    if (x && !*x) {
        FREE(*x);
        *x = NULL;
    }
    return NULL;
}



/* ns_new_hop.  create and initialize a hop struct.
   lp    local port.  if 0:  if otherwise matching hop exists, reuse that.
                             otherwise, find the first FREE (as in, not used
                             by us) port, starting with NS_MIN_PORT.
   fw    firewall machine.  numeric or symbolic.
   fp    foreign port. if 0: default to SSH port.
   delay wait n seconds for tunnel to come up before trying to use it
   s     the session to add the hop to
   <-    a matching (existing or newly created) hop structure, or NULL */

static _ns_hop *
ns_new_hop(int lp, char *fw, int fp, int delay, _ns_sess * s)
{
    _ns_hop *h = ha;

    if (!fw || !*fw)
        return NULL;

    if (!fp)
        fp = ns_get_ssh_port(); /* remote port defaults to SSH */

    if (s) {
        /* see if we already have a matching hop. */
        while (h && !(((h->localport == lp) || (!lp)) && (!strcmp(h->fw, fw)) && (h->fwport == fp) && (h->sess->port == s->port) && (!strcmp(h->sess->host, s->host))))
            h = h->next;

        if (h) {
            if (delay)
                h->delay = delay;       /* may change delay! */
            h->refcount++;
            return h;
        }
    }

    h = MALLOC(sizeof(_ns_hop));
    if (h) {
        bzero(h, sizeof(_ns_hop));
        if ((h->fw = STRDUP(fw))) {
            if (!lp) {
                lp = NS_MIN_PORT;       /* local port defaults to */
                if (ha) {       /* NS_MIN_PORT. if that's */
                    int f;      /* taken, use next FREE port. */

                    do {        /* FREE as in, not used by us. */
                        _ns_hop *i = ha;

                        f = 0;
                        while (i)
                            if (i->localport == lp) {
                                f = 1;
                                lp++;
                                i = NULL;
                            } else
                                i = i->next;
                    } while (f);
                }
            }
            h->delay = (delay ? delay : NS_TUNNEL_DELAY);
            h->localport = lp;
            h->fwport = fp;
            h->refcount++;
            h->next = ha;
            h->sess = s;
            ha = h;
        } else {
            FREE(h);
            return NULL;
        }
    }

    return h;
}



/* ns_dst_hop.  deref (and, where necessary, release) a hop struct.
   if sp is provided, additional integrity magic will take place.
   ss  hop to deref/FREE
   sp  session that the hop used to belong to (NULL for none (as if))
   <-  NULL */

static _ns_hop *
ns_dst_hop(_ns_hop ** ss, _ns_sess * sp)
{
    if (ss && *ss) {
        _ns_hop *s = *ss;

        if (s->refcount <= 0) {
            D_ESCREEN(("ns_dst_hop: leak alert -- trying to double-FREE hop...\n"));
            return NULL;
        }

        if (!--(s->refcount)) { /* was last ref to hop => FREE hop */
            if (s->fw)
                FREE(s->fw);
            if (ha == s)        /* delist */
                ha = s->next;
            else {
                _ns_hop *h = ha;

                while (h && h->next != s)
                    h = h->next;
                if (h)
                    h->next = s->next;
            }
            FREE(s);
        } else if (sp && sp->hop == s) {
            /* hop shouldn't point back at a session that just dereffed it
               as it's probably about to die. fix the back ref to a session
               that's actually valid. */
            _ns_sess *p = sa;

            while (p && ((p == sp) || (p->port != sp->port) || (strcmp(p->host, sp->host))))
                p = p->next;
            if (!p)
                ns_desc_hop(s, NS_PREFIX "ns_dst_sess: Leak alert -- found a hop that is only\n referenced once, but has a refcount > 1. Hop data follow");
            else
                s->sess = p;
        }
        *ss = NULL;
    }
    return NULL;
}



_ns_efuns *
ns_new_efuns(void)
{
    _ns_efuns *s = MALLOC(sizeof(_ns_efuns));

    if (s) {
        bzero(s, sizeof(_ns_efuns));
    }
    return s;
}

static _ns_efuns *
ns_ref_efuns(_ns_efuns ** ss)
{
    if (ss && *ss) {
        (*ss)->refcount++;
        return *ss;
    }
    return NULL;
}

_ns_efuns *
ns_dst_efuns(_ns_efuns ** ss)
{
    if (ss && *ss) {
        _ns_efuns *s = *ss;

        *ss = NULL;
        if (!--(s->refcount)) {
            FREE(s);
        }
    }
    return NULL;
}



static _ns_disp *
ns_new_disp(void)
{
    _ns_disp *s = MALLOC(sizeof(_ns_disp));

    if (s) {
        bzero(s, sizeof(_ns_disp));
    }
    return s;
}

static _ns_disp *
ns_dst_disp(_ns_disp ** ss)
{
    if (ss && *ss) {
        _ns_disp *s = *ss;

        if (s->name)
            FREE(s->name);
        if (s->efuns)
            ns_dst_efuns(&(s->efuns));
        if (s->child)           /* nested screen? */
            ns_dst_sess(&(s->child));
        *ss = NULL;
        FREE(s);
    }
    return NULL;
}

static _ns_disp *
ns_dst_dsps(_ns_disp ** ss)
{
    if (ss && *ss) {
        _ns_disp *s = *ss, *t;

        *ss = NULL;
        do {
            t = s->next;
            ns_dst_disp(&s);
            s = t;
        } while (s);
    }
    return NULL;
}



_ns_sess *
ns_1st_sess(void)
{
    return sa;
}



static _ns_sess *
ns_new_sess(void)
{
    _ns_sess *s = MALLOC(sizeof(_ns_sess));

    if (s) {
        bzero(s, sizeof(_ns_sess));
        s->escape = NS_SCREEN_ESCAPE;   /* default setup for the screen program */
        s->literal = NS_SCREEN_LITERAL; /* set even ifndef NS_HAVE_SCREEN */
        s->dsbb = NS_SCREEN_DEFSBB;
        s->delay = NS_INIT_DELAY;
        s->fd = -1;
        if (sa) {               /* add to end of list */
            _ns_sess *r = sa;

            while (r->next)
                r = r->next;
            r->next = s;
        } else
            sa = s;
    }
    return s;
}

static _ns_sess *
ns_dst_sess(_ns_sess ** ss)
{
    if (ss && *ss) {
        _ns_sess *s = *ss;

        ns_dst_dsps(&(s->dsps));
        if (s->hop)
            ns_dst_hop(&(s->hop), s);
        if (s->host)
            FREE(s->host);
        if (s->user)
            FREE(s->user);
        if (s->pass)
            FREE(s->pass);
#ifdef NS_HAVE_TWIN
        if (s->twin_str)
            FREE(s->twin_str);
#endif
        if (s->efuns)
            ns_dst_efuns(&(s->efuns));
        if (s->prvs)
            s->prvs->next = s->next;
        else
            sa = s->next;       /* align anchor */
        if (s->next)
            s->next->prvs = s->prvs;
        *ss = NULL;
        FREE(s);
    }
    return NULL;
}




/***************************************************************************/
/* display-list handling */
/*************************/



/* ns_magic_disp
   if we pass one or both of session and display info to this, both
   elements will be set up with sensible data afterwards.  shouldn't
   fail unless the lists are major corrupted or no info is supplied.
   s  a session pointer's address
   d  a display pointer's address
   <- NS_SUCC if we could set up the info */

int
ns_magic_disp(_ns_sess ** s, _ns_disp ** d)
{
    if (!d)
        return NS_FAIL;

    if (*d) {
        (*d)->sess->curr = *d;
        if (s && !*s)
            (*s) = (*d)->sess;
#ifdef NS_PARANOID
        else if (s && *s != (*d)->sess) {
            D_ESCREEN(("ns_magic_disp: was given a disp and a session that do not belong (\n"));
            return NS_FAIL;
        }
#endif
        return NS_SUCC;
    } else if (s && *s) {
        if (!(*s)->curr) {
            if ((*s)->curr = (*s)->dsps)
                return NS_SUCC;
        } else
            return NS_SUCC;
    }
    return NS_FAIL;
}



/* we need a certain display struct (index n in session s).
   give it to us if it exists.
   s  the session the display should be in
   n  the index of the display (>=0).  displays in a session are sorted
      by index, but may be sparse (0, 1, 3, 7)
   <- the requested display */

static _ns_disp *
disp_fetch(_ns_sess * s, int n)
{
    _ns_disp *e = NULL, *c;

    for (c = s->dsps; c && (c->index < n); c = c->next)
        e = c;
    if (c && (c->index == n))   /* found it */
        return c;
    return NULL;
}



/* we need a certain display struct (index n in session s).
   give it to us.  if you can't find it, make one up and insert it into
   the list.
   s  the session the display should be in
   n  the index of the display (>=0).  displays in a session are sorted
      by index, but may be sparse (0, 1, 3, 7)
   <- the requested display */

static _ns_disp *
disp_fetch_or_make(_ns_sess * s, int n)
{
    _ns_disp *d, *e = NULL, *c;

    for (c = s->dsps; c && (c->index < n); c = c->next)
        e = c;

    if (c && (c->index == n))   /* found it */
        return c;

    if (!(d = ns_new_disp()))   /* not there, create new */
        return NULL;            /* can't create, fail */

    d->index = n;

    if ((d->next = c))          /* if not last element... */
        c->prvs = d;
    if ((d->prvs = e))          /* if not first element */
        e->next = d;
    else                        /* make first */
        s->dsps = d;

    d->sess = s;                /* note session on display */

    if (!d->sess->curr)         /* note as current on session if first display */
        d->sess->curr = d;

    return d;
}



/* get element number from screen-index (latter is sparse, former ain't)
   screen   the session in question
   n        the index screen gave us (sparse)
   <-       the real index (element number in our list of displays) */

int
disp_get_real_by_screen(_ns_sess * screen, int n)
{
    _ns_disp *d2 = screen->dsps;
    int r = 0;

    while (d2 && d2->index != n) {
        d2 = d2->next;
        r++;
    }
#ifdef NS_PARANOID
    if (!d2)
        return -1;
#endif
    return r;
}



/* get screen-index from element number (former is sparse, latter ain't)
   screen   the session in question
   n        the real index (element number in our list of displays)
   <-       the index screen knows (sparse) */

int
disp_get_screen_by_real(_ns_sess * screen, int r)
{
    _ns_disp *d2 = screen->dsps;

    while (d2 && (r-- > 0))
        d2 = d2->next;
#ifdef NS_PARANOID
    if (!d2)
        return -1;
#endif
    return d2->index;
}



/* remove a display from the internal list and release its struct and data
   disp  the display in question */

static void
disp_kill(_ns_disp * d3)
{
    if (d3->prvs) {
        d3->prvs->next = d3->next;
        if (d3->sess->curr == d3)
            d3->sess->curr = d3->prvs;
    } else {
        d3->sess->dsps = d3->next;
        if (d3->sess->curr == d3)
            d3->sess->curr = d3->next;
    }
    if (d3->next)
        d3->next->prvs = d3->prvs;
    ns_dst_disp(&d3);
}



/***************************************************************************/
/* attach/detach */
/*****************/



/* ns_sess_init
   init an opened session (transmit .screenrc, or whatever)
   sess  the session
   <-    error code */

int
ns_sess_init(_ns_sess * sess)
{
#ifdef NS_HAVE_SCREEN
    if ((sess->backend == NS_MODE_NEGOTIATE) || (sess->backend == NS_MODE_SCREEN)) {
        (void) ns_parse_screenrc(sess, sess->sysrc, NS_ESC_SYSSCREENRC);
        return ns_parse_screenrc(sess, sess->home, NS_ESC_SCREENRC);
    }
#endif
    return NS_SUCC;
}



/* return port number for service SSH (secure shell).
   <-  a port number -- 22 in all likelihood. */

int
ns_get_ssh_port(void)
{
    static int port = 0;
    struct servent *srv;

    if (port)
        return port;
    /* (fixme) replace with getservbyname_r on systems that have it */
    srv = getservbyname("ssh", "tcp");
    return (port = (srv ? ntohs(srv->s_port) : NS_DFLT_SSH_PORT));
}




/* ns_parse_hop
   parse a hop-string into a hop-struct
   h:   one of NULL lclport:fw:fwport fw:fwport lclport:fw
        if set, describes how to tunnel through a fw to access an URL
        describing a target behind said firewall
   <-   a hop struct, or NULL
*/

static _ns_hop *
ns_parse_hop(_ns_sess * s, char *h)
{
    char *p = h, *e;
    int f = 0, v, lp = 0, fp = 0, delay = 0;

    if (!h || !*h)
        return NULL;

    if ((e = strrchr(h, ','))) {
        *(e++) = '\0';
        if (*e)
            delay = atoi(e);
    }

    while (*p && *p != ':')
        if (!isdigit(*(p++)))
            f = 1;

    if (!*p)                    /* fw only */
        return ns_new_hop(lp, h, fp, delay, s);

    if (!f) {                   /* lp:fw... */
        if (!(v = atoi(h)))
            return NULL;
        lp = v;
        e = ++p;
        while (*e && *e != ':')
            e++;
        if (*e) {
            *(e++) = '\0';
            if (!(v = atoi(e)))
                return NULL;
            fp = v;
        }
    } else {                    /* fw:fp */
        *(p++) = '\0';
        if (!(v = atoi(p)))
            return NULL;
        fp = v;
        p = h;
    }
    return ns_new_hop(lp, p, fp, delay, s);
}



/* ns_desc_string
   c        the string
   doc      context-info
   !stdout  the string, in human-readable form */

static void
ns_desc_string(char *c, char *doc)
{
    char *p = c;
    char buff[1024], *pbuff;
    size_t len, n;

    len = sizeof(buff);
    pbuff = buff;

    if (doc) {
        n = snprintf(pbuff, len, "%s: ", doc);
        len -= n;
        pbuff += n;
    }

    if (!c) {
        snprintf(pbuff, len, "NULL\n");
        D_ESCREEN(("%s", buff));
        return;
    } else if (!*c) {
        snprintf(pbuff, len, "empty\n");
        D_ESCREEN(("%s", buff));
        return;
    }

    while (*p) {
        if (*p < ' ') {
            snprintf(pbuff, len, "^%c", *p + 'A' - 1);
            pbuff += 2;
            len -= 2;
        } else {
            snprintf(pbuff++, len--, "%c", *p);
        }
        p++;
    }

    D_ESCREEN(("%s\n", buff));

    return;
}



/* ns_desc_hop
   print basic info about a hop (tunnel, firewall).  mostly for debugging.
   hop:    a hop struct as generated by (eg) ns_attach_by_URL()
   doc:    info about the context
 ! stderr: info about the hop */

static void
ns_desc_hop(_ns_hop * h, char *doc)
{
    if (!h && doc) {
        D_ESCREEN(("%s: ns_desc_hop called with broken pointer!\n", doc));
        return;
    }

    if (doc)
        D_ESCREEN(("%s:\n", doc));

    D_ESCREEN(("tunnel from localhost:%d to %s:%d to %s:%d is %s.  (delay %d, %d ref%s)\n",
               h->localport, h->fw, h->fwport, h->sess->host, h->sess->port, h->established ? "up" : "down", h->delay, h->refcount, h->refcount == 1 ? "" : "s"));
}



/* ns_desc_sess
   print basic info about a session.  mostly for debugging.
   sess:   a session struct as generated by (eg) ns_attach_by_URL()
   doc:    info about the context
 ! stderr: info about the session */

static void
ns_desc_sess(_ns_sess * sess, char *doc)
{
    if (!sess) {
        D_ESCREEN(("%s: ns_desc_sess called with broken pointer!\n", doc));
        return;
    }
    if (sess->where == NS_LCL)
        D_ESCREEN(("%s: (efuns@%p)\t (user %s) local %s %s\n", doc, sess->efuns, sess->user, sess->proto, sess->rsrc));
    else {
        D_ESCREEN(("%s: (efuns@%p)\t %s://%s%s%s@%s:%s/%s\n",
                   doc, sess->efuns, sess->proto ? sess->proto : "???", sess->user, sess->pass ? ":" : "", sess->pass ? sess->pass : "", sess->host,
                   sess->port, sess->rsrc));
    }
    if (sess->hop)
        ns_desc_hop(sess->hop, NULL);
    if (sess->sysrc)
        D_ESCREEN(("%s: searching for sysrc in %s\n", doc, sess->sysrc));
    if (sess->home)
        D_ESCREEN(("%s: searching for usrrc in %s\n", doc, sess->home));
    D_ESCREEN(("%s: escapes set to ^%c-%c\n", doc, sess->escape + 'A' - 1, sess->literal));
#ifdef NS_HAVE_TWIN
    D_ESCREEN(("%s: twin %s at %p\n", doc, sess->twin_str ? sess->twin_str : "NULL", sess->twin));
#endif
}



/* run a command. uses the terminal's internal run facility.
   converts system/"char *" to exec/"arg **".
   efuns: struct of callbacks into the terminal program.
          ns_run() will fail if no callback to the terminal's "run program"
          (exec) facility is provided.
   cmd:   a string to exec
   <-     whatever the callback returns.  In Eterm, it's a file-descriptor.  */

int
ns_run(_ns_efuns * efuns, char *cmd)
{
    char **args = NULL;
    char *p = cmd;
    int c, n = 0, s = 0;

    if (!efuns || !efuns->execute)
        goto fail;

    if (cmd && *cmd) {          /* count args (if any) */
        D_ESCREEN(("ns_run: executing \"%s\"...\n", cmd));
        do {
            n++;
            while (*p && *p != ' ') {
                if (*p == '\"') {
                    do {
                        p++;
                        if (s)
                            s = 0;
                        else if (*p == '\\')
                            s = 1;
                        else if (*p == '\"')
                            s = 2;
                    }
                    while (*p && s != 2);
                }
                p++;
            }
            while (*p == ' ')
                p++;
        }
        while (*p);

        if (!(args = MALLOC((n + 2) * sizeof(char *))))
            goto fail;

        for (p = cmd, c = 0; c < n; c++) {
            args[c] = p;
            while (*p && *p != ' ') {
                if (*p == '\"') {       /* leave quoting stuff together as one arg */
                    args[c] = &p[1];    /* but remove the quote signs */
                    do {
                        p++;
                        if (s)
                            s = 0;
                        else if (*p == '\\')
                            s = 1;
                        else if (*p == '\"')
                            s = 2;
                    }
                    while (*p && s != 2);
                    *p = '\0';
                }
                p++;
            }
            while (*p == ' ')
                *(p++) = '\0';
        }
        args[c++] = NULL;
    }

    n = efuns->execute(NULL, args);
    if (args)
        FREE(args);
    return n;

  fail:
    return NS_FAIL;
}



/* create a call line. used in ns_attach_ssh/lcl
   tmpl   the template. should contain one %s
   dflt   the default value
   opt    the user-supplied value (or NULL)
   <-     a new MALLOC'd string (or NULL) */

static char *
ns_make_call_el(char *tmpl, char *dflt, char *opt)
{
    int l, r;
    char *p;

    if (tmpl && dflt && *tmpl && strstr(tmpl, "%s")) {
        l = strlen(tmpl) + (opt ? strlen(opt) : strlen(dflt)) - 1L;
        if ((p = MALLOC(l))) {
            r = snprintf(p, l, tmpl, opt ? opt : dflt);
            if ((r >= 0) && (r < l)) {
                return p;
            }
            FREE(p);
        }
    }
    return NULL;
}



static char *
ns_make_call(_ns_sess * sess)
{
    char *call, *tmp = NULL, *screen = NULL, *scream = NULL, *screem = NULL;

#ifdef NS_HAVE_TWIN
    if (sess->backend == NS_MODE_TWIN)
        return ns_make_call_el(NS_TWIN_CALL, NS_TWIN_OPTS, sess->rsrc);
#endif
    /* unless decidedly in other mode... */
    if (sess->backend != NS_MODE_SCREEN)
        tmp = scream = ns_make_call_el(NS_SCREAM_CALL, NS_SCREAM_OPTS, sess->rsrc);
#ifdef NS_HAVE_SCREEN
    if (sess->backend != NS_MODE_SCREAM)
        tmp = screen = ns_make_call_el(NS_SCREEN_CALL, NS_SCREEN_OPTS, sess->rsrc);
#endif
    if (sess->backend == NS_MODE_NEGOTIATE) {
        int r, l = strlen(NS_SCREEM_CALL) + (scream ? strlen(scream) : 0) + (screen ? strlen(screen) : 0) - 3;

        if ((screem = MALLOC(l))) {
            r = snprintf(screem, l, NS_SCREEM_CALL, scream ? scream : "", screen ? screen : "");
#ifdef NS_PARANOID
            if ((r < 0) || (r > l)) {
                ns_free(&screem);
            }
#endif
        }
        tmp = screem;
    }
    call = ns_make_call_el(NS_WRAP_CALL, tmp, NULL);
    ns_free(&screen);
    ns_free(&scream);
    ns_free(&screem);
    return call;
}



/* attach a local session (using screen/scream)
   sp  the session
   <-  NS_FAIL, or the result of ns_run() */

static int
ns_attach_lcl(_ns_sess ** sp)
{
    _ns_sess *sess;
    char *call;
    int ret = -1;

    if (!sp || !*sp)
        return ret;

    sess = *sp;

    if ((call = ns_make_call(sess))) {
        char *c2 = ns_make_call_el("/bin/sh -c \"%s\"", call, NULL);

        ns_free(&call);
        if (c2) {
            ret = ns_run(sess->efuns, c2);
            ns_free(&c2);
        }
    }
    return ret;
}



/* attach a remote session (using screen/scream via ssh)
   sp  the session
   <-  -1, or the result of ns_run() */

static int
ns_attach_ssh(_ns_sess ** sp)
{
    _ns_sess *sess;
    char cmd[NS_MAXCMD + 1];
    char esc[] = " -e^___";
    char *call, *p;
    int ret;

    if (!sp || !*sp)
        return NS_FAIL;

    sess = *sp;

    p = &esc[3];                /* escapes for screen and compatibles */
    if (sess->escape < ' ') {
        *p++ = '^';
        *p++ = sess->escape + 'A' - 1;
    } else
        *p++ = sess->escape;    /* this should never happen */
    if (sess->literal < ' ') {  /* this should never happen */
        *p++ = '^';
        *p++ = sess->literal + 'A' - 1;
    } else
        *p++ = sess->literal;

    call = ns_make_call(sess);

    if (sess->hop) {
        if (sess->hop->established == NS_HOP_DOWN) {    /* the nightmare foe */
            ret = snprintf(cmd, NS_MAXCMD, "%s %s -p %d -L %d:%s:%d %s@%s",
                           NS_SSH_CALL, NS_SSH_TUNNEL_OPTS, sess->hop->fwport, sess->hop->localport, sess->host, sess->port, sess->user, sess->hop->fw);
            if (ret < 0 || ret > NS_MAXCMD)
                return NS_FAIL;
            ns_run(sess->efuns, cmd);
            sleep(sess->hop->delay);
        }
        ret = snprintf(cmd, NS_MAXCMD, "%s %s -p %d %s@localhost \"%s%s\"",
                       NS_SSH_CALL, NS_SSH_OPTS, sess->hop->localport, sess->user, call, ((sess->backend == NS_MODE_SCREEN) || (sess->backend == NS_MODE_NEGOTIATE)) ? esc : "");
    } else {
        ret = snprintf(cmd, NS_MAXCMD, "%s %s -p %d %s@%s \"%s%s\"", NS_SSH_CALL, NS_SSH_OPTS, sess->port, sess->user, sess->host, call,
                       ((sess->backend == NS_MODE_SCREEN) || (sess->backend == NS_MODE_NEGOTIATE)) ? esc : "");
    }
    ns_free(&call);

    return (ret < 0 || ret > NS_MAXCMD) ? NS_FAIL : ns_run(sess->efuns, cmd);
}



/* ns_attach_by_sess
   attach/create a scream/screen session, locally or over the net.
   sess:   a session struct as generated by (eg) ns_attach_by_URL()
 ! err:    if non-NULL, variable pointed at will contain an error status
   <-      the requested session, or NULL in case of failure.
           a session thus returned must be detached/closed later.
*/

_ns_sess *
ns_attach_by_sess(_ns_sess ** sp, int *err)
{
    _ns_sess *sess;
    int err_dummy;

    if (!err)
        err = &err_dummy;
    *err = NS_INVALID_SESS;

    if (!sp || !*sp)
        return NULL;
    sess = *sp;

    ns_desc_sess(sess, "ns_attach_by_sess()");

    (void) ns_sess_init(sess);

    switch (sess->where) {
      case NS_LCL:
          sess->fd = ns_attach_lcl(&sess);
          break;
      case NS_SU:              /* (fixme) uses ssh, should use su */
          /* local session, but for a different uid. */
          /* FALL-THROUGH */
      case NS_SSH:
          if (!sess->delay) {
              sess->delay = NS_INIT_DELAY ? NS_INIT_DELAY : 1;
          }
          sess->fd = ns_attach_ssh(&sess);
          break;
      default:
          *err = NS_UNKNOWN_LOC;
          goto fail;
    }

    D_ESCREEN(("ns_attach_by_sess: screen session-fd is %d, ^%c-%c\n", sess->fd, sess->escape + 'A' - 1, sess->literal));

#ifdef NS_HAVE_TWIN_
    if (sess->backend == NS_MODE_TWIN) {
        sleep(2);
        if (!Tw_CheckMagic(libscream_magic)) {
            D_ESCREEN(("ns_attach_by_sess: Tw_CheckMagic failed\n"));
            goto fail;
        } else {
            if (!(sess->twin = Tw_Open(sess->twin_str))) {
                D_ESCREEN(("ns_attach_by_sess: Tw_Open(%s) failed\n", sess->twin_str));
                goto fail;
            }
            D_ESCREEN(("ns_attach_by_sess: Tw_Open() succeeded\n"));
            ns_desc_sess(sess, "ns_sess_init");
        }
    }
#endif

    return sess;

  fail:
    return ns_dst_sess(sp);
}



/* ns_attach_by_URL
   parse URL into sess struct (with sensible defaults), then pick up/create
   said session using ns_attach_by_sess()
   url:    URL to create/pick up a session at.
           proto://user:password@host.domain:port  (all parts optional)
           NULL/empty string equivalent to
           screen://current_user@localhost/-xRR
   hop:    one of NULL lclport:fw:fwport fw:fwport lclport:fw
           if set, describes how to tunnel through a fw to access an URL
           describing a target behind said firewall
   ef:     a struct containing callbacks into client (resize scrollbars etc.)
           while setting those callbacks is optional; omitting the struct
           itself seems unwise.
 ! err:    if non-NULL, variable pointed at will contain an error status
   xd:     pointer to extra-data the terminal app wants to associate with
           a session, or NULL
   <-      the requested session, or NULL in case of failure.
           a session thus returned must be detached/closed later.
*/

_ns_sess *
ns_attach_by_URL(char *url, char *hop, _ns_efuns ** ef, int *err, void *xd)
{
    int err_dummy;
    char *p, *d = NULL;
    _ns_sess *sess = ns_new_sess();
    struct passwd *pwe = getpwuid(getuid());

    if (!err)
        err = &err_dummy;
    *err = NS_OOM;

    if (!sess)
        return NULL;

    if (url && strlen(url)) {
        char *q;

        if (!(d = STRDUP(url)))
            goto fail;

        if ((q = strstr(d, "://"))) {   /* protocol, if any */
            *q = '\0';
            if (!(sess->proto = STRDUP(d)))
                goto fail;
            q += 3;
        } else
            q = d;

        if ((p = strchr(q, '@'))) {     /* user, if any */
            char *r;

            if (p != q) {       /* ignore empty user */
                *p = '\0';
                if ((r = strchr(q, ':'))) {     /* password, if any */
                    *(r++) = '\0';
                    if (!(sess->pass = STRDUP(r)))      /* password may be empty string! */
                        goto fail;
                }
                sess->user = STRDUP(q);
            }
            q = p + 1;
        }

        if ((p = strchr(q, '/'))) {
            *(p++) = '\0';
            if (strlen(p)) {
                char *r = p;
                int f;

                while (*r) {
                    if (*r == '+')
                        *(r++) = ' ';
                    else if ((*r == '%') && (strlen(r) > 2)) {
                        long v;
                        char *e;
                        char b[3];

                        b[0] = r[1];
                        b[1] = r[2];
                        b[2] = '\0';
                        v = strtol(b, &e, 16);
                        if (!*e) {
                            *(r++) = (char) (v & 0xff);
                            memmove(r, &r[2], strlen(&r[2]));
                        }
                    } else
                        r++;
                }
                r = p;
                f = 0;
                while (*r) {
                    if (*r == ' ') {    /* Padding between arguments */
                        while (*r == ' ')
                            r++;
                    } else {
                        if (*r == '-') {
#  ifdef NS_HAVE_SCREEN
                            if (*(++r) == 'e') {        /* set escape */
                                char x = 0, y = 0;

                                while (*(++r) == ' ');
                                if ((x = ns_parse_esc(&r)) && (y = ns_parse_esc(&r))) {
                                    sess->escape = x;
                                    sess->literal = y;
                                    sess->escdef = NS_ESC_CMDLINE;
                                }
                            } else if (*r == 'c') {     /* alt screenrc */
                                char *rc, *rx;

                                while (*(++r) == ' ');
                                if ((rx = strchr(r, ' ')))
                                    *rx = '\0';
                                if (*r != '/')
                                    D_ESCREEN(("URL: path for screen's option -c should be absolute (%s)\n", r));
                                if ((rc = STRDUP(r))) {
                                    if (sess->home)     /* this should never happen */
                                        FREE(sess->home);
                                    D_ESCREEN(("URL: searching for rc in %s\n", rc));
                                    sess->home = rc;
                                }
                                if (rx) {
                                    r = rx;
                                    *rx = ' ';
                                }
                            }
#  endif
                            while (*r && (f || *r != ' ')) {
                                if (*r == '\"')
                                    f = 1 - f;
                                r++;
                            }
                        }
                        while (*r && *r != ' ') /* proceed to space */
                            r++;
                    }
                }

                if (!(sess->rsrc = STRDUP(p)))
                    goto fail;
            }
        }

        if ((p = strchr(q, ':'))) {     /* port, if any */
            *(p++) = '\0';
            if (!*p || !(sess->port = atoi(p)) || sess->port > NS_MAX_PORT) {
                *err = NS_MALFORMED_URL;
                goto fail;
            }
        }

        if (strlen(q) && !(sess->host = STRDUP(q)))     /* host, if any */
            goto fail;

        FREE(d);
    }

    sess->where = NS_SSH;

    if (!sess->user) {          /* default user (current user) */
        if (!pwe) {
            *err = NS_UNKNOWN_USER;
            goto fail;
        }
        if (!(sess->user = STRDUP(pwe->pw_name)))
            goto fail;
    } else if ((sess->host && strcmp(sess->host, "localhost") && strcmp(sess->host, "127.0.0.1")) || sess->port) {
        pwe = NULL;
    } else if (!pwe || strcmp(pwe->pw_name, sess->user)) {      /* user!=current_user */
        sess->where = NS_SU;
        if (!(pwe = getpwnam(sess->user))) {
            *err = NS_UNKNOWN_USER;
            goto fail;
        }
    } else {
        *err = NS_UNKNOWN_USER;
        goto fail;
    }


#ifdef NS_HAVE_SCREEN
    if (getenv("SYSSCREENRC")) {        /* $SYSSCREENRC */
        if (!(sess->sysrc = STRDUP(getenv("SCREENRC"))))
            goto fail;
    } else {
        char *loc[] = { "/usr/local/etc/screenrc",      /* official */
            "/etc/screenrc",    /* actual (on SuSE) */
            "/usr/etc/screenrc",
            "/opt/etc/screenrc"
        };
        int n, nloc = sizeof(loc) / sizeof(char *);

        for (n = 0; n < nloc; n++)
            if (!access(loc[n], R_OK)) {
                if (!(sess->sysrc = STRDUP(loc[n])))
                    goto fail;
                n = nloc;
            }
    }

    if (getenv("SCREENRC")) {   /* $SCREENRC */
        sess->home = STRDUP(getenv("SCREENRC"));
    } else if (pwe && !sess->home) {    /* ~/.screenrc */
        if ((sess->home = MALLOC(strlen(pwe->pw_dir) + strlen(NS_SCREEN_RC) + 2)))
            sprintf(sess->home, "%s/%s", pwe->pw_dir, NS_SCREEN_RC);
    }
#endif

    if (!sess->host) {          /* no host */
        if (!(sess->host = STRDUP("localhost")))
            goto fail;
        if (!sess->port) {      /* no host/port */
            sess->where = NS_LCL;
        }
    } else if ((p = strchr(sess->host, '/')))   /* have host */
        *p = '\0';

    if (!sess->port)            /* no port -> default port (SSH) */
        sess->port = ns_get_ssh_port();

    sess->backend = NS_MODE_NEGOTIATE;
    if (sess->proto) {
#ifdef NS_HAVE_SCREEN
        if (!strcmp(sess->proto, "screen")) {
            sess->backend = NS_MODE_SCREEN;
        } else
#endif
#ifdef NS_HAVE_TWIN
        if (!strcmp(sess->proto, "twin")) {
            char *twd = getenv("TWDISPLAY");

            sess->backend = NS_MODE_TWIN;
            if (!sess->twin_str)
                sess->twin_str = STRDUP(twd ? twd : ":0");
        } else
#endif
        if (!strcmp(sess->proto, "scream")) {
            sess->backend = NS_MODE_SCREAM;
        } else {
            *err = NS_UNKNOWN_PROTO;
            goto fail;
        }
    }

    if (!sess->efuns && ef && *ef) {
        sess->efuns = ns_ref_efuns(ef);
    }

    sess->userdef = xd;

    if (hop && strlen(hop)) {
        sess->hop = ns_parse_hop(sess, hop);
        if (sess->hop && (!strcmp(sess->host, sess->hop->fw) || !strcmp(sess->host, "localhost") || !strcmp(sess->host, "127.0.0.1")))
            D_ESCREEN(("ns_attach_by_URL: routing in circles...\n"));
    }

    *err = NS_SUCC;
    return ns_attach_by_sess(&sess, err);

  fail:
    if (d)
        FREE(d);

    return ns_dst_sess(&sess);
}



/* detach a session and release its memory
   sess  the session
   <-    error code */
int
ns_detach(_ns_sess ** sess)
{
    ns_desc_sess(*sess, "ns_detach");
#ifdef NS_HAVE_TWIN
    if (((*sess)->backend == NS_MODE_TWIN) && (*sess)->twin) {
        Tw_Flush((*sess)->twin);
        Tw_Close((*sess)->twin);
    }
#endif
    (void) ns_dst_sess(sess);
    return NS_SUCC;
}



/****************************************************************************
 ____             _                        _ 
| __ )  __ _  ___| | __      ___ _ __   __| |
|  _ \ / _` |/ __| |/ /____ / _ \ '_ \ / _` |
| |_) | (_| | (__|   <_____|  __/ | | | (_| |
|____/ \__,_|\___|_|\_\     \___|_| |_|\__,_|
                                             

backend abstraction (utils)

   this abstracts the backend against the frontend; the terminal-emulator
   calls these functions without knowing what the backend is. */



/***************************************************************************/
/* display foo */
/***************/



/* toggle last two displays */
int
ns_tog_disp(_ns_sess * s)
{
    if (!s)
        return NS_FAIL;

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          return ns_screen_command(s, "\x01\x01");
          break;
#endif
      default:
          return NS_FAIL;
    }
}

/* go to display #d */
int
ns_go2_disp(_ns_sess * s, int d)
{
    char b[] = "\x01_";

    if (!s)
        return NS_FAIL;
    if (s->curr && s->curr->index == d)
        return NS_SUCC;

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          b[1] = '0' + d;
          return ns_screen_command(s, b);
          break;
#endif
#ifdef NS_HAVE_TWIN_
      case NS_MODE_TWIN:
          {
              tscreen ts = Tw_FirstScreen(s->twin);

              printf("screen: %p\n", ts);
              while (d-- && ts)
                  ts = Tw_NextObj(s->twin, ts);
              if (ts) {
                  Tw_RaiseScreen(s->twin, ts);
                  return NS_SUCC;
              }
          }
          break;
#endif
      default:
          return NS_FAIL;
    }
}

/* toggle monitor mode for disp (if possible) */
int
ns_mon_disp(_ns_sess * s, int no)
{
    if (!s)
        return NS_FAIL;

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          ns_go2_disp(s, no);
          return ns_screen_command(s, "\x01M");
          break;
#endif
      default:
          return NS_FAIL;
    }
}

/* scrollback buffer mode (if any) */
int
ns_sbb_disp(_ns_sess * s, int no)
{
    if (!s)
        return NS_FAIL;

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          ns_go2_disp(s, no);
          return ns_screen_command(s, "\x01\x1b");
          break;
#endif
      default:
          return NS_FAIL;
    }
}

/* go to display #+-d */
int
ns_rel_disp(_ns_sess * s, int d)
{
    int n;
    _ns_disp *x;

    if (!s)
        return NS_FAIL;
    if (!d)
        return NS_SUCC;

    if (!s->curr) {
        if (!(s->curr = s->dsps)) {
            return NS_FAIL;
        }
    }

    x = s->curr;

    if (d < 0) {
        _ns_disp *l;

        for (l = s->dsps; l->next; l = l->next);

        while (d++) {
            if (!(x = x->prvs))
                x = l;
        }
    } else {
        while (d--) {
            if (!(x = x->next))
                x = s->dsps;
        }
    }
    ns_go2_disp(s, x->index);
}

/* add a client display with the name "name" after display number #after */
int
ns_add_disp(_ns_sess * s, int after, char *name)
{
    if (!s) {
        return NS_FAIL;
    }

    D_ESCREEN(("ns_add_disp: add %s after #%d\n", name, after));

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          if (after >= 0)
              ns_go2_disp(s, after);
          if (ns_screen_command(s, "\x01\x03") == NS_SUCC) {
              /* yes, -1 for "current_display" works even though we just made a new
                 display that isn't in our list yet.  faith will see you through. */
              ns_ren_disp(s, -1, name);
          }
          break;
#endif
#ifdef NS_HAVE_TWIN
      case NS_MODE_TWIN:
          break;
#endif
      default:
          return NS_FAIL;
    }
}


/* move client display #fm to display slot #to */
int
ns_mov_disp(_ns_sess * s, int fm, int to)
{
    _ns_disp *d;

    if (!s) {
        return NS_FAIL;
    }

    if (fm == to)
        return NS_SUCC;

    if ((fm < 0) || (to < 0))
        return NS_FAIL;

    if (!(d = s->dsps))         /* this should never happen */
        return NS_FAIL;

    fm = disp_get_screen_by_real(s, fm);
    to = disp_get_screen_by_real(s, to);

    if (fm == to)               /* ??? */
        return NS_SUCC;

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          D_ESCREEN(("ns_mov_disp: move #%d to #%d\n", fm, to));
          ns_mov_screen_disp(s, fm, to);
          break;
#endif
    }
    return NS_FAIL;
}

/* resize display #d to w*h */
int
ns_rsz_disp(_ns_sess * s, int d, int w, int h)
{
    if (!s) {
        return NS_FAIL;
    }

    switch (s->backend) {
      default:
          return NS_FAIL;
    }
}

/* remove display #d */
int
ns_rem_disp(_ns_sess * s, int d, int ask)
{
    char *i = NULL, *n;
    size_t l;
    int ret = NS_FAIL;

    if (!s) {
        return NS_FAIL;
    }

    if (!s->curr) {
        if (!(s->curr = s->dsps))
            return NS_FAIL;
    }

    if (d < 0) {
        d = s->curr->index;
    }

    if (ask) {
        (void) ns_inp_dial(s, "Really delete this display?", 1, &i, NULL);
        if (!i || !*i)
            return NS_FAIL;
    }

    if (*i == 'y' || *i == 'Y') {
        switch (s->backend) {
#ifdef NS_HAVE_SCREEN
          case NS_MODE_SCREEN:
              ns_go2_disp(s, d);
              ret = ns_screen_command(s, "\x01ky\r");
              break;
#endif
        }
    }

    if (i)
        FREE(i);

    return ret;
}

/* rename display #d to "name".  if d==-1, use current */
int
ns_ren_disp(_ns_sess * s, int d, char *name)
{
    char *i = NULL, *n;
    size_t l;
    int ret = NS_FAIL;

    if (!s) {
        return NS_FAIL;
    }

    if (!s->curr) {
        if (!(s->curr = s->dsps))
            return NS_FAIL;
    }

    if (d < 0) {
        d = s->curr->index;
    }

    if (!name || !*name) {      /* ask */
        i = s->curr->name;
        l = strlen(i);
        (void) ns_inp_dial(s, "Enter a new name for the current display", 12, &i, NULL);
        if (!i || !*i)
            return NS_FAIL;
    }

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          if ((n = MALLOC(strlen(i ? i : name) + l + 1))) {
              ns_go2_disp(s, d);
              strcpy(&n[l], i ? i : name);      /* copy new name */
              while (l)         /* prepend backspaces */
                  n[--l] = '\x08';
              ret = ns_screen_xcommand(s, 'A', n);      /* rename */
              FREE(n);
          }
          break;
#endif
    }

    if (i)
        FREE(i);

    return ret;
}

/* log activity in display #d to file "logfile" */
int
ns_log_disp(_ns_sess * s, int d, char *logfile)
{
    if (!s) {
        return NS_FAIL;
    }

    switch (s->backend) {
      default:
          return NS_FAIL;
    }
}



/***************************************************************************/
/* region/window foo */
/*********************/


int
ns_tog_region(_ns_sess * s, _ns_disp * d)
{
    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;
}

int
ns_go2_region(_ns_sess * s, _ns_disp * d, int n)
{
    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;
}



int
ns_rel_region(_ns_sess * s, _ns_disp * d, int n)
{
    int ret = NS_FAIL;

    if (!n)
        return NS_SUCC;

    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          if (n < 0)
              return NS_FAIL;
          do {
              ret = ns_screen_command(s, "\x01\x09");
          } while (--n && (ret == NS_SUCC));
          break;
#endif
    }
    return ret;
}



int
ns_add_region(_ns_sess * s, _ns_disp * d, int after, char *name)
{
    int ret = NS_FAIL;

    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          ret = ns_screen_command(s, "\x01S");
          break;
#endif
    }
    return ret;
}



int
ns_rsz_region(_ns_sess * s, _ns_disp * d, int r, int w, int h)
{
    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;
}

int
ns_rem_region(_ns_sess * s, _ns_disp * d, int r, int ask)
{
    int ret = NS_FAIL;

    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          ret = ns_screen_command(s, "\x01X");
          break;
#endif
    }
    return ret;
}



int
ns_one_region(_ns_sess * s, _ns_disp * d, int r)
{
    int ret = NS_FAIL;

    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          ret = ns_screen_command(s, "\x01Q");
          break;
#endif
    }
    return ret;
}



int
ns_mov_region(_ns_sess * s, _ns_disp * d, int fm, int to)
{
    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;
}

int
ns_ren_region(_ns_sess * s, _ns_disp * d, int r, char *name)
{
    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;
}

int
ns_log_region(_ns_sess * s, _ns_disp * d, int r, char *logfile)
{
    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;
}

int
ns_mon_region(_ns_sess * s, _ns_disp * d, int r)
{
    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;
}

int
ns_sbb_region(_ns_sess * s, _ns_disp * d, int r)
{
    if (ns_magic_disp(&s, &d) == NS_FAIL)
        return NS_FAIL;
}



/***************************************************************************/
/* session foo */
/***************/



/* scroll horizontally to column x (dummy) */
int
ns_scroll2x(_ns_sess * s, int x)
{
    if (!s) {
        return NS_FAIL;
    }

    return NS_FAIL;
}

/* scroll vertically so line y of the scrollback buffer is the top line */
int
ns_scroll2y(_ns_sess * s, int y)
{
    if (!s) {
        return NS_FAIL;
    }

    return NS_FAIL;
}

/* force an update of the status line */
int
ns_upd_stat(_ns_sess * s)
{
    if (!s) {
        return NS_FAIL;
    }

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          return ns_screen_command(s, NS_SCREEN_UPDATE);
#endif
      default:
          return NS_FAIL;
    }
}

int
ns_statement(_ns_sess * s, char *c)
{
    int ret = NS_FAIL;
    char *i = NULL;
    char x, y;

    if (!s) {
        return NS_FAIL;
    }

    y = x = s->escape;

    if (!c || !*c) {
        (void) ns_inp_dial(s, "Enter a command to send to the \"screen\" program", 64, &i, ns_inp_tab);
        if (!i || !*i)
            return NS_FAIL;
    }

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          if ((ret = ns_parse_screen_cmd(s, i ? i : c, NS_ESC_INTERACTIVE)) == NS_SUCC) {
              if (s->escape != x) {
                  y = s->escape;
                  s->escape = x;
              }
              ret = ns_screen_xcommand(s, NS_SCREEN_CMD, i ? i : c);
              s->escape = y;
          } else if (ret == NS_NOT_ALLOWED) {
              ns_inp_dial(s, "Sorry, David, I cannot allow that.", 0, NULL, NULL);
          }
#endif
      default:
          ret = NS_FAIL;
    }

    if (i)
        FREE(i);

    return ret;
}

int
ns_reset(_ns_sess * s, int type)
{
    if (!s) {
        return NS_FAIL;
    }

    switch (s->backend) {
#ifdef NS_HAVE_SCREEN
      case NS_MODE_SCREEN:
          return ns_screen_command(s, NS_SCREEN_INIT);
#endif
      default:
          return NS_FAIL;
    }
}

char *
ns_get_url(_ns_sess * s, int d)
{
    int r, l;
    char *u;
    char esc[] = "^_\0";
    char lit[] = "^_\0";

    if (!s) {
        return NULL;
    }

    l = ((s->proto) ? strlen(s->proto) + 3 : 0) + strlen(s->user) + 1 + strlen(s->host) + 1 + 5 + 1 + ((s->rsrc) ? strlen(s->rsrc) : 0) + 7 + (s->name ? strlen(s->name) + 4 : 0) +
        1;

    if ((u = malloc(l + 1))) {
        if (!s->escape) {
            esc[0] = '\0';
        } else if (s->escape < ' ') {
            esc[1] = s->escape + 'A' - 1;
        } else {
            esc[0] = s->escape;
            esc[1] = '\0';
        }
        if (!s->literal) {
            lit[0] = '\0';
        } else if (s->literal < ' ') {
            lit[1] = s->literal + 'A' - 1;
        } else {
            lit[0] = s->literal;
            lit[1] = '\0';
        }
        r = snprintf(u, l, "%s%s%s@%s:%d/%s%s%s%s%s%s", s->proto ? s->proto : "", s->proto ? "://" : "", s->user, s->host, s->port, ((s->rsrc) ? s->rsrc : ""),
                     ((s->escape) ? "+-e" : ""), esc, ((s->escape) ? lit : ""), ((s->name) ? "+-x+" : ""), ((s->name) ? s->name : ""));
        D_ESCREEN(("ns_get_url: URL is %s\n", u));
        if ((r >= 0) && (r < l)) {
            return u;
        }
        FREE(u);
    }

    return NULL;
}



/****************************************************************************
                                                        _  __ _      
 ___  ___ _ __ ___  ___ _ __        ___ _ __   ___  ___(_)/ _(_) ___ 
/ __|/ __| '__/ _ \/ _ \ '_ \ _____/ __| '_ \ / _ \/ __| | |_| |/ __|
\__ \ (__| | |  __/  __/ | | |_____\__ \ |_) |  __/ (__| |  _| | (__ 
|___/\___|_|  \___|\___|_| |_|     |___/ .__/ \___|\___|_|_| |_|\___|
                                       |_|                           
screen-specific routines

   these routines handle a specific backend, the GNU "screen" program. */



#ifdef NS_HAVE_SCREEN

/* ns_swp_screen_disp - swap screen displays
   s   session
   fm  from (old index) 
   to  to   (new index)
   <-  error code */

static int
ns_swp_screen_disp(_ns_sess * s, int fm, int to)
{
    char *t1 = "\x01'%d\r";
    char *t2 = "\x01:number %d\r";
    char b[NS_MAXCMD + 1];
    int l;
    _ns_disp *d, *d2, *n;

#  ifdef NS_PARANOID
    if ((fm > 9999) || (to > 9999))
        return NS_FAIL;
#  endif

    if (!s->curr || s->curr->index != fm) {     /* switch to source disp if necessary */
        if (!(s->curr = disp_fetch(s, fm))) {
            return NS_FAIL;
        }

        l = snprintf(b, NS_MAXCMD, t1, fm);
#  ifdef NS_PARANOID
        if ((l <= 0) || (l > NS_MAXCMD)) {
            return NS_FAIL;
        }
#  endif

        (void) ns_screen_command(s, b);
    }

    l = snprintf(b, NS_MAXCMD, t2, to);
#  ifdef NS_PARANOID
    if ((l <= 0) || (l > NS_MAXCMD)) {
        return NS_FAIL;
    }
#  endif

    (void) ns_screen_command(s, b);

    d2 = disp_fetch(s, to);

    s->curr->index = to;

    if (d2)                     /* target did exist => screen swapped them, so adjust tgt index */
        d2->index = fm;

    d = s->dsps;
    while (d) {
        if ((n = d->next) && (d->index > n->index)) {   /* must... sort... */
            for (d2 = n; d2->next && (d2->index <= d->index); d2 = d2->next);

            if (d->prvs)        /* remove offender from list */
                d->prvs->next = d->next;
            else
                s->dsps = d->next;
            if (d->next)
                d->next->prvs = d->prvs;

            d->prvs = d2;
            if ((d->next = d2->next))
                d2->next->prvs = d;
            d2->next = d;

            d = s->dsps;
        } else
            d = d->next;
    }

    return NS_SUCC;
}


/* ns_mov_screen_disp - move a screen display to a new position
   this does some magic to implement an "insert" using screen's "swap"
   s   session
   fm  from (old index) 
   to  to   (new index)
   <-  error code */

static int
ns_mov_screen_disp(_ns_sess * s, int fm, int to)
{
    _ns_efuns *efuns;
    _ns_disp *d, *d2 = NULL;
    int n = 1;

#  ifdef NS_PARANOID
    if (!(d = s->dsps))         /* this should never happen */
        return NS_FAIL;
#  endif

    while (d && d->next) {
        n++;
        if (d->index == to)
            d2 = d;
        d = d->next;
    }

    if (d2) {                   /* target exist, do the whole enchilada */
        if ((d2->prvs) && (d2->prvs->index == fm)) {    /* special case: swap */
            ns_swp_screen_disp(s, fm, to);
        } else {
            while (d && (d->index >= to)) {
                ns_swp_screen_disp(s, d->index, d->index + 1);
                d = d->prvs;
            }

            ns_swp_screen_disp(s, fm + ((fm > to) ? 1 : 0), to);

            /* done. now unsparse. */
            if (to > fm) {      /* moved right */
                d = s->dsps;
                while (d->index <= fm)
                    d = d->next;
                while (d) {
                    ns_swp_screen_disp(s, d->index, d->index - 1);
                    d = d->next;
                }
            }
        }
    } else if (d->index == to) {        /* kinda ugly : ( */
        if ((to - fm) == 1) {   /* swap last two */
            ns_swp_screen_disp(s, fm, to);
        } else {                /* move before last */
            while (d && (d->index >= to)) {     /* renumber */
                ns_swp_screen_disp(s, d->index, d->index + 1);
                d = d->prvs;
            }

            ns_swp_screen_disp(s, fm, to);

            /* done. now unsparse. */
            d = s->dsps;
            while (d->index <= fm)
                d = d->next;
            while (d) {
                ns_swp_screen_disp(s, d->index, d->index - 1);
                d = d->next;
            }
        }
    } else {                    /* no target, simple renumber */
        ns_swp_screen_disp(s, fm, to);
    }

    s->curr = NULL;
    ns_dst_dsps(&(s->dsps));

    if (NS_EFUN_EXISTS(efuns, s, NULL, expire_buttons))
        efuns->expire_buttons(s->userdef, n);

    ns_upd_stat(s);
}



/* send a command string to a session, using the appropriate escape-char
   sess  the session
   cmd   the command string.  escapes must be coded as NS_SCREEN_ESCAPE;
         this routine will convert the string to use the escapes actually
         used in the session
   <-    error code */

int
ns_screen_command(_ns_sess * sess, char *cmd)
{
    _ns_efuns *efuns;
    char *c;
    int ret = NS_SUCC;

    D_ESCREEN(("sess %8p, cmd %8p\n", sess, cmd));
    if (!cmd || !*cmd)
        return NS_FAIL;

    D_ESCREEN(("Sending screen command %s\n", safe_print_string(cmd, strlen(cmd))));
    if (NS_EFUN_EXISTS(efuns, sess, NULL, inp_text)) {
        D_ESCREEN((" -> inp_text is set\n"));
        if ((c = STRDUP(cmd))) {
            char *p = c;    /* replace default escape-char with that */

            D_ESCREEN((" -> translating escapes\n"));
            while (*p) {    /* actually used in this session */
                if (*p == NS_SCREEN_ESCAPE)
                    *p = sess->escape;
                p++;
            }
            ns_desc_string(c, "ns_screen_command: xlated string");
            efuns->inp_text(NULL, sess->fd, c);
            FREE(c);
        } else {
            D_ESCREEN((" -> out of memory\n"));
            ret = NS_OOM;
        }
    } /* out of memory */
    else {
        ret = NS_EFUN_NOT_SET;
        D_ESCREEN(("ns_screen_command: sess->efuns->inp_text not set!\n"));
    }
    return ret;
}



/* send a single command string to screen, adding the equiv of ^A:
   s     the session
   cmd   the command string
   <-    error code */

int
ns_screen_xcommand(_ns_sess * s, char prefix, char *cmd)
{
    char *i;
    int ret = NS_OOM;

    if ((i = MALLOC(strlen(cmd) + 4))) {
        size_t l = strlen(cmd) + 2;

        strcpy(&i[2], cmd);
        i[0] = s->escape;
        i[1] = prefix;
        i[l] = '\n';
        i[++l] = '\0';
        ret = ns_screen_command(s, i);
        FREE(i);
    }
    return ret;
}



/* tab completion for screen-commands
  !b  current entry (changes)
   l  number of characters to compare in current entry
   m  maximum number of characters in entry (size of input buffer)
   <- error code */

static int
ns_inp_tab(void *xd, char *b, size_t l, size_t m)
{
    char *sc[] = { "acladd", "addacl", "aclchg", "chacl", "acldel", "aclgrp",
        "aclumask", "umask", "activity",
        "allpartial", "at", "attrcolor", "autonuke", "bce",
        "bell_msg", "bind", "bindkey", "break", "breaktype",
        "bufferfile", "c1", "caption", "charset", "chdir",
        "clear", "compacthist", "console", "copy",
        "crlf", "debug", "defc1", "defautonuke", "defbce",
        "defbreaktype", "defcharset", "defflow", "defgr",
        "defencoding", "deflog", "deflogin", "defmode",
        "defmonitor", "defobuflimit", "defscrollback",
        "defshell", "defsilence", "defslowpast", "defutf8",
        "defwrap", "defwritelock", "defzombie", "detach",
        "dinfo", "displays", "digraph", "dumptermcap",
        "escape", "eval", "exec", "fit", "flow", "focus", "gr",
        "hardcopy", "hardcopy_append", "hardcopydir",
        "height", "help", "history", "ignorecase", "encoding",
        "kill", "license", "lockscreen", "log", "logfile",
        "login", "logtstamp", "mapdefault", "mapnotnext",
        "maptimeout", "markkeys", "meta", "monitor",
        "multiuser", "nethack", "next", "nonblock", "number",
        "obuflimit", "only", "other", "partial", "password",
        "paste", "pastefont", "pow_break", "pow_detach",
        "prev", "printcmd", "process", "quit", "readbuf",
        "readreg", "redisplay", "remove", "removebuf", "reset",
        "resize", "screen", "scrollback", "select",
        "sessionname", "setenv", "setsid", "shell",
        "shelltitle", "silence", "silencewait", "sleep",
        "slowpast", "source", "sorendition", "split", "stuff",
        "su", "suspend", "term", "termcap", "terminfo",
        "termcapinfo", "unsetenv", "utf8", "vbell",
        "vbell_msg", "vbellwait", "verbose", "version",
        "width", "windowlist", "windows", "wrap", "writebuf",
        "writelock", "xoff", "xon", "zombie"
    };

    _ns_efuns *efuns;
    _ns_sess *s = (_ns_sess *) xd;
    int nsc = sizeof(sc) / sizeof(char *);

    if (NS_EFUN_EXISTS(efuns, s, NULL, inp_tab))
        return efuns->inp_tab((void *) s, sc, nsc, b, l, m) < 0 ? NS_FAIL : NS_SUCC;

    D_ESCREEN(("ns_screen_command: sess->efuns->inp_tab not set!\n"));
    return NS_EFUN_NOT_SET;
}



/* parse argument to screen's "escape" statement.
   x   points to the char to process
       screen-manual says this can be one of x ^X \123 or \\ \^ ...
  !x   the pointer is advanced to the next segment (from esc to literal etc.)
   <-  return as char ('\0' -> fail) */

char
ns_parse_esc(char **x)
{
    char r = '\0';

    if (**x == '\\') {
        (*x)++;
        r = **x;
        if (r >= '0' && r <= '7') {     /* octal, otherwise literal */
            char b[4] = "\0\0\0";
            char *e = *x;
            size_t l = 0;

            while ((*e >= '0' && *e <= '7') && (l < 3)) {       /* can't use endptr here : ( */
                e++;
                l++;
            }
            *x = &e[-1];
            while (--l)
                b[l] = *(--e);
            r = (char) strtol(b, &e, 8);
        }
    } else if (**x == '^') {
        (*x)++;
        r = **x;
        if (r >= 'A' && r <= 'Z')
            r = 1 + r - 'A';
        else if (r >= 'a' && r <= 'z')
            r = 1 + r - 'a';
        else
            r = '\0';
    } /* malformed */
    else
        r = **x;

    if (**x)
        (*x)++;
    return r;
}



/* ns_parse_screen_cmd
   parse a command the user intends to send to the screen program,
   either via .screenrc or using ^A:
   s       the affected (current) session.  s->current should be set.
   p       the command
   whence  which parsing stage (screenrc, interactive, ...)
   <-  error code */

int
ns_parse_screen_cmd(_ns_sess * s, char *p, int whence)
{
    char *p2;
    long v1 = -1;

    if (!p || !*p)
        return NS_FAIL;

    if ((p2 = strchr(p, ' '))) {        /* first argument */
        char *e;

        while (isspace(*p2))
            p2++;
        v1 = strtol(p2, &e, 0); /* magic conversion mode */
        if ((p2 == e) || (v1 < 0))
            v1 = -1;
    }
#define IS_CMD(b) (strncasecmp(p,b,strlen(b))==0)
    if (!p2) {
        D_ESCREEN(("screenrc: ignoring  \"%s\" without an argument...\n", p));
        /* must return success so it's fowarded to screen in interactive mode.
           that way, the user can read the original reply instead of a fake
           one from us. */
        return NS_SUCC;
    } else if (IS_CMD("defescape"))
        D_ESCREEN(("screenrc: ignoring  \"defescape\", did you mean \"escape\"?\n"));
    else if (IS_CMD("defhstatus") || IS_CMD("hardstatus") || IS_CMD("echo") || IS_CMD("colon") || IS_CMD("wall") ||
#ifdef NS_PARANOID
             IS_CMD("nethack") ||
#endif
             IS_CMD("info") || IS_CMD("time") || IS_CMD("title") || IS_CMD("lastmsg") || IS_CMD("msgwait") || IS_CMD("msgminwait")) {
        D_ESCREEN(("screenrc: ignoring  \"%s\", not applicable...\n", p));
        return NS_NOT_ALLOWED;
    } else if (IS_CMD("escape")) {
        char x = 0, y = 0;

        if ((x = ns_parse_esc(&p2)) && (y = ns_parse_esc(&p2))) {
            if (s->escdef == NS_ESC_CMDLINE) {
                D_ESCREEN(("screenrc: ignoring  \"escape\"; overridden on command-line...\n", x, y));
                return NS_NOT_ALLOWED;
            } else {
                s->escape = x;
                s->literal = y;
                s->escdef = whence;
                return NS_SUCC;
            }
        } else
            D_ESCREEN(("screenrc: ignoring  \"escape\" because of invalid arguments %o %o...\n", x, y));
    } else if (IS_CMD("defscrollback")) {
        if (v1 < NS_SCREEN_DEFSBB)
            D_ESCREEN(("screenrc: ignoring  \"%s\" for value < %d...\n", p, NS_SCREEN_DEFSBB));
        else {
            s->dsbb = v1;
            return NS_SUCC;
        }
    } else if (IS_CMD("scrollback")) {
        if (v1 < NS_SCREEN_DEFSBB)
            D_ESCREEN(("screenrc: ignoring  \"%s\" for value < %d...\n", p, NS_SCREEN_DEFSBB));
        else {
            if (!s->curr)
                s->curr = s->dsps;
            if (!s->curr)
                D_ESCREEN(("screenrc: ignoring  \"%s\", cannot determine current display!?...\n", p));
            else
                s->curr->sbb = v1;
            return NS_SUCC;
        }
    } else {
        D_ESCREEN(("screenrc: bored now \"%s\"\n", p));
        return NS_SUCC;
    }
    return NS_FAIL;
}



/* ns_parse_screen_key
   parse and forward a screen-hotkey
   s    the session to forward to
   c    the character following the escape-char.  (when we got here,
        we already got (and threw out) a screen-escape, so we'll have
        to also send one if we ever forward c to the screen program.
   <-   error code */

int
ns_parse_screen_key(_ns_sess * s, char c)
{
    int ret = NS_SUCC;
    char b[3];

    b[0] = s->escape;
    b[1] = c;
    b[2] = '\0';

    if (c < 27)
        D_ESCREEN(("screen_key: ^%c-^%c %d\n", s->escape + 'A' - 1, c + 'A' - 1, c));
    else
        D_ESCREEN(("screen_key: ^%c-%c %d\n", s->escape + 'A' - 1, c, c));

    switch (c) {
      case NS_SCREEN_CMD:      /* send command (statement) to screen server */
          ns_statement(s, NULL);
          break;
      case NS_SCREEN_RENAME:   /* rename current display */
          ret = ns_ren_disp(s, -1, NULL);
          break;
      case NS_SCREEN_KILL:
          ret = ns_rem_disp(s, -1, TRUE);
          break;
      default:
          ret = ns_screen_command(s, b);
    }

    return ret;
}



/* ns_parse_screen_interactive
   parse a whole string that may contain screen-escapes that should be
   handled interactively (that should open dialog boxes etc.).
   this will normally be called by menus, buttons etc. that want to send
   input to the add without generating X events for the keystrokes (real
   keystrokes do not come through here; the keyboard-handler should call
   ns_parse_screen_key() directly when it sees the session's escape-char).
   s   the session in question
   c   the string to parse
   <-  error code */

int
ns_parse_screen_interactive(_ns_sess * sess, char *c)
{
    char *s, *p, *o;

    if (!c || !*c)
        return NS_FAIL;
#ifdef NS_PARANOID
    if (!(s = o = STRDUP(c)))
        return NS_FAIL;
#else
    s = c;
#endif

    p = s;

    while ((p = strchr(s, NS_SCREEN_ESCAPE))) {
        *p = '\0';
        (void) ns_screen_command(sess, s);
        *p = NS_SCREEN_ESCAPE;
        if (*(++p))
            ns_parse_screen_key(sess, *(p++));
        s = p;
    }
    (void) ns_screen_command(sess, s);

#ifdef NS_PARANOID
    FREE(o);
#endif

    return NS_SUCC;
}



/* ns_parse_screenrc -- read the user's screenrc (if we can find it),
   parse it (we need to know if she changes the escapes etc.), and
   send it to the actually screen
   s       the session
   fn      name of the file in question
   whence  which screenrc are we in?
   <-      error code */

static int
ns_parse_screenrc(_ns_sess * s, char *fn, int whence)
{
    int fd = -1;
    char *rc = NULL;

    if (fn) {
        struct stat st;
        ssize_t rd = 0;

        if ((fd = open(fn, 0)) >= 0) {
            if (!fstat(fd, &st)) {
                if ((rc = MALLOC(st.st_size + 1))) {
                    char *p;

                    while (((rd = read(fd, rc, st.st_size)) < 0) && (errno == EINTR));
                    if (rd < 0)
                        goto fail;
                    rc[rd] = '\0';

                    p = rc;
                    while (*p) {
                        char *p2 = p, *n;
                        int f = 0;

                        while (*p2 && *p2 != '\n' && *p2 != '\r')       /* find EOL */
                            p2++;
                        n = p2;
                        while (*n == '\r' || *n == '\n')        /* delete EOL */
                            *(n++) = '\0';
                        while (isspace(*p))
                            p++;

                        p2 = p; /* on first non-white */
                        while (*p2) {
                            if (*p2 == '\\') {
                                p2++;
                                if (*p2)        /* sanity check */
                                    p2++;
                            } else {
                                if (*p2 == '\"')
                                    f = 1 - f;
                                if (!f && *p2 == '#')   /* comment, kill to EOL */
                                    *p2 = '\0';
                                else
                                    p2++;
                            }
                        }

                        if (strlen(p))  /* any commands in line? */
                            ns_parse_screen_cmd(s, p, whence);
                        p = n;  /* done, next line */
                    }
                    FREE(rc);
                    close(fd);
                    return NS_SUCC;
                }
            }
        }
    }

  fail:
    if (fd >= 0)
        close(fd);
    if (rc)
        FREE(rc);
    return NS_FAIL;
}




/* parse a message (not a display-list) set by the "screen" program
   screen   the session associated with that instance of screen,
            as returned by ns_attach_by_URL() and related.
            the session must contain a valid struct of callbacks (efuns),
            as certain functionalities ("add a tab", "show status message")
            may be called from here.
   p        the offending message-line
   <-       returns an error code. */

static int
ns_parse_screen_msg(_ns_sess * screen, char *p)
{
    _ns_efuns *efuns;
    char *p2;
    char vdate[33], vtype[3], vrem[17];
    int ma, mi, mu, ret = NS_SUCC, type;

    if (!p)
        return NS_FAIL;

    if (*p == ':')
        p++;
    while (isspace(*p))
        p++;

    type = (strlen(p) > 1) ? NS_SCREEN_STATUS : NS_SCREEN_ST_CLR;

    if (type == NS_SCREEN_ST_CLR) {
        if (NS_EFUN_EXISTS(efuns, screen, NULL, err_msg))
            ret = efuns->err_msg(NULL, type, "");
    }
    /* a screen display can disappear because the program in it dies, or
       because we explicitly ask screen to kill the display.  in the latter
       case, screen messages upon success.  rather than explicitly killing
       the disp-struct here, we force a status-line update instead (in which
       the status-line checker will notice the disp has gone, and delete it
       from the struct-list).  this way, we won't need to duplicate the
       delete-logic here. */
    else if (!strncmp(p, "Window ", strlen("Window ")) && (p2 = strrchr(p, ' ')) && !strcmp(p2, " killed.")) {
        ret = ns_upd_stat(screen);
        p = NULL;
    } else if (!strncmp(p, NS_SCREEN_SESS_T, strlen(NS_SCREEN_SESS_T))) {
        if (screen->name) {
            FREE(screen->name);
        }
        if ((screen->name = STRDUP(&p[strlen(NS_SCREEN_SESS_T)]))) {
            size_t lsn = strlen(screen->name);

            if (lsn) {
                screen->name[--lsn] = '\0';
            }
            D_ESCREEN(("ns_parse_screen_msg: session is \"%s\"\n", screen->name));
        }
        p = NULL;
    } else if (!strcmp(p, "New screen...") ||
               !strncmp(p, "msgwait", strlen("msgwait")) ||
               !strncmp(p, "msgminwait", strlen("msgminwait")) ||
               !strcmp(p, "Press ^@ to destroy or ^@ to resurrect window") || !strcmp(p, "Aborted because of window size change."))
        p = NULL;
    else if (sscanf(p, NS_SCREEN_VERSION_T, vtype, &ma, &mi, &mu, vrem, vdate) == 6) {
        if (!strcmp("en", vtype))
            screen->backend = NS_MODE_SCREEN;
        else if (!strcmp("am", vtype))
            screen->backend = NS_MODE_SCREAM;
        p = NULL;
        D_ESCREEN(("ns_parse_screen_msg: scre%s %d.%2d.%2d %s a/o %s -> mode %d\n", vtype, ma, mi, mu, vrem, vdate, screen->backend));
    } else if (!strcmp(p, NS_SCREEN_NO_DEBUG))
        p = "debug info was not compiled into \"screen\"...";
    else if (!strncmp(p, NS_SCREEN_DK_CMD_T, strlen(NS_SCREEN_DK_CMD_T))) {
        p[strlen(p) - 1] = '\0';
        p2 = &p[strlen(NS_SCREEN_DK_CMD_T)];
        p = "unknown screen statement ignored";
    }
    if (p) {                    /* status. send to status-line or dialog or whatever */
        if (NS_EFUN_EXISTS(efuns, screen, NULL, err_msg))
            ret = efuns->err_msg(NULL, type, p);
    }
    return ret;
}



/* parse the "hardstatus"-line of screens.
   this is, and unfortunately has to be, a ton of heuristics.
   I'm pretty sure there will be (esoteric) situations that are not handled
   (correctly) by this code, particularly in connection with more sessions
   than can be enumerated in the status-line (we do have workarounds for
   that case, they're just not very well tested yet).
   do not touch this unless you are absolutely sure you know what you're
   doing.   2002/05/01  Azundris  <scream@azundris.com>

   screen   the session associated with that instance of screen,
            as returned by ns_attach_by_URL() and related.
            the session must contain a valid struct of callbacks (efuns),
            as certain functionalities ("add a tab", "show status message")
            may be called from here.
   force    the terminal wants us to update.  if it doesn't, we may see
            fit to do so anyway in certain cases.
   width    the terminal's width in columns (ie that of the status line)
   p        the pointer to the status line.  may point into the terminal's
            line buffer if that holds plain text data (not interleaved with
            colour- and boldness-data)
   <-       returns an error code. */

int
ns_parse_screen(_ns_sess * screen, int force, int width, char *p)
{
    char *p4, *p3, *p2;         /* pointers for parser magic */
    static const char *p5 = NS_SCREEN_FLAGS;
    static int l = sizeof(NS_SCREEN_FLAGS);
    size_t status_blanks = 0;   /* status-bar overflow? */
    int ret = NS_SUCC, tmp, parsed,     /* no of *visible* elements in status line */
     n,                         /* screen's index (immutable, sparse) */
     r;                         /* real index (r'th element) */
    _ns_efuns *efuns;
    _ns_disp *disp = NULL, *d2 = NULL;

    if (!screen || !p || !width)
        return NS_FAIL;

    if (!force && screen->timestamp)
        return NS_SUCC;

    if ((p = STRDUP(p))) {
        _ns_parse pd[NS_MAX_DISPS];

        p2 = &p[width - 1];
        while (p2 > p && *p2 == ' ') {
            status_blanks++;
            *(p2--) = '\0';
        }                       /* p2 now points behind last item */

        D_ESCREEN(("parse_screen: screen sends ::%s::\n", p));

        if (strlen(p) < 2) {    /* special case: display 0 */
            disp = screen->dsps;        /* might not get a status-line in d0! */
            if (disp && !(disp->flags & NS_SCREAM_CURR)) {      /* flags need updating */
                disp->flags |= NS_SCREAM_CURR;  /* set flag to avoid calling inp_text */
                ret = ns_upd_stat(screen);
            } /* more than once */
            else if (!screen->timestamp) {
                /* send init string the first time around, just to be on
                   the safe side.  we could send it before entering this
                   function for the first time, but that would break if
                   escapes or screenrc were set from the
                   command-line. don't ask. */

                D_ESCREEN(("parse_screen: preparing screen...\n"));

                if (screen->delay > 0) {
                    screen->timestamp = time(NULL) + screen->delay;
                    if (NS_EFUN_EXISTS(efuns, screen, NULL, waitstate)) {
                        ret = efuns->waitstate(NULL, screen->delay * 1000);
                    }
                    (void) ns_upd_stat(screen);
                } else {
                    (void) ns_screen_command(screen, NS_SCREEN_INIT);
                    screen->timestamp = 1;
                }
            } else if ((screen->timestamp > 1) && (time(NULL) >= screen->timestamp)) {
                (void) ns_screen_command(screen, NS_SCREEN_INIT);
                screen->timestamp = 1;
                D_ESCREEN(("parse_screen: resetting screen...\n"));
            } else {
                D_ESCREEN(("parse_screen: nothing to do in exception, updating anyways...\n"));
                ret = ns_upd_stat(screen);
            }
            FREE(p);
            return ret;
        } else if (screen->backend == NS_MODE_NEGOTIATE) {
            /* I can't believe we haven't decided on a backend yet!  Ask! */
            (void) ns_screen_command(screen, NS_SCREEN_VERSION);
            (void) ns_screen_command(screen, NS_SCREEN_SESSION);
            screen->timestamp = time(NULL);
        }

        p3 = p;
        while (isspace(*p3))    /* skip left padding */
            p3++;

        if (isdigit(*p3)) {     /* list of displays */
            parsed = r = 0;
            do {
                n = atoi(p3);
                pd[parsed].name = NULL;
                pd[parsed].screen = n;
                pd[parsed].real = r++;

                while (isdigit(*p3))    /* skip index */
                    p3++;

                pd[parsed].flags = 0;   /* get and skip flags */
                while (*p3 && *p3 != ' ') {
                    for (n = 0; n < l; n++) {
                        if (*p3 == p5[n]) {
                            pd[parsed].flags |= (1 << n);
                            break;
                        }
                    }
                    p3++;
                }

                if (*p3 == ' ') {       /* skip space, read name */
                    *(p3++) = '\0';
                    p4 = p3;
                    while (p3[0] && p3[1] && (p3[0] != ' ' || p3[1] != ' '))
                        p3++;
                    if (p3[0] == ' ') {
                        *(p3++) = '\0';
                        while (isspace(*p3))
                            p3++;
                    }
                    pd[parsed++].name = p4;
                    if (parsed >= NS_MAX_DISPS)
                        p3 = &p3[strlen(p3)];
                } /* out of mem => skip remainder */
                else
                    p3 = &p3[strlen(p3)];       /* weirdness  => skip remainder */
            } while (*p3);

            for (r = 0; r < parsed; r++) {
                n = pd[r].screen;
                disp = disp_fetch(screen, n);

                if (!disp) {    /* new display */
                    if (!(disp = disp_fetch_or_make(screen, n)) || !(disp->name = STRDUP(pd[r].name))) {
                        D_ESCREEN(("parse_screen: out of memory in new_display(%d)\n", n));
                        ret = NS_FAIL;
                    } else {
                        if (NS_EFUN_EXISTS(efuns, screen, NULL, ins_disp))
                            ret = efuns->ins_disp(screen->userdef, pd[r].real - 1, pd[r].screen, disp->name);
                    }
                } else if ((tmp = strcmp(disp->name, pd[r].name)) ||    /* upd display */
                           (disp->flags != pd[r].flags)) {
                    if (tmp) {
                        FREE(disp->name);
                        if (!(disp->name = STRDUP(pd[r].name))) {
                            FREE(p);
                            return NS_FAIL;
                        }
                    }
                    if (pd[r].flags & NS_SCREAM_CURR)
                        disp->sess->curr = disp;
                    disp->flags = pd[r].flags & NS_SCREAM_MASK;
                    if (NS_EFUN_EXISTS(efuns, screen, NULL, upd_disp))
                        ret = efuns->upd_disp(screen->userdef, r, disp->flags, disp->name);
                }

                /* remove any displays from list that have disappeared
                   from the middle of the status-line */
                if (!d2 || d2->next != disp) {  /* remove expired displays */
                    _ns_disp *d3 = disp->prvs, *d4;

                    while (d3 && d3 != d2) {
                        D_ESCREEN(("parse_screen: remove expired middle %d \"%s\"...\n", d3->index, d3->name));
                        d4 = d3->prvs;
                        if (NS_EFUN_EXISTS(efuns, screen, NULL, del_disp))
                            ret = efuns->del_disp(screen->userdef, disp_get_real_by_screen(screen, d3->index));
                        disp_kill(d3);
                        d3 = d4;
                    }
                }
                d2 = disp;
            }



#ifdef NS_PARANOID
            if (!r) {
                if (!(err_inhibit & NS_ERR_WEIRDSCREEN)) {
                    err_inhibit |= NS_ERR_WEIRDSCREEN;
                    fprintf(stderr, "parse_screen: !r\n"
                            "This should never happen. It is assumed that you use a\n"
                            "rather unusual configuration for \"screen\".   Please\n"
                            "send the result of 'screen --version' to <scream@azundris.com>\n"
                            "(together with your ~/.screenrc and /etc/screenrc if present).\n"
                            "If at all possible, please also run 'Eterm -e screen' and make\n"
                            "a screenshot of the offending window (and the window only, the\n" "beauty of your desktop is not relevant to this investigation. : ).\n");
                }
                ret = ns_upd_stat(screen);
                FREE(p);
                return NS_FAIL;
            } else
#endif
                /* kill overhang (o/t right) if status-line isn't side-scrolling
                   (as it will if not all the disp names fit in the status-line) */
            if (disp->next && status_blanks > (strlen(disp->next->name) + 6)) {
                _ns_disp *d3 = disp;

                for (disp = disp->next; disp;) {
                    D_ESCREEN(("parse_screen: remove expired right %d \"%s\"...\n", disp->index, disp->name));
                    d2 = disp;
                    if (d2->sess->curr == d2)
                        d2->sess->curr = d3;
                    disp = disp->next;
                    if (NS_EFUN_EXISTS(efuns, screen, NULL, del_disp))
                        ret = efuns->del_disp(screen->userdef, disp_get_real_by_screen(screen, d2->index));
                    disp_kill(d2);
                }
                d3->next = NULL;
            }
        }

        else                    /* not a list of displays, but a message. handle separately. */
            ret = ns_parse_screen_msg(screen, p);

        FREE(p);                /* release our (modified) copy of the status-line */
    }

    return ret;
}

#endif



/****************************************************************************
 _____                _                       _ 
|  ___| __ ___  _ __ | |_       ___ _ __   __| |
| |_ | '__/ _ \| '_ \| __|____ / _ \ '_ \ / _` |
|  _|| | | (_) | | | | ||_____|  __/ | | | (_| |
|_|  |_|  \___/|_| |_|\__|     \___|_| |_|\__,_|
                                                

frontend abstraction (callbacks for messages to the client)

   this abstracts the frontend against the backend; the abstraction-layer
   (libscream) calls these in response to message from the backend (screen,
   or whatever) without really knowing what terminal-emulator (Eterm,
   konsole, multi-gnome-terminal, ...) the frontend is. */



/* function that moves horizontal scrollbar to x/1000 % of width */
void
ns_register_ssx(_ns_efuns * efuns, int (*set_scroll_x) (void *, int))
{
    efuns->set_scroll_x = set_scroll_x;
}

/* function that moves vertical scrollbar to y/1000 % of height */
void
ns_register_ssy(_ns_efuns * efuns, int (*set_scroll_y) (void *, int))
{
    efuns->set_scroll_y = set_scroll_y;
}

/* function that sets horizontal scrollbar to w/1000 % of width */
void
ns_register_ssw(_ns_efuns * efuns, int (*set_scroll_w) (void *, int))
{
    efuns->set_scroll_w = set_scroll_w;
}

/* function that sets vertical scrollbar to h/1000 % of height */
void
ns_register_ssh(_ns_efuns * efuns, int (*set_scroll_h) (void *, int))
{
    efuns->set_scroll_h = set_scroll_h;
}

/* function that redraws the terminal */
void
ns_register_red(_ns_efuns * efuns, int (*redraw) (void *))
{
    efuns->redraw = redraw;
}

/* function that redraw part of the terminal */
void
ns_register_rda(_ns_efuns * efuns, int (*redraw_xywh) (void *, int, int, int, int))
{
    efuns->redraw_xywh = redraw_xywh;
}

/* function that redraw part of the terminal */
void
ns_register_exb(_ns_efuns * efuns, int (*expire_buttons) (void *, int))
{
    efuns->expire_buttons = expire_buttons;
}

/* function to call when a new client was added ("add tab").
   after denotes the index of the button after which this one should
   be inserted (0..n, 0 denoting "make it the first button") */
void
ns_register_ins(_ns_efuns * efuns, int (*ins_disp) (void *, int, int, char *))
{
    efuns->ins_disp = ins_disp;
}

/* function to call when a client was closed ("remove tab") */
void
ns_register_del(_ns_efuns * efuns, int (*del_disp) (void *, int))
{
    efuns->del_disp = del_disp;
}

/* function to call when a client's title was changed ("update tab") */
void
ns_register_upd(_ns_efuns * efuns, int (*upd_disp) (void *, int, int, char *))
{
    efuns->upd_disp = upd_disp;
}

/* function to pass status lines to */
void
ns_register_err(_ns_efuns * efuns, int (*err_msg) (void *, int, char *))
{
    efuns->err_msg = err_msg;
}

/* function that will execute client programs (in pseudo-terminal et al) */
void
ns_register_exe(_ns_efuns * efuns, int (*execute) (void *, char **))
{
    efuns->execute = execute;
}

/* function that will hand text as input to the client */
void
ns_register_txt(_ns_efuns * efuns, int (*inp_text) (void *, int, char *))
{
    efuns->inp_text = inp_text;
}



/* function that will open a dialog */
void
ns_register_inp(_ns_efuns * efuns, int (*inp_dial) (void *, char *, int, char **, int (*)(void *, char *, size_t, size_t)))
{
    efuns->inp_dial = inp_dial;
}



/* function that will handle tab-completion in a dialog */
void
ns_register_tab(_ns_efuns * efuns, int (*inp_tab) (void *, char *[], int, char *, size_t, size_t))
{
    efuns->inp_tab = inp_tab;
}



/* function that will do whatever while waiting */
void
ns_register_fun(_ns_efuns * efuns, int (*inp_fun) (void *, int))
{
    efuns->waitstate = inp_fun;
}



/* get callbacks.  at least one of session and display must be non-NULL.
   s  session, or NULL. if NULL, will be initialized from d->sess
   d  display, or NULL. if NULL, will be initialized from s->curr.
                        if set, will override session callbacks;
                        note that NULL pointers in d->efuns *will*
                        override (disable) non-NULL pointers in s->efuns!
   <- callback-struct */

_ns_efuns *
ns_get_efuns(_ns_sess * s, _ns_disp * d)
{
    if (!s) {
        if (!d || !d->sess)
            return NULL;
        else
            s = d->sess;
    }
    if (!d)
        d = s->curr;
    if (d && d->efuns)
        return d->efuns;
    else
        return s->efuns;
}



/* ns_inp_dial
   open a dialog (wrapp around efuns->inp_dial)
   s        the session
  !retstr   where we'll store a pointer to the result (the user's input)
   prompt   the prompt to appear in the dialog box
   <-       msg */


int
ns_inp_dial(_ns_sess * s, char *prompt, int maxlen, char **retstr, int (*inp_tab) (void *, char *, size_t, size_t))
{
    _ns_efuns *efuns;
    int ret = NS_SUCC;

    if (NS_EFUN_EXISTS(efuns, s, NULL, inp_dial)) {
        (void) efuns->inp_dial((void *) s, prompt, maxlen, retstr, inp_tab);
    } else {
        ret = NS_EFUN_NOT_SET;
        D_ESCREEN(("ns_inp_dial: sess->efuns->inp_dial not set!\n"));
    }
    return ret;
}



/***************************************************************************/
