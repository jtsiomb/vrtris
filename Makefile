src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = vrtris


CFLAGS = -pedantic -Wall -g `pkg-config --cflags sdl2`
LDFLAGS = $(libsys) $(libgl) `pkg-config --libs sdl2` -ldrawtext -lgoatvr \
		  -limago -lm

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')

ifeq ($(sys), mingw)
	bin = vrtris.exe

	libgl = -lopengl32 -lglew32
	libsys = -lmingw32 -lwinmm -mwindows

else ifeq ($(sys), Darwin)
	libgl = -framework OpenGL -lGLEW

else
	libgl = -lGL -lGLEW
endif


$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: cross
cross:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw

.PHONY: cross-clean
cross-clean:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw clean

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
