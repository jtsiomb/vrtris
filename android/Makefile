root = ..
include $(root)/Makefile

android_platform = android-19
name = vrtris
pkgprefix = com.mutantstargoat

src += $(wildcard $(root)/src/android/*.c) \
	   $(wildcard $(root)/libs/openal_droid/*.c)
obj = $(src:.c=.arm.o)

vr_cflags =
vr_ldflags =

lib = libs/armeabi/lib$(name).so
apk-release = bin/$(name).apk
apk-debug = bin/$(name)-debug.apk

pkg = $(pkgprefix).$(name)
act = android.app.NativeActivity

opt = -O3
dbg = -g
warn = -pedantic -Wall
inc = -I$(root)/libs/openal_droid

CC = arm-linux-androideabi-gcc

CFLAGS = $(warn) $(opt) $(dbg) $(inc) -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16
LDFLAGS = -Wl,--fix-cortex-a8 -Wl,-z,defs -llog -landroid -lEGL -lGLESv2 -lOpenSLES

.PHONY: apk-debug
apk-debug: $(apk-debug)

.PHONY: apk
apk: $(apk-release)

$(apk-debug): $(lib) AndroidManifest.xml assets/data assets/sdr
	ant debug

$(apk-release): $(lib) AndroidManifest.xml assets/data assets/sdr
	ant release

assets/data:
	mkdir -p assets
	cd assets && rm -f data && ln -s ../../data .

assets/sdr:
	mkdir -p assets
	cd assets && rm -f sdr && ln -s ../../sdr .

.PHONY: lib
lib: $(lib)

$(lib): $(obj)
	@echo Linking $(lib)
	@mkdir -p libs/armeabi
	$(CC) -o $@ -shared $(obj) $(LDFLAGS)

%.arm.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

ant_files = *.xml *.properties propguard-project.txt
ant_dirs = bin libs res obj src gen

.PHONY: clean
clean:
	rm -f $(obj) $(lib) $(apk-release) $(apk-debug) $(ant_files)
	rm -rf $(ant_dirs)
	rm -f assets/data assets/sdr
	[ -d assets ] && rmdir assets || true

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: install-debug

.PHONY: install-debug
install-debug: $(apk-debug)
	adb install -r $(apk-debug)

.PHONY: install-release
install-release: $(apk-release)
	adb install -r $(apk-release)

.PHONY: uninstall
uninstall:
	adb uninstall $(pkg)

.PHONY: run
run:
	adb shell am start -n $(pkg)/$(act)

.PHONY: stop
stop:
	adb shell am force-stop $(pkg)

AndroidManifest.xml: manifest.xml.in
	android create project -p . -t $(android_platform) -k $(pkg) -a NativeActivity -n $(name)
	cat manifest.xml.in | sed 's/$$APPNAME/$(name)/g' | sed 's/$$APPTITLE/$(title)/g' >$@
	$(MAKE) icons


.PHONY: icons
icons: $(root)/datasrc/icon-512.png
	mkdir -p res/drawable-xhdpi res/drawable-hdpi res/drawable-mdpi res/drawable-ldpi
	convert $< -resize 96 res/drawable-xhdpi/ic_launcher.png
	convert $< -resize 72 res/drawable-hdpi/ic_launcher.png
	convert $< -resize 48 res/drawable-mdpi/ic_launcher.png
	convert $< -resize 36 res/drawable-ldpi/ic_launcher.png

.PHONY: update-project
update-project: build.xml
	android update project -p . -t $(android_platform)

.PHONY: logcat
logcat:
	adb logcat $(name):V AndroidRuntime:V DEBUG:V '*:S'
