include ../Makefile.include
DEPS= Makefile 0001-HTTP-reset-expected-DL-UL-sizes-on-redirects.patch

# lib name, version
LIBNAME=curl
ifeq ($(ARCH), i386)
VERSION=7.21.6
else
VERSION=7.48.0
endif
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --without-libssh2 --disable-ldap --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-manual
CONFIGURE_IOS=$(CONFIGURE) --with-random=/dev/urandom

LIBDYLIB=$(SOURCE)/lib/.libs/lib$(LIBNAME).dylib

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
ifeq ($(ARCH), i386)
	cd $(SOURCE); patch -p1 < ../0001-HTTP-reset-expected-DL-UL-sizes-on-redirects.patch
endif
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -C $(SOURCE)/lib

.installed: $(LIBDYLIB)
	# install only libcurl and includes
	make -C $(SOURCE)/lib install
	make -C $(SOURCE)/include install
	# make sure to also install libcurl.pc, we need to do
	# this because we are selectively installing only libcurl
	make -C $(SOURCE) install-pkgconfigDATA
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
