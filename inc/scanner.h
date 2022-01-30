#ifndef __SCANNER_H__
#define __SCANNER_H__

typedef struct token {
  int col_no;
  int line_no;
  char *lexeme;
} TOKEN;

void token_to_string(int token_type, char *token_name);
#endif