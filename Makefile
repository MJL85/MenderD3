# Minimal Makefile
#
# By: Michael Laforest <paralizer -AT- users -DOT- sourceforge -DOT- net>
#
# This Makefile will invoke the Makefile in
# the src/ subdirectory.
#
# $Header$

release:
	cd src/ && $(MAKE) release

debug:
	cd src/ && $(MAKE) debug

clean:
	cd src/ && $(MAKE) clean


