// compiler front-end

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "script.h"


typedef struct
{
  const char    *string;
  int           id;
} option_t;


static int FindOptionID(const option_t *opttab, const char *string)
{
  const option_t    *opt;

  for(opt = opttab; opt->string; opt++)
  {
    if(!strcmp(opt->string, string))
      return opt->id;
  }
  return -1;
}


// option id's
enum
{
  OPT_HELP,
  OPT_VERBOSE,
  OPT_OUTDIR,
};


static const char   *s_optionshelp[] =
{
  "-h, -help      - this text",
  "-verbose       - print parsed entities + level info",
  "-outdir <dir>  - specify output directory",
  NULL
};


static const option_t   s_options[] =
{
  { "-h",           OPT_HELP },
  { "-help",        OPT_HELP },
  { "-verbose",     OPT_VERBOSE },
  { "-outdir",      OPT_OUTDIR },
  { NULL }  // sentinel
};


static void Help(const char *progname)
{
  int   i;

  fprintf(stderr, "SCC - chicken shooter script tool - (C) 2002 Shadax, bla bla bla...\n");
  fprintf(stderr, "Script version: #%u\n", SCRIPTVERSION);
  fprintf(stderr, "Date:           %s %s\n\n", __DATE__, __TIME__);
  fprintf(stderr, "Usage: %s <script file> [options]\nOptions:\n", progname);
  for(i = 0; s_optionshelp[i]; i++)
  {
    fprintf(stderr, "  %s\n", s_optionshelp[i]);
  }
}


int main(int argc, char *argv[])
{
  int           verbose = 0;
  const char    *outdir = NULL;
  int   i;

  if(argc < 2)
  {
    Help(argv[0]);
    return 1;
  }

  // parse possible options
  for(i = 2; i < argc; i++)
  {
    int optid;

    if((optid = FindOptionID(s_options, argv[i])) != -1)
    {
      switch(optid)
      {
        case OPT_HELP:
          Help(argv[0]);
          return 0;
          break;

        case OPT_VERBOSE:
          verbose = 1;
          break;

        case OPT_OUTDIR:
          if(++i < argc)
          {
            outdir = argv[i];
          }
          else
          {
            fprintf(stderr, "%s requires a directory name\n", argv[i-1]);
            return 2;
          }
          break;

        default:    // cannot happen
          assert(0);
      }
    }
    else
    {
      fprintf(stderr, "unknown option: %s\n", argv[i]);
      return 2;
    }
  }

  SC_CompileFile(argv[1], outdir, verbose);

  return 0;
}
