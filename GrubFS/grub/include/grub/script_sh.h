/* normal_parser.h  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2007,2009,2010  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_NORMAL_PARSER_HEADER
#define GRUB_NORMAL_PARSER_HEADER	1

#include <grub/types.h>
#include <grub/err.h>
#include <grub/parser.h>
#include <grub/command.h>

struct grub_script_mem;

/* The generic header for each scripting command or structure.  */
struct grub_script_cmd
{
  /* This function is called to execute the command.  */
  grub_err_t (*exec) (struct grub_script_cmd *cmd);

  /* The next command.  This can be used by the parent to form a chain
     of commands.  */
  struct grub_script_cmd *next;
};

struct grub_script
{
  unsigned refcnt;
  struct grub_script_mem *mem;
  struct grub_script_cmd *cmd;

  /* grub_scripts from block arguments.  */
  struct grub_script *next_siblings;
  struct grub_script *children;
};

typedef enum
{
  GRUB_SCRIPT_ARG_TYPE_VAR,
  GRUB_SCRIPT_ARG_TYPE_TEXT,
  GRUB_SCRIPT_ARG_TYPE_GETTEXT,
  GRUB_SCRIPT_ARG_TYPE_DQVAR,
  GRUB_SCRIPT_ARG_TYPE_DQSTR,
  GRUB_SCRIPT_ARG_TYPE_SQSTR,
  GRUB_SCRIPT_ARG_TYPE_BLOCK
} grub_script_arg_type_t;

/* A part of an argument.  */
struct grub_script_arg
{
  grub_script_arg_type_t type;

  char *str;

  /* Parsed block argument.  */
  struct grub_script *script;

  /* Next argument part.  */
  struct grub_script_arg *next;
};

/* An argument vector.  */
struct grub_script_argv
{
  unsigned argc;
  char **args;
  struct grub_script *script;
};

/* Pluggable wildcard translator.  */
struct grub_script_wildcard_translator
{
  grub_err_t (*expand) (const char *str, char ***expansions);
};
extern struct grub_script_wildcard_translator *grub_wildcard_translator;
extern struct grub_script_wildcard_translator grub_filename_translator;

/* A complete argument.  It consists of a list of one or more `struct
   grub_script_arg's.  */
struct grub_script_arglist
{
  struct grub_script_arglist *next;
  struct grub_script_arg *arg;
  /* Only stored in the first link.  */
  int argcount;
};

/* A single command line.  */
struct grub_script_cmdline
{
  struct grub_script_cmd cmd;

  /* The arguments for this command.  */
  struct grub_script_arglist *arglist;
};

/* An if statement.  */
struct grub_script_cmdif
{
  struct grub_script_cmd cmd;

  /* The command used to check if the 'if' is true or false.  */
  struct grub_script_cmd *exec_to_evaluate;

  /* The code executed in case the result of 'if' was true.  */
  struct grub_script_cmd *exec_on_true;

  /* The code executed in case the result of 'if' was false.  */
  struct grub_script_cmd *exec_on_false;
};

/* A for statement.  */
struct grub_script_cmdfor
{
  struct grub_script_cmd cmd;

  /* The name used as looping variable.  */
  struct grub_script_arg *name;

  /* The words loop iterates over.  */
  struct grub_script_arglist *words;

  /* The command list executed in each loop.  */
  struct grub_script_cmd *list;
};

/* A while/until command.  */
struct grub_script_cmdwhile
{
  struct grub_script_cmd cmd;

  /* The command list used as condition.  */
  struct grub_script_cmd *cond;

  /* The command list executed in each loop.  */
  struct grub_script_cmd *list;

  /* The flag to indicate this as "until" loop.  */
  int until;
};

/* State of the lexer as passed to the lexer.  */
struct grub_lexer_param
{
  /* Function used by the lexer to get a new line when more input is
     expected, but not available.  */
  grub_reader_getline_t getline;

  /* Caller-supplied data passed to `getline'.  */
  void *getline_data;

