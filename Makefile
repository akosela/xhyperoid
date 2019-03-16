# makefile for xhyperoid

# You need an ANSI C compiler, e.g. gcc.
CC=gcc

CFLAGS=-O2 -Wall

# You'll need to comment this out on most non-Linux systems to disable
# the sound. Linuxers should leave it as-is.
#
CFLAGS+=-DDO_SOUND

# Where things are installed by `make install':
#
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
XBINDIR=$(BINDIR)
MANDIR=$(PREFIX)/share/man/man6
SOUNDSDIR=$(PREFIX)/share/xhyperoid
#
# if you want the X version to be installed in the usual X executables
# directory, uncomment this:
#
#XBINDIR=/usr/X11R6/bin

# you shouldn't need to change anything below this point.
#--------------------------------------------------------------------

CFLAGS+=-DSOUNDSDIR=\"$(SOUNDSDIR)\"


OBJS=hyperoid.o roidsupp.o sound.o svga.o convxpm.o
XOBJS=hyperoid.o roidsupp.o sound.o gtk.o

all: x vga

x: xhyperoid

vga: vhyperoid

vhyperoid: $(OBJS)
	$(CC) -o vhyperoid $(OBJS) -lvga -lm

xhyperoid: $(XOBJS)
	$(CC) -o xhyperoid $(XOBJS) `gtk-config --libs`

gtk.o: gtk.c
	$(CC) $(CFLAGS) `gtk-config --cflags` -c gtk.c -o gtk.o

installdirs:
	/bin/sh ./mkinstalldirs $(BINDIR) $(XBINDIR) $(MANDIR) $(SOUNDSDIR)

# check at least one executable has been made
check-made:
	@if [ ! -f vhyperoid -a ! -f xhyperoid ]; then \
	  echo 'do "make vga", "make x", or "make" first (see README).'; \
	  false; \
	fi

install: check-made installdirs
	if [ -f vhyperoid ]; then \
	install -o root -m 4511 vhyperoid $(BINDIR); fi
	if [ -f xhyperoid ]; then \
	install -m 511 xhyperoid $(XBINDIR); fi
	install -m 444 xhyperoid.6 $(MANDIR)
	ln -f $(MANDIR)/xhyperoid.6 $(MANDIR)/vhyperoid.6
	chmod 555 $(SOUNDSDIR)
	install -m 444 sounds/*.ub $(SOUNDSDIR)

# I delete the sounds a slightly odd way to save having "rm -fr",
# which I'd prefer to avoid just in case SOUNDSDIR is bogus. :-)
uninstall:
	$(RM) $(BINDIR)/vhyperoid $(XBINDIR)/xhyperoid
	$(RM) $(MANDIR)/hyperoid.6* $(MANDIR)/[vx]hyperoid.6*
	$(RM) $(SOUNDSDIR)/*
	rmdir $(SOUNDSDIR)

clean:
	$(RM) vhyperoid xhyperoid *.o *~


# The stuff below makes the distribution tgz.
#
VERS=1.2

dist: tgz
tgz: ../xhyperoid-$(VERS).tar.gz
  
# Based on the example in ESR's Software Release Practice HOWTO.
#
../xhyperoid-$(VERS).tar.gz: clean
	$(RM) ../xhyperoid-$(VERS)
	@cd ..;ln -s xhyperoid xhyperoid-$(VERS)
	cd ..;tar zchvf xhyperoid-$(VERS).tar.gz xhyperoid-$(VERS)
	@cd ..;$(RM) xhyperoid-$(VERS)
