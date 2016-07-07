include ../Makefile.include
include ../config.site.mk
DEPS= Makefile

# lib name, version
LIBNAME=yajl
VERSION=2.1.0
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz
# configuration settings
LIBDYLIB=$(SOURCE)/build/$(LIBNAME)-$(VERSION)/lib/libyajl.dylib

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	sed -ie "s|ADD_CUSTOM_TARGET|#ADD_CUSTOM_TARGET|g" "$(SOURCE)/CMakeLists.txt"
	sed -ie "s|./run_tests.sh|#./run_tests.sh|g" "$(SOURCE)/CMakeLists.txt"
	sed -ie "s|WORKING_DIRECTORY|#WORKING_DIRECTORY|g" "$(SOURCE)/CMakeLists.txt"
	sed -ie "s|ADD_SUBDIRECTORY(reformatter)|#ADD_SUBDIRECTORY(reformatter)|g" "$(SOURCE)/CMakeLists.txt"
	cd $(SOURCE); rm -rf build; mkdir -p build
	cd $(SOURCE)/build; CC="$(CC)" CXX="$(CXX)" cmake        \
	  -DCMAKE_C_FLAGS:STRING="$(CPPFLAGS) $(CFLAGS)"     \
	  -DCMAKE_CXX_FLAGS:STRING="$(CPPFLAGS) $(CXXFLAGS)" \
	  -DCMAKE_INCLUDE_PATH=$(PREFIX)/include             \
	  -DCMAKE_LIBRARY_PATH=$(PREFIX)/lib                 \
	  -DCMAKE_INSTALL_NAME_DIR=$(PREFIX)/lib             \
	  -DCMAKE_INSTALL_PREFIX=$(PREFIX)                   \
	  -DCMAKE_FIND_ROOT_PATH=$(platform_sdk_path) ..

$(LIBDYLIB): $(SOURCE)
	make -C $(SOURCE)/build

.installed: $(LIBDYLIB)
	make -C $(SOURCE)/build install
	touch $@

clean:
	make -C $(SOURCE)/build clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
