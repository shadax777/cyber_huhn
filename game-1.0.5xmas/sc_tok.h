#ifndef __SC_TOK_H
#define __SC_TOK_H


int         Tok_LoadFile(const char *filename); // load a file and tokenize its lines
int         Tok_CurLineNum(void);
const char  *Tok_NextToken(void);   // returns NULL if no more tokens available
void        Tok_Free(void);         // free memory acquired by Tok_LoadFile()


#endif  /* !__SC_TOK_H */