  /* A reference counter.  If this is >0 it means that the parser
     expects more tokens and `getline' should be called to fetch more.
     Otherwise the lexer can stop processing if the current buffer is
     depleted.  */
  int refs;

  /* While walking through the databuffer, `record' the characters to
     this other buffer.  It can be used to edit the menu entry at a
     later moment.  */

  /* If true, recording is enabled.  */
  int record;

  /* Points to the recording.  */
  char *recording;

  /* index in the RECORDING.  */
  int recordpos;

  /* Size of RECORDING.  */
  int recordlen;

  /* End of file reached.  */
  int eof;

  /* Merge multiple word tokens.  */
  int merge_start;
  int merge_end;

  /* Part of a multi-part token.  */
  char *text;
  unsigned used;
  unsigned size;

  /* Type of text.  */
  grub_script_arg_type_t type;

  /* Flag to indicate resplit in progres.  */
  unsigned resplit;

  /* Text that is unput.  */
  char *prefix;

  /* Flex scanner.  */
  void *yyscanner;

  /* Flex scanner buffer.  */
  void *buffer;
};

#define GRUB_LEXER_INITIAL_TEXT_SIZE   32
#define GRUB_LEXER_INITIAL_RECORD_SIZE 256

/* State of the parser as passes to the parser.  */
struct grub_parser_param
{
  /* Keep track of the memory allocated for this specific
     function.  */
  struct grub_script_mem *func_mem;

  /* When set to 0, no errors have occurred during parsing.  */
  int err;

  /* The memory that was used while parsing and scanning.  */
  struct grub_script_mem *memused;

  /* The block argument scripts.  */
  struct grub_script *scripts;

  /* The result of the parser.  */
  struct grub_script_cmd *parsed;

  struct grub_lexer_param *lexerstate;
};

void grub_script_init (void);
void grub_script_fini (void);

void grub_script_mem_free (struct grub_script_mem *mem);

void grub_script_argv_free    (struct grub_script_argv *argv);
int grub_script_argv_make     (struct grub_script_argv *argv, int argc, char **args);
int grub_script_argv_next     (struct grub_script_argv *argv);
int grub_script_argv_append   (struct grub_script_argv *argv, const char *s,
			       grub_size_t slen);
int grub_script_argv_split_append (struct grub_script_argv *argv, const char *s);

struct grub_script_arglist *
grub_script_create_arglist (struct grub_parser_param *state);

struct grub_script_arglist *
grub_script_add_arglist (struct grub_parser_param *state,
			 struct grub_script_arglist *list,
			 struct grub_script_arg *arg);
struct grub_script_cmd *
grub_script_create_cmdline (struct grub_parser_param *state,
			    struct grub_script_arglist *arglist);

struct grub_script_cmd *
grub_script_create_cmdif (struct grub_parser_param *state,
			  struct grub_script_cmd *exec_to_evaluate,
			  struct grub_script_cmd *exec_on_true,
			  struct grub_script_cmd *exec_on_false);

struct grub_script_cmd *
grub_script_create_cmdfor (struct grub_parser_param *state,
			   struct grub_script_arg *name,
			   struct grub_script_arglist *words,
			   struct grub_script_cmd *list);

struct grub_script_cmd *
grub_script_create_cmdwhile (struct grub_parser_param *state,
			     struct grub_script_cmd *cond,
			     struct grub_script_cmd *list,
			     int is_an_until_loop);

struct grub_script_cmd *
grub_script_append_cmd (struct grub_parser_param *state,
			struct grub_script_cmd *list,
			struct grub_script_cmd *last);
struct grub_script_arg *
grub_script_arg_add (struct grub_parser_param *state,
		     struct grub_script_arg *arg,
		     grub_script_arg_type_t type, char *str);

struct grub_script *grub_script_parse (char *script,
				       grub_reader_getline_t getline_func,
				       void *getline_func_data);
void grub_script_free (struct grub_script *script);
struct grub_script *grub_script_create (struct grub_script_cmd *cmd,
					struct grub_script_mem *mem);

struct grub_lexer_param *grub_script_lexer_init (struct grub_parser_param *parser,
						 char *script,
						 grub_reader_getline_t getline_func,
						 void *getline_func_data);
