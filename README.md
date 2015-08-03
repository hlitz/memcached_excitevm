<<<<<<< HEAD
# Transactional Memcached

This repository contains code for a transactional version of memcached, based
on the Ruan et al. paper from ASPLOS 2014.  In that work, we developed
several versions of memcached.  We expect that only the last version
(internally, version 12b) will be of interest to TM users, and thus to avoid
confusion it is, for now, the only version of code present in the repository.

This work was performed before we implemented a proper solution for condition
variables in legacy code.  We hope to provide an updated version soon.

If you find any bugs in this code, please contact Mike Spear
(spear@cse.lehigh.edu).

If you use this code in a publication, please be sure to cite Ruan et
al. ASPLOS 2014.
=======
# Memcached

## Dependencies

* libevent, http://www.monkey.org/~provos/libevent/ (libevent-dev)

## Environment

### Linux

If using Linux, you need a kernel with epoll.  Sure, libevent will
work with normal select, but it sucks.

epoll isn't in Linux 2.4, but there's a backport at:

    http://www.xmailserver.org/linux-patches/nio-improve.html

You want the epoll-lt patch (level-triggered).

### Mac OS X

If you're using MacOS, you'll want libevent 1.1 or higher to deal with
a kqueue bug.

Also, be warned that the -k (mlockall) option to memcached might be
dangerous when using a large cache.  Just make sure the memcached machines
don't swap.  memcached does non-blocking network I/O, but not disk.  (it
should never go to disk, or you've lost the whole point of it)

## Website

* http://www.memcached.org

## Contributing

Want to contribute?  Up-to-date pointers should be at:

* http://contributing.appspot.com/memcached
>>>>>>> a7ece60c549c1efdcb81c7a6d028bf4bd5efee6f
