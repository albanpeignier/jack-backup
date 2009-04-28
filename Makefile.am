
MAINTAINERCLEANFILES =	aclocal.m4	\
			config.guess	\
			config.log	\
			config.status	\
			config.sub	\
			configure	\
			depcomp		\
			install-sh	\
			ltmain.sh	\
			Makefile.in	\
			missing		\
			mkinstalldirs

SUBDIRS = common

bin_PROGRAMS = jack.record
jack_record_SOURCES = jack.record.c
jack_record_LDADD = common/libcommon.a

man_MANS = jack.record.1
EXTRA_DIST = $(man_MANS)
