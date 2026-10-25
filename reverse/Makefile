# Makefile for epp512 reverse engineering
#
# Tom Trebisky -- 10/22/2026
#
# Note that "lm" stands for "load module" which is
#  the executable code with the header and relocation
#  info stripped off the front of it.

all:	epp512_lm.dis epp512.odx
all2:	access_lm.dis access.odx

fetch: epp512.exe access.exe

epp512.exe: ../../EMUP/epp512.exe
	cp ../EMUP/epp512.exe .

access.exe: ../../EMUP/access.exe
	cp ../EMUP/access.exe .

# Skip the header and reloc table.
# This extracts just the executable code
# the skip value must be gotten by inspecting
# the header
epp512.lm: epp512.exe
	dd if=epp512.exe of=epp512.lm skip=15

access.lm: access.exe
	dd if=access.exe of=access.lm skip=2

# Hex dump of just the code
epp512.odx: epp512.lm
	odx epp512.lm >epp512.odx

access.odx: access.lm
	odx access.lm >access.odx

# Here "-a" activates auto (intelligent) sync
# The "-s 0x26B4" specifies a sync value
# The 0x26B4 value is the initial IP (PC) from the header
epp512_lm.raw:	epp512.lm
#	ndisasm -a -s 0x26B4 epp512.lm >epp512_lm.raw
	ndisasm -a -s 0x794 -s 0x26B4 epp512.lm >epp512_lm.raw

access_lm.raw:	access.lm
	ndisasm -a -s 0x760e access.lm >access_lm.raw

epp512_lm.dis:	epp512_lm.raw
	./fixadd >epp512_lm.dis

access_lm.dis:	access_lm.raw
	./fixadd >access_lm.dis

clean:
	rm -f epp512_lm.raw
	rm -f epp512_lm.dis
