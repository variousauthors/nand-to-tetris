#include "tokenizer.h"
#include "error.h"
#include "symbol.h"
#include <ctype.h>
#include <stdbool.h>

int tokenval;
int lineno = 1;
char identifierBuffer[MAX_STRING];

bool isIdPart(char c)
{
  return isalnum(c) || c == '_';
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
      commit(buffer);

      c = nextchar(buffer);

      if (c != '/' && c != '*')
      {
        // this is not a comment
        rollback(buffer);
        return TK_SLASH;
      }

      if (c == '*')
      {
        // multi-line comment
        while ((c = nextchar(buffer)) != EOF)
        {
          // if we hit * then / we are done
          if (c == '*') {
            c = nextchar(buffer);

            if (c == '/') {
              // we are done
              commit(buffer);
              break;
            }
          }

          if (c == '\n') {
            lineno++;
          }

          commit(buffer);
        }
        // put the linebreak back on there
        rollback(buffer);
        break;
      }
      else
      {
        // single line comment
        while ((c = nextchar(buffer)) != '\n')
        {
          commit(buffer);
        }
        // put the linebreak back on there
        lineno++;
        rollback(buffer);
        break;
      }
    }
    case '0':
    {
      // 0 can only occur on it's own
      commit(buffer);

      c = nextchar(buffer);

      if (isdigit(c))
      {
        error("0 cannot appear at the start of a number");
      }

      // unpeak the next char
      rollback(buffer);
      tokenval = 0;

      return TK_INTEGER;
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

      return TK_INTEGER;
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
    case '"':
    {
      int i = 0;

      while ((c = nextchar(buffer)) != '"')
      {
        if (c == '\n')
        {
          error("encountered newline in string");
        }

        identifierBuffer[i++] = c;

        commit(buffer);
      }

      // consume the closing "
      commit(buffer);

      identifierBuffer[i] = EOS;

      return TK_STRING;
    }

    // symbols
    case '{':
      return TK_BRACE_L;
    case '}':
      return TK_BRACE_R;
    case '(':
      return TK_PAREN_L;
    case ')':
      return TK_PAREN_R;
    case '[':
      return TK_BRACKET_L;
    case ']':
      return TK_BRACKET_R;
    case '.':
      return TK_DOT;
    case ',':
      return TK_COMMA;
    case ';':
      return TK_SEMI;
    case '+':
      return TK_PLUS;
    case '-':
      return TK_MINUS;
    case '*':
      return TK_ASTERISK;
    case '&':
      return TK_AMP;
    case '|':
      return TK_BAR;
    case '<':
      return TK_ANGLE_BRACKET_L;
    case '>':
      return TK_ANGLE_BRACKET_R;
    case '=':
      return TK_EQUAL;
    case '~':
      return TK_TILDE;

    default:
      fprintf(stderr, "unknown char: |%c| %d\n", c, c);
      error("encountered unknown char");
      break;
    }
  }

  return TK_EOF;
}
