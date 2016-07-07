include ../Makefile.include
include ../config.site.mk
DEPS= Makefile

# lib name, version
LIBNAME=glew
VERSION=1.13.0
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

# configuration settings
export SYSTEM=darwin
export GLEW_DEST=$(PREFIX)
export CC.ARCH=$(CC)
export CFLAGS.ARCH=$(CFLAGS)
export LDFLAGS.ARCH=$(LDFLAGS)

LIBDYLIB=$(SOURCE)/lib/libGLEW.a

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore

$(LIBDYLIB): $(SOURCE)
	sed -ie "s|CC = cc|CC = $(CC.ARCH)|" "$(SOURCE)/config/Makefile.darwin"
	sed -ie "s|LD = cc|LD = $(CC.ARCH)|" "$(SOURCE)/config/Makefile.darwin"
	sed -ie "s|CFLAGS.EXTRA =|CFLAGS.EXTRA = \$(CFLAGS.ARCH)|" "$(SOURCE)/config/Makefile.darwin"
	sed -ie "s|LDFLAGS.EXTRA =|LDFLAGS.EXTRA = \$(LDFLAGS.ARCH)|" "$(SOURCE)/config/Makefile.darwin"
	make -C $(SOURCE)

.installed: $(LIBDYLIB)
	make -C $(SOURCE) install
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
