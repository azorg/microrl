/*
 * MicroRL library test unut #3
 * File "mrl_test3.c"
 */

//-----------------------------------------------------------------------------
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include "mrl.h"
//-----------------------------------------------------------------------------
#define CLI_HELP
#define MAX_CMD   80 // maximum number of tokens
#define COMPL_NUM 10 // maximum completion tokens
//-----------------------------------------------------------------------------
// command/options description structure
typedef struct cmd_ cmd_t;
struct cmd_ {
  int16_t id;        // ID in tree (>=0)
  int16_t parent_id; // parent ID for options (or -1 for root)
  void (*fn)(int argc, char* const argv[], const cmd_t*); // callback function
  const char *name;  // command/option name
#ifdef CLI_HELP
  const char *args;  // arguments for help
  const char *help;  // help (description) string 
#endif // CLI_HELP
};
//-----------------------------------------------------------------------------
// MicroRL object
static mrl_t mrl;

// abstract 'vlue'
int value = 0;

// abstract FLASH
struct {
  int page;
  int size;
} flash = {63, 2048};

#ifdef MRL_USE_COMPLETE
// array for comletion
static const char *compl_world[COMPL_NUM + 1];
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
extern cmd_t const cmd_tree[];
//-----------------------------------------------------------------------------
#ifdef CLI_HELP
// find command record by ID
static const cmd_t *cmd_by_id(int id)
{
  const cmd_t *o;
  for (o = cmd_tree; o->name != NULL; o++)
    if (o->id == id)
      break;
  return o;
}
#endif // CLI_HELP
//-----------------------------------------------------------------------------
// show help based on command/option tree
void fn_help(int argc, char* const argv[], const cmd_t *cmd)
{ // help
#ifdef CLI_HELP
  if (cmd->id == 0)
  { // root (full) help
    const cmd_t *o;

    printf("MicroRL test #3\r\n"
           "Use TAB key for completion\r\n"
           "Command:\r\n");

    for (o = cmd_tree; o->name != NULL; o++)
    {
      int i, parents_cnt = 0;
      const cmd_t *parents[MAX_CMD];
      const cmd_t *p = o;

      // find all parents
      for (i = 0; i < MAX_CMD; i++)
        if (p->parent_id >= 0)
          p = parents[parents_cnt++] = cmd_by_id((int) p->parent_id);
        else
          break;

      // print all parents
      printf(" ");
      while (--parents_cnt >= 0)
        printf("%s ", parents[parents_cnt]->name);

      // print command and help
      printf("%s\033[33m%s\033[0m - \033[32m%s\033[0m\r\n",
             o->name, o->args, o->help);
    } // for (o
  }
  else
  { // command (partial) help
    const cmd_t *o;

    // print command help header
    printf("%s:\r\n", cmd->help);

    // find all parent/children
    for (o = cmd_tree; o->name != NULL; o++)
    {
      int i, flg = 0, parents_cnt = 0;
      const cmd_t *parents[MAX_CMD];
      const cmd_t *p = o;

      // find all parents
      for (i = 0; i < MAX_CMD; i++)
      {
        if (p->parent_id >= 0)
        {
          if (p->parent_id == cmd->id)
            flg = 1;
          p = parents[parents_cnt++] = cmd_by_id((int) p->parent_id);
        }
        else
          break;
      }

      if (flg)
      {
        // print all parents
        printf(" ");
        while (--parents_cnt >= 0)
          printf("%s ", parents[parents_cnt]->name);

        // print command and help
        printf("%s\033[33m%s\033[0m - \033[32m%s\033[0m\r\n",
               o->name, o->args, o->help);
      }
    } // for
  }
#endif // CLI_HELP
}
//-----------------------------------------------------------------------------
void fn_ver_mrl(int argc, char* const argv[], const cmd_t *cmd)
{
  printf("MicroRL version 2.2 (forked) argc=%i\r\n", argc);
}
//-----------------------------------------------------------------------------
void fn_ver_demo(int argc, char* const argv[], const cmd_t *cmd)
{
  printf("MicroRL demo test version 3.0 argc=%i\r\n", argc);
}
//-----------------------------------------------------------------------------
void fn_clear(int argc, char* const argv[], const cmd_t *cmd)
{
  printf("\033[2J"); // ESC seq for clear entire screen
  printf("\033[H");  // ESC seq for move cursor at left-top corner
}
//-----------------------------------------------------------------------------
static void fn_value(int argc, char* const argv[], const cmd_t *cmd)
{
  if (argc == 1)
    value = mrl_str2int(argv[0], 0, 0);
  
  if (argc > 1)
#ifdef CLI_HELP
    printf("error: bad number of arguments\r\n"
           " %s\033[33m%s\033[0m - \033[32m%s\033[0m\r\n",
           cmd->name, cmd->args, cmd->help);
#else
    printf("error: bad number of arguments\r\n");
#endif
  else
    printf("val=%u\r\n", value);
}
//-----------------------------------------------------------------------------
void fn_fl_page(int argc, char* const argv[], const cmd_t *cmd)
{
  if (argc == 1)
    flash.page = mrl_str2int(argv[0], 0, 0);
  
  if (argc > 1)
#ifdef CLI_HELP
    printf("error: bad number of arguments\r\n"
           " %s\033[33m%s\033[0m - \033[32m%s\033[0m\r\n",
           cmd->name, cmd->args, cmd->help);
#else
    printf("error: bad number of arguments\r\n");
#endif
  else
    printf("flash.pn=%u\r\n", flash.page);
}
//-----------------------------------------------------------------------------
void fn_fl_erase(int argc, char* const argv[], const cmd_t *cmd)
{
  printf("FLASH erased (argc=%i)\r\n", argc);
}
//-----------------------------------------------------------------------------
void fn_fl_erfull(int argc, char* const argv[], const cmd_t *cmd)
{
  printf("FLASH full erased\r\n");
}
//-----------------------------------------------------------------------------
#ifdef CLI_HELP
#  define _F(id, parent_id, func, name, args, help) \
   { id, parent_id, func, name, args, help }, // CLI help ON
