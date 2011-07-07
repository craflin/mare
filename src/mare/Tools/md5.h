
#pragma once

class MD5
{
public:
  MD5();
  void update(const unsigned char *buf, unsigned len);
  void final(unsigned char digest[16]);

private:
  unsigned int buf[4];
  unsigned int bits[2];
  unsigned char in[64];

  void transform(unsigned int buf[4], unsigned int in[16]);
};
