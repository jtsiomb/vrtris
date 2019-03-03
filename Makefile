src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = vrtris


warn = -pedantic -Wall -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast
dbg = -g
opt = -O0

CFLAGS = $(warn) $(dbg) $(opt) `pkg-config --cflags sdl2 freetype2`
LDFLAGS = $(libsys) -ldrawtext $(libgl) `pkg-config --libs sdl2 freetype2` \
		  -lgoatvr -limago -lpng -lz -ljpeg -lpthread -lm

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')

ifeq ($(sys), mingw)
	obj = $(src:.c=.w32.o)
	dep	= $(obj:.o=.d)

	bin = vrtris.exe

	libgl = -lopengl32 -lglu32 -lglew32
	libsys = -lmingw32 -lSDL2main -lwinmm -mwindows

else ifeq ($(sys), Darwin)
	libgl = -framework OpenGL -lGLEW

else
	libgl = -lGL -lGLU -lGLEW
endif


$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.c
	@echo depfile $@
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: cross
cross:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw

.PHONY: cross-clean
cross-clean:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw clean

.PHONY: instalien
instalien: vrtris.exe
	cp $< /alien/vrtris/$<
	for i in `tools/dlldepends | grep -v '++'`; do echo $$i; rsync $$i /alien/vrtris; done

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(bin) $(data)
	mkdir -p $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/share/vrtris
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)
	cp $(data) $(DESTDIR)$(PREFIX)/share/vrtris

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bin)
	for i in $(data); do rm -f $(DESTDIR)$(PREFIX)/share/vrtris/$i; done
	rmdir $(DESTDIR)$(PREFIX)/share/vrtris
