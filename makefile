.PHONY: all debug install uninstall clean format reconfigure

BUILDDIR = builddir
DEBUGDIR = debugdir

all:
	@test -d $(BUILDDIR) || meson setup $(BUILDDIR) --buildtype=release
	@meson compile -C $(BUILDDIR)

debug:
	@test -d $(DEBUGDIR) || meson setup $(DEBUGDIR) --buildtype=debug
	@meson compile -C $(DEBUGDIR)

install:
	@meson install -C $(BUILDDIR)

uninstall:
	@ninja -C $(BUILDDIR) uninstall

clean:
	@rm -rf $(BUILDDIR) $(DEBUGDIR)

reconfigure:
	@test -d $(BUILDDIR) && meson setup $(BUILDDIR) --reconfigure || true
	@test -d $(DEBUGDIR) && meson setup $(DEBUGDIR) --reconfigure || true

format:
	@clang-format -style="{BasedOnStyle: webkit, \
	IndentWidth: 8, \
	AlignConsecutiveDeclarations: true, \
	AlignConsecutiveAssignments: true, \
	ReflowComments: true, \
	SortIncludes: false, \
	MacroBlockBegin: 'S_', \
	MacroBlockEnd: '_S'}" \
	-i source/*.c source/*.h