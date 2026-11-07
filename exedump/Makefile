# Tom Trebisky  10/25/2026

exedump: exedump.c
	cc -o exedump exedump.c

fetch: epp512.exe

# Fetch our input file
epp512.exe: ../../EMUP/epp512.exe
	cp ../../EMUP/epp512.exe .

data:
	./exedump -d >data

install: exedump
	cp exedump /home/tom/bin

# THE END
