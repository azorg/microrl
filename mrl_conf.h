/*
 * MicroRL library (config file)
 * File "mrl_conf.h"
 */

#pragma once
#ifndef MRL_CONF_H
#define MRL_CONF_H
//-----------------------------------------------------------------------------
// Off echo on half duplex serial interfaces
//#define MRL_ECHO_OFF // FIXME
//-----------------------------------------------------------------------------
// Command line length, define cmdline buffer size.
// Set max number of chars + 1 because last byte of buffer need to
// contain '\0' - NULL terminator, and not use for storing inputed char.
#ifndef MRL_COMMAND_LINE_LEN
#  define MRL_COMMAND_LINE_LEN 200 // FIXME
#endif
//-----------------------------------------------------------------------------
// Command token number, define max token it command line, if number of token
// typed in command line exceed this value, then prints message about it and
// command line not to be parsed and 'execute' callback will not calls.
// Token is word separate by white space, for example 3 token line:
// "=> set mode test"
#ifndef MRL_COMMAND_TOKEN_NUM 
#  define MRL_COMMAND_TOKEN_NUM 20 // FIXME
#endif
//-----------------------------------------------------------------------------
// Define you prompt string here. You can use colors escape code,
// for highlight you prompt (if you terminal supports color).
// #define MRL_PROMPT_DEFAULT "\033[31m=>\033[0m " - red color
// #define MRL_PROMPT_DEFAULT "\033[32m=>\033[0m " - green color
// #define MRL_PROMPT_DEFAULT "\033[33m=>\033[0m " - yellow color
// #define MRL_PROMPT_DEFAULT "\033[34m=>\033[0m " - blue color
// #define MRL_PROMPT_DEFAULT "\033[35m=>\033[0m " - magenta color
// #define MRL_PROMPT_DEFAULT "=> "                - default color
#ifndef MRL_PROMPT_DEFAULT
//#define MRL_PROMPT_DEFAULT "\033[31m=>\033[0m "
//#define MRL_PROMPT_DEFAULT "\033[32m=>\033[0m "
//#define MRL_PROMPT_DEFAULT "=> "
#  define MRL_PROMPT_DEFAULT "$ " // FIXME
//#define MRL_PROMPT_DEFAULT "\033[32m$\033[0m " // FIXME
//#define MRL_PROMPT_DEFAULT "" // FIXME: EMPTY
#endif
//-----------------------------------------------------------------------------
// Define prompt text (without ESC sequence, only text) prompt length,
// it needs because if you use ESC sequence, it's not possible detect
// only text length.
#ifndef MRL_PROMPT_DEFAULT_LEN
//#define MRL_PROMPT_DEFAULT_LEN 3 // strlen("=> ")
#  define MRL_PROMPT_DEFAULT_LEN 2 // strlen("$ ") // FIXME
//#define MRL_PROMPT_DEFAULT_LEN 0 // strlen("") // FIXME
#endif
//-----------------------------------------------------------------------------
// Define it, if you wanna use completion functional, also set completion
// callback in you code, now if user press TAB calls 'copmlitetion'
// callback. If you no need it, you can just set NULL to callback ptr and
// do not use it, but for memory saving tune, if you are not going to use
// it - disable this define.
//#define MRL_USE_COMPLETE // FIXME
//-----------------------------------------------------------------------------
// Define it, if much alternatives for complete.
// Number of completion help columns.
#ifdef MRL_USE_COMPLETE
#  ifndef MRL_COMPLETE_COLS
#    define MRL_COMPLETE_COLS 10
#  endif
#endif
//-----------------------------------------------------------------------------
// Define it, if you wanna use history. It s work's like bash history, and
// set stored value to cmdline, if UP and DOWN key pressed. Using history add
// memory consuming, depends from MRL_RING_HISTORY_LEN parametr
//#define MRL_USE_HISTORY // FIXME
//-----------------------------------------------------------------------------
// History ring buffer length, define static buffer size.
// For saving memory, each entered cmdline store to history in ring buffer,
// so we can not say, how many line we can store, it depends from cmdline len,
// but memory using more effective. We not prefer dynamic memory allocation
// for small and embedded devices. Overhead is only 1 char on each saved line
#ifdef MRL_USE_HISTORY
#  ifndef MRL_RING_HISTORY_LEN
#    define MRL_RING_HISTORY_LEN 512 // FIXME
#  endif
#endif
//-----------------------------------------------------------------------------
// Enable Handling terminal ESC sequence. If disabling, then cursor arrow,
// HOME, END, DELETE will not work (use Ctrl+A(B,F,P,N,A,E,H,K,U,C,D)) but
// decrease code memory.
#define MRL_USE_ESC_SEQ // FIXME
//-----------------------------------------------------------------------------
// Disable out ESC sequence 
//#define MRL_PRINT_ESC_OFF // FIXME
//-----------------------------------------------------------------------------
// Use snprintf() from standard compiler library, but it gives some overhead.
// If not defined, use u16int_to_str() function, it's decrease size of code.
// Try to build with and without, and compare total code size for tune library.
//#define MRL_USE_LIBC_STDIO
//-----------------------------------------------------------------------------
// Enable 'interrupt signal' callback, if user press Ctrl+C
//#define MRL_USE_CTRL_C // FIXME
//-----------------------------------------------------------------------------
// Print prompt at 'microrl_init', if enable, prompt will print at startup,
// otherwise first prompt will print after first press Enter in terminal
// NOTE!: Enable it, if you call 'microrl_init' after your communication
// subsystem already initialise and ready to print message
//#define MRL_ENABLE_INIT_PROMPT
//-----------------------------------------------------------------------------
// Enable mrl_uint2str() - simple print unsigned integer value to string 
//#define MRL_UINT2STR
//-----------------------------------------------------------------------------
// Enable mrl_int2str() - simple print integer value to string 
//#define MRL_INT2STR
//-----------------------------------------------------------------------------
// Enable mrl_str2int() function as atoi() alternative for use into command
// execution callback (0xHHH, 0OOOO, 0bBBBB format supported)
#define MRL_STR2INT
//-----------------------------------------------------------------------------
// Selected new line symbol(s)
//#define MRL_ENDL_LF // Linux console
#define MRL_ENDL_CR_LF // one CR or one LF - in, CR+LF - out (minicom/Linux)

#if defined(MRL_ENDL_LF)
#  define MRL_ENDL "\n"
#elif defined(MRL_ENDL_CR)
#  define MRL_ENDL "\r"
#elif defined(MRL_ENDL_CRLF)
#  define MRL_ENDL "\r\n"
#elif defined(MRL_ENDL_LFCR)
#  define MRL_ENDL "\n\r"
#elif defined(MRL_ENDL_CR_LF)
#  define MRL_ENDL "\r\n"
#else
#  error "You must define new line symbol."
#endif
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
#  if MRL_RING_HISTORY_LEN < MRL_COMMAND_LINE_LEN
#    error "MRL_RING_HISTORY_LEN < MRL_COMMAND_LINE_LEN"
#  endif
#endif
//-----------------------------------------------------------------------------
#endif // MRL_CONF_H

/*** end of "mrl_conf.h" file ***/

