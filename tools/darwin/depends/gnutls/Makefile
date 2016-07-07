include ../Makefile.include
DEPS= Makefile size-max.patch

# lib name, version
BASE_URL=ftp://ftp.gnutls.org/gcrypt/gnutls/v3.4
LIBNAME=gnutls
VERSION=3.4.12
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.xz
ARCHIVE_TOOL=/usr/local/bin/gtar

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) --disable-shared --without-p11-kit --with-included-libtasn1 --disable-nls --enable-local-libopts --disable-doc

LIBDYLIB=$(SOURCE)/lib/.libs/lib$(LIBNAME).a

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); patch -p0 < ../size-max.patch
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -C $(SOURCE)

.installed: $(LIBDYLIB)
	make -C $(SOURCE) install
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
