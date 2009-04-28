
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

bin_PROGRAMS = jack.backup
jack_backup_SOURCES = jack.backup.c
jack_backup_LDADD = common/libcommon.a

man_MANS = jack.backup.1
EXTRA_DIST = $(man_MANS)
