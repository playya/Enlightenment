#!/bin/sh

NAME=embrace
DB=~/.e/apps/embrace/$NAME.db

# generic mandatory attributes:
# * title
# * type => plugin to use
#
# generic optional attributes:
#  * interval => poll interval in seconds (default is 60)
#
# each plugin may read additional attributes:
#
# pop3 and imap: str host, int port, str user, str pass
# mbox, maildir and imap: str path

edb_ed $DB add /$NAME/theme str "default"
edb_ed $DB add /$NAME/num_mailboxes int 4

edb_ed $DB add /$NAME/mailbox1/title str "inbox"
edb_ed $DB add /$NAME/mailbox1/path str "~/Mail/inbox"
edb_ed $DB add /$NAME/mailbox1/type str "maildir"

edb_ed $DB add /$NAME/mailbox2/title str "e-cvs"
edb_ed $DB add /$NAME/mailbox2/path str "~/Mail/enlightenment-cvs"
edb_ed $DB add /$NAME/mailbox2/type str "mbox"

edb_ed $DB add /$NAME/mailbox3/title str "my pop3 box"
edb_ed $DB add /$NAME/mailbox3/host str "foo.bar.com"
edb_ed $DB add /$NAME/mailbox3/port int 123
edb_ed $DB add /$NAME/mailbox3/type str "pop3"
edb_ed $DB add /$NAME/mailbox3/user str "myuser"
edb_ed $DB add /$NAME/mailbox3/pass str "mypass"

edb_ed $DB add /$NAME/mailbox4/title str "my imap mailbox"
edb_ed $DB add /$NAME/mailbox4/host str "foo.bar.com"
edb_ed $DB add /$NAME/mailbox4/port int 143
edb_ed $DB add /$NAME/mailbox4/type str "imap"
edb_ed $DB add /$NAME/mailbox4/user str "myuser"
edb_ed $DB add /$NAME/mailbox4/pass str "mypass"
edb_ed $DB add /$NAME/mailbox4/path str "path.to.mailbox"
