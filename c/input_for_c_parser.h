// file extracted from json_to_c_stub.c which is somehow contract to honor for c_parser

// INPUT

struct test_1 {
  int a;
  int b;
  int c;
  char * d;
};

struct test_2 {
  int a;
  struct test_1 c;
  struct test_1 * e;
  struct test_1 * f;
};

