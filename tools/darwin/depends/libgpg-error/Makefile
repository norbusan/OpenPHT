include ../Makefile.include
DEPS= Makefile

# lib name, version
LIBNAME=libgpg-error
VERSION=1.21
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.bz2

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) \
  --disable-languages \
  --disable-nls

LIBDYLIB=$(SOURCE)/src/.libs/$(LIBNAME).dylib

CLEAN_FILES=$(ARCHIVE) $(SOURCE)

all: .installed

$(TARBALLS_LOCATION)/$(ARCHIVE):
	$(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(SOURCE): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(SOURCE)
	$(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	echo $(SOURCE) > .gitignore
	cd $(SOURCE); $(CONFIGURE)

$(LIBDYLIB): $(SOURCE)
	make -C $(SOURCE)/src gen-posix-lock-obj
	$(SOURCE)/src/gen-posix-lock-obj > $(SOURCE)/src/syscfg/tmp.h
	mv $(SOURCE)/src/syscfg/tmp.h $(SOURCE)/src/syscfg/$$(awk 'NR==1 {print $$2}' $(SOURCE)/src/syscfg/tmp.h)
	make -C $(SOURCE)

.installed: $(LIBDYLIB)
	make -C $(SOURCE) install
	touch $@

clean:
	make -C $(SOURCE) clean
	rm -f .installed

distclean::
	rm -rf $(SOURCE) .installed

