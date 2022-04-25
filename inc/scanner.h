#ifndef __SCANNER_H__
#define __SCANNER_H__
typedef struct token_data_ {
  unsigned int col_no;
  unsigned int line_no;
  char *lexeme;
} TOKEN;

void token_to_string(int token_type, char * str);

#endif
