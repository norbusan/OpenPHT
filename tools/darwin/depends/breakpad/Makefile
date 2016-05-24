include ../Makefile.include
include ../config.site.mk
DEPS= ../Makefile.include ../config.site.mk Makefile cmake.patch

# lib name, version
BASE_URL=https://chromium.googlesource.com/breakpad/breakpad/+archive
LIBNAME=breakpad
VERSION=chrome_49
SOURCE=$(LIBNAME)-$(VERSION)
FILENAME=$(VERSION).tar.gz
ARCHIVE=$(SOURCE).tar.gz

LIBDYLIB=$(SOURCE)/build/src/lib$(LIBNAME)_client.a

all: $(LIBDYLIB) .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(FILENAME)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(SOURCE); mkdir -p $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE) -C $(SOURCE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); rm -rf build; mkdir -p build
	cd $(SOURCE); patch -p1 < ../cmake.patch
	cd $(SOURCE)/build; CC="$(CC)" CXX="$(CXX)" cmake        \
	  -DCMAKE_C_FLAGS:STRING="$(CPPFLAGS) $(CFLAGS)"     \
	  -DCMAKE_CXX_FLAGS:STRING="$(CPPFLAGS) $(CXXFLAGS)" \
	  -DCMAKE_INCLUDE_PATH=$(PREFIX)/include             \
	  -DCMAKE_LIBRARY_PATH=$(PREFIX)/lib                 \
	  -DCMAKE_INSTALL_NAME_DIR=$(PREFIX)/lib             \
	  -DCMAKE_INSTALL_PREFIX=$(PREFIX)                   \
	  -DCMAKE_FIND_ROOT_PATH=$(platform_sdk_path) ..

$(LIBDYLIB): $(SOURCE)
	make -j 1 -C $(SOURCE)/build

.installed:
	make -C $(SOURCE)/build install
	touch $@

clean:
	rm -rf $(SOURCE) .installed

distclean::
	rm -rf $(SOURCE) .installed

