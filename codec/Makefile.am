codecdir = $(top_srcdir)/codec

lib_LTLIBRARIES = libcodec.la

libcodec_la_SOURCES = \
    Codec.cpp \
    SlidingWindow.cpp
libcodec_la_CPPFLAGS = -I $(codecdir)/lzss

nobase_include_HEADERS = \
    lzss_codec.hpp \
    lzss/Codec.hpp \
    lzss/SlidingWindow.hpp
