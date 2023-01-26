// sc_tok.c - read all tokens of a file
//          - tokens can be: strings, multiple quoted strings, '{', '=' and
//            strings starting with '$'


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include "sc_tok.h"


// a tokenized line
typedef struct
{
  char  **tokens;
  int   ntokens;
} tokline_t;


static tokline_t    *s_toklines;    // all tokenized lines of the file
static int          s_numlines;     // total no. of lines
static int          s_curlinenum;   // currently being parsed line no.
static int          s_curtoknum;    // current token no. in current line


static char *Compress(char *s); // remove comments and spaces
static char *ReadLine(FILE *f); // return the next line or NULL from file
static char **ReadAllLines(FILE *f, int *nlines);

static void TokenizeLine(tokline_t *tl, const char *line);


// log malloc()- and free()-calls for diagnostical reason
static int  s_nmalloc = 0, s_nfree = 0;
static void *_shuttle_;
#define MALLOC(n)       (s_nmalloc+=((_shuttle_=malloc(n))!=NULL), _shuttle_)
#define REALLOC(p, n)   (s_nmalloc+=((_shuttle_=(p))==NULL), realloc(_shuttle_, n))
#define FREE(p)         (s_nfree+=((_shuttle_=(p))!=NULL), free(_shuttle_))


//--------------------------------------------------------
// Tok_LoadFile - attempt to load a file, and tokenize each line
//              - returns 1/0 depending on success or failure
//--------------------------------------------------------
int Tok_LoadFile(const char *filename)
{
  FILE  *f;
  char  **lines;    // lines of the file
  int   i;

  s_numlines = s_curlinenum = s_curtoknum = 0;

  // read all lines of the file
  if(!(f = fopen(filename, "r")))
  {
    fprintf(stderr, "Tok_LoadFile: could not open file '%s': %s\n", filename, strerror(errno));
    return 0;
  }
  lines = ReadAllLines(f, &s_numlines);
  fclose(f);

  // tokenize each line
  if(!(s_toklines = (tokline_t *)MALLOC(s_numlines * sizeof(tokline_t))))
  {
    fprintf(stderr, "Tok_LoadFile: out of mem (%u)\n", s_numlines * sizeof(tokline_t));
    return 0;
  }
  for(i = 0; i < s_numlines; i++)
  {
    TokenizeLine(&s_toklines[i], lines[i]);
  }

  // free the file lines (TokenizeLines() made its own copies)
  for(i = 0; i < s_numlines; i++)
    FREE(lines[i]);
  FREE(lines);

// DEBUG: print all tokens
#if 0
  for(i = 0; i < s_numlines; i++)
  {
    int k;

    printf("#%i (%i): ", i+1, s_toklines[i].ntokens);
    for(k = 0; k < s_toklines[i].ntokens; k++)
      printf("'%s' ", s_toklines[i].tokens[k]);
    printf("\n");
  }

  {
    const char  *tok;

    while((tok = Tok_NextToken()))
      printf("%s\n", tok);
  }
#endif
  return 1;
} // Tok_LoadFile


//--------------------------
// Tok_CurLineNum
//--------------------------
int Tok_CurLineNum(void)
{
  return s_curlinenum + 1;  // +1 to make it human-readable
} // Tok_CurLineNum


//----------------------------------------------
// Tok_NextToken - returns the next token or NULL if no more available
//----------------------------------------------
const char *Tok_NextToken(void)
{
  const char    *ret;

  if(s_curlinenum >= s_numlines)
    return NULL;

  // if necessary, jump into next line
  if(s_curtoknum >= s_toklines[s_curlinenum].ntokens)
  {
    s_curlinenum++;
    s_curtoknum = 0;
    ret = Tok_NextToken();
  }
  else
  {
    ret = s_toklines[s_curlinenum].tokens[s_curtoknum++];
  }
  return ret;
} // Tok_NextToken


//----------------------------------------
// Tok_Free - free memory acquired by Tok_LoadFile()
//----------------------------------------
void Tok_Free(void)
{
  while(s_numlines > 0)
  {
    int i;

    s_numlines--;
    for(i = 0; i < s_toklines[s_numlines].ntokens; i++)
      FREE(s_toklines[s_numlines].tokens[i]);
    FREE(s_toklines[s_numlines].tokens);
  }
  FREE(s_toklines);
  s_toklines = NULL;
} // Tok_Free


//---------------------------------------------------------------------------


//-----------------------------------------
// Compress - removes c++-style comments and strips leading and trailing spaces
//          - returns the compressed input string
//-----------------------------------------
static char *Compress(char *s)
{
  char  *c, *start, *end;

  if(!s)
    return NULL;

  // skip leading spaces
  for(c = s; *c && isspace(*c); c++)
    ;
  start = c;

  // search for and strip c++-style comment
  if((c = strstr(s, "//")) != NULL)
    *c = '\0';

  // strip trailing spaces
  end = &s[strlen(s) - 1];
  while(end >= s && isspace(*end))
    *end-- = '\0';

  return strcpy(s, start);
} // Compress


