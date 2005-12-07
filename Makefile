CC= gcc
CFLAGS= -Wall -O2 -g
LDFLAGS=
MODULES= main.o decrunch.o ppdepack.o mmcmp.o unsqsh.o

amigadepacker:	$(MODULES)
	$(CC) -o $@ $(MODULES)

main.o:	main.c decrunch.h
	$(CC) $(CFLAGS) -c $<

decrunch.o:	decrunch.c decrunch.h ppdepack.h
	$(CC) $(CFLAGS) -c $<

ppdepack.o:	ppdepack.c ppdepack.h
	$(CC) $(CFLAGS) -c $<

mmcmp.o:	mmcmp.c mmcmp.h
	$(CC) $(CFLAGS) -c $<

unsqsh.o:	unsqsh.c unsqsh.h
	$(CC) $(CFLAGS) -c $<

clean:	
	rm -f *.o