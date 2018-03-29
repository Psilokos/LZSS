codecdir = $(pkglibdir)/codec

codec_LTLIBRARIES = libcodec.la

libcodec_la_SOURCES = \
    Codec.cpp \
    Decoder.cpp \
    Encoder.cpp \
    SlidingWindow.cpp

libcodec_la_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/codec/lzss

nobase_include_HEADERS = \
    lzss_codec.hpp \
    lzss/Codec.hpp \
    lzss/Decoder.hpp \
    lzss/Encoder.hpp \
    lzss/SlidingWindow.hpp
