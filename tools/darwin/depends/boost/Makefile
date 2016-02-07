include ../Makefile.include
include ../config.site.mk

LIBNAME=boost
VERSION=1_60_0
SOURCE=$(LIBNAME)_$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

CONFIGURE=
ifeq ($(ARCH), i386)
  CONFIGURE=architecture=x86 address-model=32 cxxflags="-arch i386" linkflags="-arch i386"
endif

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

.installed: $(TARBALLS_LOCATION)/$(ARCHIVE)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); ./bootstrap.sh --prefix=$(PREFIX)
	cd $(SOURCE); ./b2 --prefix=$(PREFIX) toolset=clang-darwin variant=release link=static $(CONFIGURE) --with-thread --with-date_time --with-regex install
	touch .installed

clean:
	rm -rf $(SOURCE) .installed

distclean::
	rm -rf $(SOURCE) .installed

