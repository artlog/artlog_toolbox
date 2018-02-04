#ifndef __ALINPUTSTREAM_HEADER__
#define __ALINPUTSTREAM_HEADER__

struct alinputstream {
  int fd;
  int eof;
  int debug;
};

void alinputstream_init(struct alinputstream * stream, int fd);

unsigned int alinputstream_readuint32(struct alinputstream * stream);

unsigned char alinputstream_readuchar(struct alinputstream * stream);

#endif