//---------------------------------------------
// ReadLine - reads a line from given stream
//          - returns a pointer to the the line or NULL if EOF is encountered
//          - don't forget to free() the returned pointer!
//---------------------------------------------
static char *ReadLine(FILE *f)
{
  size_t    maxlen = 8; // max. buffer length; grows as needed
  char      *line = NULL, *linep;
  int       c;

  assert(f);

  // test if standing on EOF
  if((c = fgetc(f)) == EOF)
    return NULL;
  ungetc(c, f);

  // allocate initial memory
  if(!(line = (char *)MALLOC(maxlen + 1)))
  {
    fprintf(stderr, "*** ReadLine: out of mem\n");
    exit(EXIT_FAILURE);
  }
  memset(line, 0, maxlen);
  linep = line;

  while((c = fgetc(f)) != EOF)
  {
    // enlarge buffer if needed
    if(linep - line == maxlen)
    {
      char  *newline;

      if(!(newline = (char *)REALLOC(line, maxlen*2 + 1)))
      {
        fprintf(stderr, "*** ReadLine: out of mem\n");
        free(line);
        exit(EXIT_FAILURE);
      }
      line = newline;
      linep = &line[maxlen];
      maxlen *= 2;
    }

    if(c == '\n')
      break;
    else
      *linep++ = (char)c;
  }
  *linep = '\0';
  return line;
} // ReadLine


//----------------------------------------------------
// ReadAllLines - read all lines of the given stream
//              - don't forget to free() the lines
//----------------------------------------------------
static char **ReadAllLines(FILE *f, int *nlines)
{
  char  **lines = NULL;
  char  *curline;

  *nlines = 0;
  while((curline = Compress(ReadLine(f))))
  {
    char    **newlines;

    if(!(newlines = (char **)REALLOC(lines, ((*nlines) + 1) * sizeof(char *))))
    {
      fprintf(stderr, "*** ReadAllLines: out of mem\n");
      FREE(lines);
      exit(EXIT_FAILURE);
    }
    lines = newlines;
    lines[(*nlines)++] = curline;
  }
  return lines;
} // ReadAllLines


//----------------------------------
// JumpBehindToken
//
// used by TokenizeLine()
// reminder: quotes will sum up strings
// note: assumes that leading space have been removed
//----------------------------------
/*
sample strings:

1)
  `foo   bar   baz'
   ^  ^
input output


2)
  `$name = "bugs bunny"'
           ^           ^
       input          output
*/
static const char *JumpBehindToken(const char *s)
{
  int   isquoted;

  if(*s == '{' || *s == '}' || *s == '=')
    return s + 1;

  isquoted = (*s == '"');
  while(*s)
  {
    s++;
    if(!isquoted)
    {
      if(isspace(*s) || *s == '{' || *s == '}' || *s == '=' || *s == '$')
        break;
    }
    // check for end of quote (or beginning of new quoted string)
    if(*s == '"')
      return s + (isquoted != 0);
  }
  return s;
} // JumpBehindToken


//--------------------------------------
// strndup - attempt to duplicate max. 'n' chars of given string
//--------------------------------------
static char *strndup(const char *s, size_t n)
{
  char  *buf, *bp;

  if((buf = (char *)MALLOC(n + 1)))
  {
    bp = buf;
    while(n-- > 0 && *s)
      *bp++ = *s++;
    *bp = '\0';
  }
  return buf;
} // strndup


//-----------------------------------------------
// TokenizeLine - build tokens from given line
//              - NOTE: the given line is assumed to be Compress()'ed
//-----------------------------------------------
static void TokenizeLine(tokline_t *tl, const char *line)
{
  const char    *start, *end;   // current start/end of a token

  tl->ntokens = 0;
  tl->tokens = NULL;
  start = line;
  while(*start)
  {
    char    **newtokens;

    // attempt to enlarge token array
    if(!(newtokens = (char **)REALLOC(tl->tokens, (tl->ntokens+1)*sizeof(char *))))
    {
      // free previously allocated token-memory
      while(tl->ntokens > 0)
        FREE(tl->tokens[--tl->ntokens]);
      FREE(tl->tokens);
      fprintf(stderr, "TokenizeLine: out of mem\n");
      exit(EXIT_FAILURE);
    }
    tl->tokens = newtokens;

    end = JumpBehindToken(start);

    // attempt to copy string into token buffer
    if(!(tl->tokens[tl->ntokens] = strndup(start, end-start)))
    {
      // free previously allocated token-memory
      while(tl->ntokens > 0)
        FREE(tl->tokens[--tl->ntokens]);
      FREE(tl->tokens);
      fprintf(stderr, "TokenizeLine: out of mem\n");
      exit(EXIT_FAILURE);
    }
    tl->ntokens++;

    start = end;
    // skip spaces till next token
    while(isspace(*start))
      start++;
  }
} // TokenizeLine


#if 0
//====================================================================
//
// test environment
//
//====================================================================
int main(int argc, char *argv[])
{
  if(argc != 2)
  {
    fprintf(stderr, "need a script file\n");
    return 1;
  }
  Tok_LoadFile(argv[1]);

  // free the whole schmodder
  Tok_Free();

  printf("# malloc: %i\n# free: %i\n", s_nmalloc, s_nfree);
  return 0;
}
#endif
