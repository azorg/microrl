/*
 * MicroRL library test unut
 * File "mrl_test.c"
 */

//-----------------------------------------------------------------------------
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h> 
#include <stdio.h>
#include "mrl.h"
//-----------------------------------------------------------------------------
// definition commands word
#define _CMD_HELP  "help"
#define _CMD_CLEAR "clear"
#define _CMD_LIST  "list"
#define _CMD_LISP  "lisp" // for demonstration completion on 'l + <TAB>'
#define _CMD_NAME  "name"
#define _CMD_VALUE "value"
#define _CMD_VER   "version"

// sub commands for version command
#define _SCMD_MRL  "microrl"
#define _SCMD_DEMO "demo"

#define _NUM_OF_CMD 6
#define _NUM_OF_VER_SCMD 2

// available  commands
char *keyworld[] = { _CMD_HELP, _CMD_CLEAR, _CMD_LIST, _CMD_NAME, _CMD_VALUE,
                     _CMD_VER, _CMD_LISP };

// version subcommands
char *ver_keyworld[] = { _SCMD_MRL, _SCMD_DEMO };

// array for comletion
char *compl_world[_NUM_OF_CMD + 1];

// 'name' var for store some string
#define _NAME_LEN 8
char name[_NAME_LEN];

// 'value' var for store long value
long value;
//-----------------------------------------------------------------------------
// get char user pressed, no waiting Enter input
static char get_char()
{
  char c;
  struct termios oldt, newt;

  // canonical mode OFF
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  
  c = getchar();
  
  // canonical mode ON
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt );

  return c;
}
//-----------------------------------------------------------------------------
// print callback for MicroRL library
static void print(const char *str)
{
  fprintf(stdout, "%s", str);
}
//-----------------------------------------------------------------------------
static void print_help()
{
  print("Use TAB key for completion\n\r"
        "Command:\n\r");
  print("\tversion {microrl | demo} - print version of microrl lib or "
        "version of this demo src\n\r");
  print("\thelp  - this message\n\r");
  print("\tclear - clear screen\n\r");
  print("\tlist  - list all commands in tree\n\r");
  print("\tname [string] - print 'name' value if no 'string', set name "
        "value to 'string' if 'string' present\n\r");
  print("\tvalue [value] - print or set long value\n\r");
  print("\tlisp - dummy command for demonstation auto-completion, "
        "while inputed 'l+<TAB>'\n\r");
}
//-----------------------------------------------------------------------------
// execute callback for microrl library
// do what you want here, but don't write to argv!!! read only!!
static int execute(int argc, const char * const * argv)
{
  int i = 0;
  // just iterate through argv word and compare it with your commands
  if (argc > 0)
  {
    if (strcmp(argv[i], _CMD_HELP) == 0)
    {
      print("MicroRL library based shell\n\r");
      print_help(); // print help
    }
    else if (strcmp(argv[i], _CMD_NAME) == 0)
    {
      if ((++i) < argc)
      { // if value preset
        if (strlen(argv[i]) < _NAME_LEN)
          strcpy(name, argv[i]);
        else
          print("name value too long!\n\r");
      }
      else
      {
        print(name);
        print("\n\r");
      }
    }
    else if (strcmp(argv[i], _CMD_VALUE) == 0)
    {
      if ((++i) < argc)
      { // if value preset
#ifdef MRL_STR2LONG
        value = mrl_str2long(argv[i], 0, 0);
#else
        value = atol(argv[i]);
#endif
      }
      else
      {
	char str[80];
	snprintf(str, sizeof(str) - 1, "%li", value);
        print(str);
        print("\n\r");
      }
    }
    else if (strcmp (argv[i], _CMD_VER) == 0)
    {
      if (++i < argc)
      {
        if (strcmp (argv[i], _SCMD_DEMO) == 0)
        {
          print("demo v1.0f\n\r");
        }
        else if(strcmp (argv[i], _SCMD_MRL) == 0)
        {
          print("MicroRL " MRL_LIB_VER "\n\r");
        }
        else
        {
          print((char*)argv[i]);
          print(" wrong argument, see help\n\r");
        }
      }
      else
      {
        print("version needs 1 parametr, see help\n\r");
      }
    }
    else if (strcmp (argv[i], _CMD_CLEAR) == 0)
    {
      print("\033[2J"); // ESC seq for clear entire screen
      print("\033[H");  // ESC seq for move cursor at left-top corner
    }
    else if (strcmp (argv[i], _CMD_LIST) == 0)
    {
      print("available command:\n"); // print all command per line
      for (int i = 0; i < _NUM_OF_CMD; i++)
      {
        print ("\t");
        print (keyworld[i]);
        print ("\n\r");
      }
    }
    else
    {
      print("command: '");
      print((char*)argv[i]);
      print("' not found.\n\r");
    }
  }
  return 0;
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_COMPLETE
// completion callback for microrl library
static char **complete(int argc, const char * const * argv)
{
  int i, j = 0;
  compl_world[0] = NULL;

  // if there is token in cmdline
  if (argc == 1)
  { // get last entered token
    char * bit = (char*)argv [argc-1];

    // iterate through our available token and match it
    for (i = 0; i < _NUM_OF_CMD; i++)
    { // if token is matched (text is part of our token starting from 0 char)
      if (strstr(keyworld [i], bit) == keyworld [i])
      {  // add it to completion set
        compl_world [j++] = keyworld [i];
      }
    }
  }
  else if ((argc > 1) && (strcmp (argv[0], _CMD_VER)==0))
  { // if command needs subcommands
    // iterate through subcommand for command _CMD_VER array
    for (i = 0; i < _NUM_OF_VER_SCMD; i++)
      if (strstr (ver_keyworld [i], argv [argc-1]) == ver_keyworld [i])
        compl_world [j++] = ver_keyworld [i];
  }
  else
  { // if there is no token in cmdline, just print all available token
    for (; j < _NUM_OF_CMD; j++)
      compl_world[j] = keyworld [j];
  }

  // note! last ptr in array always must be NULL!!!
  compl_world[j] = NULL;

  // return set of variants
  return compl_world;
}
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
#ifdef MRL_USE_CTRL_C
static void sigint()
{
  print ("CTRL-C catched!\r\n");
}
#endif // MRL_USE_CTRL_C
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  // MicroRL object
  mrl_t mrl;

  // call init with ptr to microrl instance and print callback
  mrl_init(&mrl, print);

  // set callback for execute
  mrl_set_execute_cb(&mrl, execute);

#ifdef MRL_USE_COMPLETE
  // set callback for completion
  mrl_set_complete_cb(&mrl, complete);
#endif // MRL_USE_COMPLETE

#ifdef MRL_USE_CTRL_C
  // set callback for Ctrl+C
  mrl_set_sigint_cb(&mrl, sigint);
#endif // MRL_USE_CTRL_C

  while (1)
  { // put received char from stdin to microrl lib
    int ch = get_char();
    int rv = mrl_insert_char(&mrl, ch);
    //printf("\n0x%02X\n", ch);
    if (rv)
    {
      printf("\nmrl_insert_char(0x%02X) return 0x%02X; exit\n", ch, rv);
      break;
    }
  }

  return 0;
}
//-----------------------------------------------------------------------------

/*** end of "mrl_test.c" file ***/

