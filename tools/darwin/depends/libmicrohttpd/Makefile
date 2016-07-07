include ../Makefile.include
DEPS= Makefile

# lib name, version
LIBNAME=libmicrohttpd
VERSION=0.9.48
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) \
  --disable-doc --disable-examples --disable-curl \
  --disable-spdy

LIBDYLIB=$(SOURCE)/src/daemon/.libs/$(LIBNAME).dylib

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -C $(SOURCE)/src/microhttpd
	make -C $(SOURCE)/src/include install

.installed: $(LIBDYLIB)
	make -C $(SOURCE)/src/microhttpd install
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
