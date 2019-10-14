/*
 * MicroRL library config file
 * File "mrl_conf.h"
 */

#ifndef MRL_CONF_H
#define MRL_CONF_H
//-----------------------------------------------------------------------------
#define MRL_LIB_VER "1.5.1f" // delivered from "1.5.1" ('f' - forked)
//-----------------------------------------------------------------------------
// Debug output
//#undef MRL_DEBUG
//-----------------------------------------------------------------------------
// Command line length, define cmdline buffer size.
// Set max number of chars + 1, because last byte of buffer need to contain
// '\0' - NULL terminator, and not use for storing inputed char.
// If user input chars more then it parametrs - 1, chars not added to
// command line.
#define MRL_COMMAND_LINE_LEN (1 + 100) // for 32 chars
//-----------------------------------------------------------------------------
// Command token number, define max token it command line, if number of token
// typed in command line exceed this value, then prints message about it and
// command line not to be parced and 'execute' callback will not calls.
// Token is word separate by white space, for example 3 token line:
// "=> set mode test"
#define MRL_COMMAND_TOKEN_NMB 8
//-----------------------------------------------------------------------------
// Define you prompt string here. You can use colors escape code,
// for highlight you prompt (if you terminal supports color).
// #define MRL_PROMPT_DEFAULT "\033[32m=> \033[0m" - green color
#define MRL_PROMPT_DEFAULT "\033[32m=> \033[0m" // green color
//#define MRL_PROMPT_DEFAULT "=> " // default color
//-----------------------------------------------------------------------------
// Define prompt text (without ESC sequence, only text) prompt length,
// it needs because if you use ESC sequence, it's not possible detect
// only text length.
#define MRL_PROMPT_LEN 3 // strlen("=> ")
//-----------------------------------------------------------------------------
// Define it, if you wanna use completion functional, also set completion
// callback in you code, now if user press TAB calls 'copmlitetion'
// callback. If you no need it, you can just set NULL to callback ptr and
// do not use it, but for memory saving tune, if you are not going to use
// it - disable this define.
#define MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
// Define it, if you wanna use history. It s work's like bash history, and
// set stored value to cmdline, if UP and DOWN key pressed. Using history add
// memory consuming, depends from _RING_HISTORY_LEN parametr
#define MRL_USE_HISTORY
//-----------------------------------------------------------------------------
// History ring buffer length, define static buffer size.
// For saving memory, each entered cmdline store to history in ring buffer,
// so we can not say, how many line we can store, it depends from cmdline len,
// but memory using more effective. We not prefer dinamic memory allocation
// for small and embedded devices. Overhead is 2 char on each saved line
#define MRL_RING_HISTORY_LEN 128
//-----------------------------------------------------------------------------
// Enable Handling terminal ESC sequence. If disabling, then cursor arrow,
// HOME, END, DEL will not work, use Ctrl+A(B,F,P,N,A,E,H,K,U,C) but
// decrease code memory.
#define MRL_USE_ESC_SEQ
//-----------------------------------------------------------------------------
// Use snprintf from you standard complier library, but it gives some overhead.
// If not defined, use u16int_to_str() function, it's decrease size of of code.
// Try to build with and without, and compare total code size for tune library.
#define MRL_USE_LIBC_STDIO
//-----------------------------------------------------------------------------
// Enable 'interrupt signal' callback, if user press Ctrl+C
#define MRL_USE_CTLR_C
//-----------------------------------------------------------------------------
// Print prompt at 'microrl_init', if enable, prompt will print at startup,
// otherwise first prompt will print after first press Enter in terminal
// NOTE!: Enable it, if you call 'microrl_init' after your communication
// subsystem already initialize and ready to print message
#define MRL_ENABLE_INIT_PROMPT
//-----------------------------------------------------------------------------
// Selected new line symbol(s)
#define MRL_ENDL_LF

#if defined(MRL_ENDL_CR)
#  define MRL_ENDL "\r"
#elif defined(MRL_ENDL_CRLF)
#  define MRL_ENDL "\r\n"
#elif defined(MRL_ENDL_LF)
#  define MRL_ENDL "\n"
#elif defined(MRL_ENDL_LFCR)
# define MRL_ENDL "\n\r"
#else
#  error "You must define new line symbol."
#endif
//-----------------------------------------------------------------------------
#if MRL_RING_HISTORY_LEN > 256
#error "This history implementation (ring buffer with 1 byte iterator) "
       "allow 256 byte buffer size maximum"
#endif
//-----------------------------------------------------------------------------
#endif // MRL_CONF_H

/*** end of "mrl_conf.h" file ***/
