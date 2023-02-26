/*
 * MicroRL library test unut #2
 * File "mrl_test2.c"
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
// command/options description structure
typedef struct cmd_ cmd_t;
struct cmd_ {
  int id;            // ID in tree (>=0)
  int parent;        // parent ID for options (or -1 for root)
  const char *name;  // command/option name
  int arg_num;       // number of argiments (or 0)
  void (*fn)(int argc, char* const argv[], const cmd_t*); // callback function
  const char *help;  // help (description) string 
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
#define COMPL_NUM 10 // FIXME: magic 
static const char *compl_world[COMPL_NUM + 1];
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
extern cmd_t const cmd_tree[];
//-----------------------------------------------------------------------------
// show help based on command/option tree
#define NAME_SIZE "7" // token length for print help
static void fn_help(int argc, char* const argv[], const cmd_t *cmd)
{
  const cmd_t *opt = (const cmd_t*) cmd_tree;

  if (cmd->id == 0) // ID of "help"
    printf("MicroRL test #2\r\n"
	   "Use TAB key for completion\r\n"
	   "Command:\r\n");
    
  else
    printf("%s:\r\n", cmd->name);

  while (opt->name != NULL)
  {
    if ((opt->parent < 0        && cmd->id == 0) || // root help
	(opt->parent == cmd->id && cmd->id != 0))   // option help
    {
      char opt_info[32];
      *opt_info = '\0';

      if (opt->arg_num)
	snprintf(opt_info, sizeof(opt_info) - 1, " (arg_num=%i)",
	         opt->arg_num);

      printf("  %" NAME_SIZE "s - %s%s\r\n", opt->name, opt->help, opt_info);
    }
    opt++;
  }
}
//-----------------------------------------------------------------------------
static void fn_ver_mrl(int argc, char* const argv[], const cmd_t *cmd)
{
  printf("MicroRL version 2.1 (forked) argc=%i\r\n", argc);
}
//-----------------------------------------------------------------------------
static void fn_ver_demo(int argc, char* const argv[], const cmd_t *cmd)
{
  printf("MicroRL demo test version 2.0 (new) argc=%i\r\n", argc);
}
//-----------------------------------------------------------------------------
static void fn_clear(int argc, char* const argv[], const cmd_t *cmd)
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
    printf("error: bad number of arguments\r\n"
	   "%s: %s (arg_num=%i)\r\n", cmd->name, cmd->help, cmd->arg_num);
  else
    printf("value=%u\r\n", value);
}
//-----------------------------------------------------------------------------
static void fn_fl_page(int argc, char* const argv[], const cmd_t *cmd)
{
  if (argc == 1)
    flash.page = mrl_str2int(argv[0], 0, 0);
  
  if (argc > 1)
    printf("error: bad number of arguments\r\n"
	   "%s: %s (arg_num=%i)\r\n", cmd->name, cmd->help, cmd->arg_num);
  else
    printf("flash.page=%u\r\n", flash.page);
}
//-----------------------------------------------------------------------------
static void fn_fl_erase(int argc, char* const argv[], const cmd_t *cmd)
{
  printf("FLASH erased (argc=%i)\r\n", argc);
}
//-----------------------------------------------------------------------------
// all commands and options tree
cmd_t const cmd_tree[] = {
  // ID  Par Name     ArgN Callback      Help
  {  0, -1, "help",    0, fn_help,      "print this help" }, // it must be first!
  {  1, -1, "version", 0, fn_help,      "print version" },
  {  2,  1, "microrl", 0, fn_ver_mrl,   "print version of MicroRL" },
  {  3,  1, "demo",    0, fn_ver_demo,  "print version of this demo test" },
  {  4, -1, "clear",   0, fn_clear,     "clear screen" },
  {  5, -1, "value",   1, fn_value,     "get/set 'value'" },
  {  6, -1, "flash",   0, fn_help,      "FLASH parameters" },
  {  7,  6, "page",    1, fn_fl_page,   "get/set FLASH page number" },
  {  8,  6, "erase",   0, fn_fl_erase,  "erase FLASH" },
  { -1, -1, NULL,      0, NULL,         NULL },
};
//-----------------------------------------------------------------------------
// execute callback for microrl library
// do what you want here, but don't write to argv!!! read only!!
static void execute(int argc, char * const argv[])
{
  int i, parent = -1, arg_shift;
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
      if (cmd->parent == parent && strcmp(cmd->name, argv[i]) == 0)
      { // command/option found
	found     = cmd;
	parent    = cmd->id;
	arg_shift = i + 1;
	i        += cmd->arg_num;
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
static const char** complete(int argc, char * const argv[])
{
  int i, parent = -1, count = 0;

#ifdef MRL_DEBUG
  mrl_clear(&mrl);
  printf("argc=%i\r\n", argc);
  for (i = 0; i < argc; i++)
    printf("argv[%i]='%s'\r\n", i, argv[i]);
  printf("argv[%i]=%p\r\n", argc, argv[argc]);
  mrl_refresh(&mrl);
#endif

#if 0
  if (argc == 0)
  { // if there is no any token in cmdline, just print all available commands
    const cmd_t *cmd = cmd_tree;
    while (cmd->name != NULL)
    {
      if (count < COMPL_NUM && cmd->parent < 0)
        compl_world[count++] = cmd->name;
      cmd++;
    }
  
    compl_world[count] = NULL;
    return compl_world;
  }
#endif

  for (i = 0; i < argc ; i++)
  {
    const cmd_t *cmd = cmd_tree;
    while (cmd->name != NULL)
    {
      if (cmd->parent == parent && strstr(cmd->name, argv[i]) == cmd->name &&
	  (parent != -1 || i == 0))
      { // substring found => add it to completion set

        if (i == argc - 1 && count < COMPL_NUM)
	  compl_world[count++] = cmd->name;

        if (strcmp(cmd->name, argv[i]) == 0)
	{ // command/option full found
	  parent = cmd->id;
	  i += cmd->arg_num;
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
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  // call init with ptr to microrl instance and print callback
  mrl_init(&mrl, print);

  // set custom prompt
  mrl_set_prompt(&mrl, prompt, prompt_len);
  
  // show prompt
  mrl_prompt(&mrl);
  //mrl_refresh(&mrl);

  // set callback for execute
  mrl_set_execute_cb(&mrl, execute);

#ifdef MRL_USE_COMPLETE
  // set callback for completion
  mrl_set_complete_cb(&mrl, complete);
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

/*** end of "mrl_test2.c" file ***/

