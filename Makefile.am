ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = lzss-encode lzss-decode

COMMON_SOURCES = src/main.cpp src/Application.cpp
COMMON_LDADD = $(top_builddir)/codec/libcodec.la

lzss_encode_SOURCES = $(COMMON_SOURCES)
lzss_encode_CPPFLAGS = $(AM_CPPFLAGS) -isystem $(top_srcdir)/codec -DBINARY_NAME=\"lzss-encode\"
lzss_encode_LDADD = $(COMMON_LDADD)

lzss_decode_SOURCES = $(COMMON_SOURCES)
lzss_decode_CPPFLAGS = $(AM_CPPFLAGS) -isystem $(top_srcdir)/codec -DBINARY_NAME=\"lzss-decode\"
lzss_decode_LDADD = $(COMMON_LDADD)

SUBDIRS = codec
