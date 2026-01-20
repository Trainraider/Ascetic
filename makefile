.SUFFIXES:
.SUFFIXES: .c .o .h

include config.mk

#Functions_______________________________________________________________________
RM = -rm -f
RMRF = -rm -rf
CP = yes | cp -f

ROOTCHECK = \
	@echo;\
	if [[ $$EUID -ne 0 ]]; then\
		echo "error: you cannot perform this operation unless you are root.";\
		exit 1;\
	fi
#________________________________________________________________________________

ifeq ($(MAKECMDGOALS),debug)
    BLD = $(DBG)
else
    BLD = $(RLS)
endif
BLD := $(shell readlink -f $(BLD))

BIN           = $(BLD)/bin/$(TARGET)
OBJ           = $(BLD)/main.o $(BLD)/version.o $(BLD)/data.o $(BLD)/settings_page.o
UI            = $(BLD)/window_main.ui $(BLD)/settings_page.ui

DESKTOP       = $(BLD)/$(TARGET).desktop

#Required flags
CPPFLAGS += -DVERSION=\"$(VERSION)\" -DNAME=\""$(NAME)"\" -DAPP_ID=\"$(APP_ID)\" -DAPP_PREFIX=\"$(APP_PREFIX)\" -DAUTHOR=\""$(AUTHOR)"\" -DCOPYRIGHT="\"$(COPYRIGHT)\"" -DTARGET=\"$(TARGET)\"
CFLAGS   += $(INCS) -I$(BLD)
LDFLAGS  += $(LIBS) -rdynamic

all: CFLAGS  += $(RELEASE_CFLAGS)
all: LDFLAGS += $(RELEASE_LDFLAGS)
all: options $(BIN) $(DESKTOP) | mkdirs

debug: CPPFLAGS += $(DEBUG_CPPFLAGS)
debug: CFLAGS  += $(DEBUG_CFLAGS)
debug: LDFLAGS += $(DEBUG_LDFLAGS)
debug: options $(BIN) | mkdirs


options:
	@echo Build options:
	@echo ""
	@echo "Output Dir: $(BLD)"
	@echo ""
	@echo "CFLAGS  = $(CFLAGS)"
	@echo ""
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo ""
	@echo "CC      = $(CC)"
	@echo ""

$(BIN): $(OBJ) | mkdirs
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)
	@echo ""
	@echo "Build successful."

#General rule for compiling object files
#make a specific rule if you depend on a header you might change
$(BLD)/%.o : $(SRC)/%.c | mkdirs
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

#Specific rules
$(BLD)/main.o : $(SRC)/main.c $(SRC)/version.h $(BLD)/data.h $(SRC)/settings_page.h | mkdirs
$(BLD)/version.o : $(SRC)/version.c $(SRC)/version.h | mkdirs
$(BLD)/data.o : $(BLD)/data.c $(BLD)/data.h | mkdirs
$(BLD)/settings_page.o: $(SRC)/settings_page.h | mkdirs

$(BLD)/data.c : $(BLD)/data.gresource.xml $(BLD)/window_main.ui $(RESOURCES) $(UI) | mkdirs
	@glib-compile-resources --sourcedir=$(BLD) --sourcedir=$(DATA) --generate-source $< --target=$@;\

$(BLD)/data.h : $(BLD)/data.gresource.xml $(BLD)/window_main.ui  $(RESOURCES) $(UI) | mkdirs
	@cd $(DATA);\
	glib-compile-resources --sourcedir=$(BLD) --sourcedir=$(DATA) --generate-header $< --target=$@;\

#General rule to compile blueprint files
$(BLD)/%.ui: $(BLP)/%.blp | mkdirs
	@blueprint-compiler compile --output $@ $<

$(BLD)/data.gresource.xml : $(DATA)/data.gresource.xml.pre | mkdirs
	@sed 's:__PREFIX__:$(APP_PREFIX):' $< > $@

$(DESKTOP): | mkdirs
	@echo "[Desktop Entry]"                         > $@
	@echo "Version=$(VERSION)"                     >> $@
	@echo "Type=Application"                       >> $@
	@echo "Name=$(NAME)"                           >> $@
	@echo "Exec=$(DESTDIR)$(PREFIX)/bin/$(TARGET)" >> $@
	@echo "Comment=$(COMMENT)"                     >> $@
	@echo "Icon=$(TARGET)"                         >> $@
	@echo "Terminal=false"                         >> $@
	@echo "Categories=$(CATEGORIES)"               >> $@

mkdirs :
	-@mkdir -p $(BLD)/bin
	-@mkdir -p $(DBG)/bin

install : all
	$(ROOTCHECK)
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@$(CP) $(BIN) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	@mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	@$(CP) $(BLD)/$(TARGET).desktop $(DESTDIR)$(PREFIX)/share/applications/$(TARGET).desktop
	@mkdir -p $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps
	@$(CP) $(DATA)/icon.svg $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps/$(TARGET).svg
	@gtk-update-icon-cache -f -t $(DESTDIR)$(PREFIX)/share/icons/hicolor
	@update-desktop-database $(DESTDIR)$(PREFIX)/share/applications

uninstall :
	@$(ROOTCHECK)
	@$(RM) $(DESTDIR)$(PREFIX)/bin/$(TARGET) \
	$(DESTDIR)$(PREFIX)/share/applications/$(TARGET).desktop \
	$(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps/$(TARGET).svg
	@gtk-update-icon-cache -f -t $(DESTDIR)$(PREFIX)/share/icons/hicolor
	@update-desktop-database $(DESTDIR)$(PREFIX)/share/applications

clean :
	@$(RM) $(DATA)/*.xml~ $(DATA)/*.xml#
	@$(RMRF) $(BLD)
	@$(RMRF) $(DBG)

format :
	@clang-format -style="{BasedOnStyle: webkit, IndentWidth: 8,AlignConsecutiveDeclarations: true, AlignConsecutiveAssignments: true, ReflowComments: true, SortIncludes: true}" -i $(SRC)/*.{c,h}

.PHONY: all options debug install uninstall clean format mkdirs