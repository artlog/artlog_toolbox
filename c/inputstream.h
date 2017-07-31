#ifndef __INPUTSTREAM_HEADER__
#define __INPUTSTREAM_HEADER__

struct inputstream {
  int fd;
  int eof;
  int debug;
};

void inputstream_init(struct inputstream * stream, int fd);

unsigned int inputstream_readuint32(struct inputstream * stream);

unsigned char inputstream_readuchar(struct inputstream * stream);

#endif
