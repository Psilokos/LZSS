bin_PROGRAMS = lzss-encode lzss-decode

COMMON_SOURCES = src/main.cpp src/Application.cpp

lzss_encode_SOURCES = $(COMMON_SOURCES)
lzss_encode_CPPFLAGS = $(AM_CPPFLAGS) -DBINARY_NAME=\"lzss-encode\"

lzss_decode_SOURCES = $(COMMON_SOURCES)
lzss_decode_CPPFLAGS = $(AM_CPPFLAGS) -DBINARY_NAME=\"lzss-decode\"
