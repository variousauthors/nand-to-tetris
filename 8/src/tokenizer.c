#include "tokenizer.h"
#include "error.h"
#include "global.h"
#include "symbol.h"
#include <ctype.h>
#include <stdbool.h>

int tokenval;
int lineno = 1;
char identifierBuffer[BSIZE];

bool isIdPart(char c)
{
  return isalnum(c) || c == '-' || c == '$' || c == '.';
}

Token nextToken(Buffer *buffer)
{
  tokenval = 0;
  // lexical analysis
  char c;

  while ((c = nextchar(buffer)) != EOF)
  {
    // printf("line %d: considering %c\n", lineno, c);

    switch (c)
    {
    case EOF:
      return TK_EOF;
    case ' ':
    case '\t':
      // ignore whitespace
      commit(buffer);
      break;
    case '\r':
    {
      commit(buffer);
      break;
    }
    case '\n':
    {
      lineno++;

      commit(buffer);
      break;
    }
    case '/':
    {
      // advance past the comment
      while ((c = nextchar(buffer)) != '\n')
      {
        commit(buffer);
      }
      // put the linebreak back on there
      rollback(buffer);
      break;
    }
    case '0': {
      // 0 can only occur on it's own
      commit(buffer);

      c = nextchar(buffer);

      if (isdigit(c)) {
        error("0 cannot appear at the start of a number");
      }

      // unpeak the next char
      rollback(buffer);
      tokenval = 0;

      return TK_NUMBER;
    }
    case '1' ... '9':
    {
      // if we get a leading 0 then assume this is
      // a number and bail if we hit a non-number
      // set tokenval = val
      // get all the digits
      int val = 0;

      do
      {
        if (!isdigit(c))
        {
          // that's then end of the token
          rollback(buffer);
          break;
        }

        commit(buffer);
        val = val * 10;
        val = val + (c - '0');

      } while ((c = nextchar(buffer)) != EOF);

      tokenval = val;

      return TK_NUMBER;
    }
    case 'a' ... 'z':
    case 'A' ... 'Z':
    {
      // if we get alpha parse the whole thing as a string
      // and put it into the symbol table
      // if it is not already in the symbol table,
      // we set tokenval = -1
      int i = 0;

      do
      {
        if (!isIdPart(c))
        {
          rollback(buffer);
          break;
        }

        identifierBuffer[i++] = c;

        commit(buffer);
      } while ((c = nextchar(buffer)) != EOF);

      identifierBuffer[i] = EOS;

      int p = lookup(identifierBuffer);

      if (p == 0)
      {
        p = insert(identifierBuffer, TK_IDENTIFIER, -1);
      }

      tokenval = p;

      return symtable[p].token;
    }
    default:
      printf("unknown char: |%c| %d\n", c, c);
      error("encountered unknown char");
      break;
    }
  }

  return TK_EOF;
}
