SHELL := /bin/bash
RM    := rm -rf

all: ./build/Makefile
	@ $(MAKE) -C build

dev: all
	Xephyr -br :1 -screen 1000x1000 &
	sleep 1
	DISPLAY=:1 xterm &
	DISPLAY=:1 xterm -geometry 100x100+50+50 &
	DISPLAY=:1 bin/95wm

./build/Makefile:
	@ (cd build >/dev/null 2>&1 && cmake ..)

distclean:
	@- (cd build >/dev/null 2>&1 && cmake .. >/dev/null 2>&1)
	@- $(MAKE) --silent -C build clean || true
	@- $(RM) ./build/Makefile
	@- $(RM) ./build/src
	@- $(RM) ./build/test
	@- $(RM) ./build/CMake*
	@- $(RM) ./build/cmake.*
	@- $(RM) ./build/*.cmake
	@- $(RM) ./build/*.txt
	@- $(RM) ./docs/*.html
	@- $(RM) ./docs/*.css
	@- $(RM) ./docs/*.png
	@- $(RM) ./docs/*.jpg
	@- $(RM) ./docs/*.gif
	@- $(RM) ./docs/*.tiff
	@- $(RM) ./docs/*.php
	@- $(RM) ./docs/search
	@- $(RM) ./docs/installdox


ifeq ($(findstring distclean,$(MAKECMDGOALS))$(findstring dev,$(MAKECMDGOALS)),)

    $(MAKECMDGOALS): ./build/Makefile
	@ $(MAKE) -C build $(MAKECMDGOALS)

endif
