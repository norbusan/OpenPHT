include ../Makefile.include
include ../config.site.mk
DEPS= Makefile

# lib name, version
LIBNAME=libdcadec
VERSION=0e07438
SOURCE=dcadec-$(VERSION)
ARCHIVE=$(SOURCE).tar.xz

ifeq ($(ARCH), i386)
  EXTRA_FLAGS = CC="$(CC)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" AR="$(AR)"
endif
# configuration settings

LIBDYLIB=$(SOURCE)/libdcadec/$(LIBNAME).a

CFLAGS += -std=gnu99

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
ifeq ($(PREFIX),)
	@echo
	@echo "ERROR: please set PREFIX to the kodi install path e.g. make PREFIX=/usr/local"
	@exit 1
endif
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore

$(LIBDYLIB): $(SOURCE)
	make -C $(SOURCE) PREFIX=$(PREFIX) $(EXTRA_FLAGS)

.installed: $(LIBDYLIB)
	make -C $(SOURCE) PREFIX=$(PREFIX) $(EXTRA_FLAGS) install
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed
