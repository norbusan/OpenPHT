include ../Makefile.include
include ../config.site.mk
DEPS= Makefile

LIBNAME=boost
VERSION=1_60_0
SOURCE=$(LIBNAME)_$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

CONFIGURE=cxxflags="-std=c++11 -stdlib=libc++" linkflags="-stdlib=libc++"
ifeq ($(ARCH), i386)
  CONFIGURE=architecture=x86 address-model=32 cxxflags="-arch i386 -std=c++11 -stdlib=libc++" linkflags="-arch i386 -stdlib=libc++"
endif

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

.installed: $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); ./bootstrap.sh --prefix=$(PREFIX)
	cd $(SOURCE); ./b2 --prefix=$(PREFIX) toolset=clang-darwin variant=release link=static $(CONFIGURE) --with-thread --with-chrono --with-date_time --with-regex install
	touch .installed

clean:
	rm -rf $(SOURCE) .installed

distclean::
	rm -rf $(SOURCE) .installed

