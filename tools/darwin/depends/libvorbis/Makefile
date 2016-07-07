include ../Makefile.include
DEPS= Makefile

# lib name, version
LIBNAME=libvorbis
VERSION=1.3.5
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
PACKAGE_NAME=libvorbis

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) \
  PACKAGE_NAME=$(PACKAGE_NAME) \
  --disable-oggtest --disable-docs --disable-examples

LIBDYLIB=$(SOURCE)/lib/.libs/$(LIBNAME).dylib

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	sed -ie "s|SUBDIRS = m4 include vq lib examples test doc|SUBDIRS = m4 include lib|" "$(SOURCE)/Makefile.in"
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