void grub_script_lexer_fini (struct grub_lexer_param *);
void grub_script_lexer_ref (struct grub_lexer_param *);
void grub_script_lexer_deref (struct grub_lexer_param *);
unsigned grub_script_lexer_record_start (struct grub_parser_param *);
char *grub_script_lexer_record_stop (struct grub_parser_param *, unsigned);
int  grub_script_lexer_yywrap (struct grub_parser_param *, const char *input);
void grub_script_lexer_record (struct grub_parser_param *, char *);

/* Functions to track allocated memory.  */
struct grub_script_mem *grub_script_mem_record (struct grub_parser_param *state);
struct grub_script_mem *grub_script_mem_record_stop (struct grub_parser_param *state,
						     struct grub_script_mem *restore);
void *grub_script_malloc (struct grub_parser_param *state, grub_size_t size);

/* Functions used by bison.  */
union YYSTYPE;
int grub_script_yylex (union YYSTYPE *, struct grub_parser_param *);
int grub_script_yyparse (struct grub_parser_param *);
void grub_script_yyerror (struct grub_parser_param *, char const *);

/* Commands to execute, don't use these directly.  */
grub_err_t grub_script_execute_cmdline (struct grub_script_cmd *cmd);
grub_err_t grub_script_execute_cmdlist (struct grub_script_cmd *cmd);
grub_err_t grub_script_execute_cmdif (struct grub_script_cmd *cmd);
grub_err_t grub_script_execute_cmdfor (struct grub_script_cmd *cmd);
grub_err_t grub_script_execute_cmdwhile (struct grub_script_cmd *cmd);

/* Execute any GRUB pre-parsed command or script.  */
grub_err_t grub_script_execute (struct grub_script *script);
grub_err_t grub_script_execute_sourcecode (const char *source);
grub_err_t grub_script_execute_new_scope (const char *source, int argc, char **args);

/* Break command for loops.  */
grub_err_t grub_script_break (grub_command_t cmd, int argc, char *argv[]);

/* SHIFT command for GRUB script.  */
grub_err_t grub_script_shift (grub_command_t cmd, int argc, char *argv[]);

/* SETPARAMS command for GRUB script functions.  */
grub_err_t grub_script_setparams (grub_command_t cmd, int argc, char *argv[]);

/* RETURN command for functions.  */
grub_err_t grub_script_return (grub_command_t cmd, int argc, char *argv[]);

/* This variable points to the parsed command.  This is used to
   communicate with the bison code.  */
extern struct grub_script_cmd *grub_script_parsed;



/* The function description.  */
struct grub_script_function
{
  /* The name.  */
  char *name;

  /* The script function.  */
  struct grub_script *func;

  /* The flags.  */
  unsigned flags;

  /* The next element.  */
  struct grub_script_function *next;

  int references;
};
typedef struct grub_script_function *grub_script_function_t;

extern grub_script_function_t grub_script_function_list;

#define FOR_SCRIPT_FUNCTIONS(var) for((var) = grub_script_function_list; \
				      (var); (var) = (var)->next)

grub_script_function_t grub_script_function_create (struct grub_script_arg *functionname,
						    struct grub_script *cmd);
void grub_script_function_remove (const char *name);
grub_script_function_t grub_script_function_find (char *functionname);

grub_err_t grub_script_function_call (grub_script_function_t func,
				      int argc, char **args);

char **
grub_script_execute_arglist_to_argv (struct grub_script_arglist *arglist, int *count);

grub_err_t
grub_normal_parse_line (char *line,
			grub_reader_getline_t getline_func,
			void *getline_func_data);

static inline struct grub_script *
grub_script_ref (struct grub_script *script)
{
  if (script)
    script->refcnt++;
  return script;
}

static inline void
grub_script_unref (struct grub_script *script)
{
  if (! script)
    return;

  if (script->refcnt == 0)
    grub_script_free (script);
  else
    script->refcnt--;
}

#endif /* ! GRUB_NORMAL_PARSER_HEADER */
