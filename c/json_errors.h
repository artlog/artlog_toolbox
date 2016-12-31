#ifndef __JSON_ERRORS_HEADER__
#define __JSON_ERRORS_HEADER__

enum json_syntax_error {
  JSON_ERROR_DICT_CLOSE_NON_DICT = 100,
  JSON_ERROR_DICT_CLOSE_INVALID_PARENT = 101,
  JSON_ERROR_DICT_KEY_WITHOUT_VALUE = 102,
  JSON_ERROR_DICT_VALUE_WITHOUT_KEY = 103,
  JSON_ERROR_DICT_KEY_NON_QUOTED = 130,
  JSON_ERROR_LIST_CLOSE_NON_LIST = 200,
  JSON_ERROR_LIST_CLOSE_INVALID_PARENT = 201,
  JSON_ERROR_LIST_CLOSE_NO_PARENT = 202,
  JSON_ERROR_STRING_IN_STRING = 310,
  JSON_ERROR_LIST_ELEMENT_INVALID_PARENT = 401,
  JSON_ERROR_VARIABLE_BOUNDARY = 520,
  JSON_ERROR_VARIABLE_NAME_NULL = 521,
};

#endif