#else
#  define _F(id, parent_id, func, name, args, help) \
   { id, parent_id, func, name }, // CLI help OFF
#endif

#define _O(id, parent_id, func, name, args, help)
//-----------------------------------------------------------------------------
// all commands and options tree
cmd_t const cmd_tree[] = {
  //  ID  Par Callback     Name       Args    Help
#ifdef CLI_HELP
  _F(  0, -1, fn_help,     "help",    "",     "print this help") // it must be first!
#endif
  _F( 10, -1, fn_help,      "version", "",     "print version")
  _F( 11, 10, fn_ver_mrl,   "microrl", "",     "print version of MicroRL")
  _F( 12, 10, fn_ver_demo,  "demo",    "",     "print version of this demo test")
  _F( 20, -1, fn_clear,     "clear",   "",     "clear screen")
  _F( 30, -1, fn_value,     "value",   " val", "get/set 'value'")
  _F( 40, -1, fn_help,      "flash",   "",     "FLASH parameters")
  _F( 41, 40, fn_fl_page,   "page",    " pn",  "get/set FLASH page number")
  _F( 42, 40, fn_fl_erase,  "erase",   "",     "erase FLASH")
  _F( 43, 42, fn_fl_erfull, "full",    "",     "erase full FLASH")
  _F( -1, -1, NULL,          NULL,     NULL,   NULL)
};
//-----------------------------------------------------------------------------
#undef _O
#undef _F
//-----------------------------------------------------------------------------
// execute callback for microrl library
// do what you want here, but don't write to argv!!! read only!!
static void execute_cb(int argc, char * const argv[])
{
  int i, parent_id = -1, arg_shift;
  const cmd_t *found = (cmd_t*) NULL;

#ifdef MRL_DEBUG
  printf("argc=%i\r\n", argc);
  for (i = 0; i < argc; i++)
    printf("argv[%i]='%s'\r\n", i, argv[i]);
  printf("argv[%i]=%p\r\n", argc, argv[argc]);
#endif

  for (i = 0; i < argc; i++)
  {
    const cmd_t *cmd = cmd_tree;
    while (cmd->name != NULL)
    {
      if ((int) cmd->parent_id == parent_id && strcmp(cmd->name, argv[i]) == 0)
      { // command/option found
        found     = cmd;
        parent_id = cmd->id;
        arg_shift = i + 1;
        break;
      }
      cmd++;
    } // while
  } // for

  if (found != (cmd_t*) NULL) // command found
    found->fn(argc - arg_shift, argv + arg_shift, found);
  else
    printf("command %s not found\r\n", argv[0]);
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_COMPLETE
// completion callback for microrl library
static const char** complete_cb(int argc, char * const argv[])
{
  int i, parent_id = -1, count = 0;

#ifdef MRL_DEBUG
  mrl_clear(&mrl);
  printf("argc=%i\r\n", argc);
  for (i = 0; i < argc; i++)
    printf("argv[%i]='%s'\r\n", i, argv[i]);
  printf("argv[%i]=%p\r\n", argc, argv[argc]);
  mrl_refresh(&mrl);
#endif

  for (i = 0; i < argc ; i++)
  {
    const cmd_t *cmd = cmd_tree;
    while (cmd->name != NULL)
    {
      if (cmd->parent_id == parent_id &&
          strstr(cmd->name, argv[i]) == cmd->name &&
          (parent_id != -1 || i == 0))
      { // substring found => add it to completion set

        if (i == argc - 1 && count < COMPL_NUM)
          compl_world[count++] = cmd->name;

        if (strcmp(cmd->name, argv[i]) == 0)
        { // command/option full found
          parent_id = cmd->id;
          break;
        }
      }
      cmd++;
    } // while
  } // for

  compl_world[count] = NULL;
  return compl_world;
}
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
// print callback for MicroRL library
static void print(const char *str)
{
  fprintf(stdout, "%s", str);
}
//-----------------------------------------------------------------------------
// get char user pressed, no waiting Enter input
static char get_char()
{
  char ch;
#ifndef MRL_ECHO_OFF
  struct termios oldt, newt;

  // canonical mode OFF
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif // !MRL_ECHO_OFF
  
  ch = getchar();
  
#ifndef MRL_ECHO_OFF
  // canonical mode ON
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt );
#endif // !MRL_ECHO_OFF

  return ch;
}
//-----------------------------------------------------------------------------
const char *prompt     = "\033[35mMicroRL> \033[0m";
const int   prompt_len = 9;
//const char *prompt     = "";
//const int   prompt_len = 0;
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  // call init with ptr to microrl instance and print callback
  mrl_init(&mrl, print);

  // set custom prompt
  mrl_set_prompt(&mrl, prompt, prompt_len);
  //mrl_set_prompt(&mrl, "", 0); // kill prompt
  
  // show prompt
  mrl_prompt(&mrl);
  //mrl_refresh(&mrl);

  // set callback for execute
  mrl_set_execute_cb(&mrl, execute_cb);

#ifdef MRL_USE_COMPLETE
  // set callback for completion
  mrl_set_complete_cb(&mrl, complete_cb);
#endif // MRL_USE_COMPLETE

  while (1)
  { // put received char from stdin to microrl lib
    int ch = get_char();
    int rv = mrl_insert_char(&mrl, ch);
    //printf("\n0x%02X\n", ch);
    if (rv)
    {
      printf("\nmrl_insert_char(0x%02X) return 0x%02X%s; exit\n",
             ch, rv, ch == 0x04 ? " (Ctrl+D pressed)" : "");
      break;
    }
  }

  return 0;
}
//-----------------------------------------------------------------------------

/*** end of "mrl_test3.c" file ***/

