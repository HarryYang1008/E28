#
# READ THIS NOTE: If you followed the assignment, you have a link
# to this file, not a copy.  You cannot modify the link
#
# THEREFORE, DO THIS:
#     cp Makefile Makefile.copy
#     rm Makefile
#     mv Makefile.copy Makefile
#
#  THEN edit this file and remove the lines above this
#  line of dashes
# ----------------------------------------------------------
# This is a makefile.  A makefile contains rules that tell
# how to build a program, often from separate source files
#
# This sample makefile shows how to build dumputmp from
# the two source files dumputmp.c and utmplib.c
#
# You must modify this file so it contains rules to make
# your solution to the project
# (note: the indented lines MUST start with a single tab
#

dumputmp: dumputmp.o utmplib.o
	cc dumputmp.o utmplib.o -o dumputmp

see: dumputmp sample.wtmp
	./dumputmp sample.wtmp | more

dumputmp.o: dumputmp.c
	$(CC) -c dumputmp.c

utmplib.o: utmplib.c
	$(CC) -c utmplib.c

clean:
	rm -f *.o core dumputmp

