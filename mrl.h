/*
 * MicroRL library
 * File "mrl.h"
 */

#pragma once
#ifndef MRL_H
#define MRL_H
//-----------------------------------------------------------------------------
#include "mrl_conf.h"
#include <stdbool.h>
//-----------------------------------------------------------------------------
#ifndef INLINE
#  define INLINE static inline
#endif
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
// history struct, contain internal variable
// history store in static ring buffer for memory saving
typedef struct {
  char buf[MRL_RING_HISTORY_LEN]; // ring buffer
  int begin; // begin index of olderst line
  int end;   // end index (index of future line) 
  int cur;   // index of current string in buffer
  int last;  // index of last string in buffer
  bool first_save;
} mrl_hist_t;
#endif // MRL_USE_HISTORY
//-----------------------------------------------------------------------------
// microrl struct, contain internal library data
typedef struct {
#ifdef MRL_USE_HISTORY
  mrl_hist_t hist; // history object
#endif

#ifdef MRL_USE_ESC_SEQ
  char escape_seq;
#endif

#if (defined(MRL_ENDL_CRLF) || defined(MRL_ENDL_LFCR))
  char tmpch;
#endif

  const char *prompt; // pointer to prompt string
  int prompt_len; // prompt sting length without escape chars
  
  char cmdline[MRL_COMMAND_LINE_LEN]; // cmdline buffer
  int cmdlen; // last position in command line
  
  int cursor; // input cursor

  // ptr to 'print' callback
  void (*print) (const char *str);

  // ptr to 'execute' callback
  void (*execute) (int argc, char * const argv[]);
  
#ifdef MRL_USE_COMPLETE
  // ptr to 'completion' callback (optoinal)
  const char** (*get_completion) (int argc, char * const argv[]);
#endif // MRL_USE_COMPLETE

#ifdef MRL_USE_CTRL_C
  // ptr to 'CTRL+C' callback (optional)
  void (*sigint) (void);
#endif // MRL_USE_CTRL_C
} mrl_t;
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
// init internal data, calls once at start up
void mrl_init(mrl_t *self, void (*print)(const char*));
//-----------------------------------------------------------------------------
// set prompt string
INLINE void mrl_set_prompt(mrl_t *self, const char *prompt, int prompt_len)
{
  self->prompt     = prompt;
  self->prompt_len = prompt_len;
}
//-----------------------------------------------------------------------------
// erase all line and go to begin
void mrl_clear(mrl_t *self);
//-----------------------------------------------------------------------------
// print prompt (reset cmdline and cursor position)
void mrl_prompt(mrl_t *self);
//-----------------------------------------------------------------------------
// refresh line (print prompt + restore cmdline and cursor position)
void mrl_refresh(mrl_t *self);
//-----------------------------------------------------------------------------
// pointer to callback func, that called when user press 'Enter'
// execute func param: argc - argument count, argv - pointer array to
// token string
INLINE void mrl_set_execute_cb(mrl_t *self,
                               void (*execute)(int, char * const[]))
{
  self->execute = execute;
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_COMPLETE
// set pointer to callback complition func, that called when user press 'Tab'
// callback func description:
//   param: argc - argument count, argv - pointer array to token string
//   must return NULL-terminated string, contain complite variant splitted
//   by 'Whitespace'. If complite token found, it's must contain only one
//   token to be complitted Empty string if complite not found, and multiple
//   string if there are some token
INLINE void mrl_set_complete_cb(
        mrl_t *self, const char**(*get_completion)(int, char * const[]))
{
  self->get_completion = get_completion;
}
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
#ifdef MRL_USE_CTRL_C
// set callback for Ctrl+C terminal signal
INLINE void mrl_set_sigint_cb(mrl_t *self, void (*sigintf)(void))
{
  self->sigint = sigintf;
}
#endif // MRL_USE_CTRL_C
//-----------------------------------------------------------------------------
// insert char to cmdline (for example call in usart RX interrupt)
// (return non zero key code if Ctrl+KEY pressed, else 0)
int mrl_insert_char(mrl_t *self, int ch);
//----------------------------------------------------------------------------
#if defined(MRL_UINT2STR) || defined(MRL_INT2STR) || !defined(MRL_USE_LIBC_STDIO)
// convert unsigned integer value to string (return string length)
int mrl_uint2str(unsigned value, char *buf);
#endif // MRL_UINT2STR || MRL_INT2STR || !MRL_USE_LIBC_STDIO
//----------------------------------------------------------------------------
#ifdef MRL_INT2STR
// convert integer value to string (return string length)
int mrl_int2str(int value, char *buf);
#endif // MRL_INT2STR
//----------------------------------------------------------------------------
#ifdef MRL_STR2INT
// C-style string to integer transformation (0xHHHH-hex, 0OOO-oct, 0bBBBB-bin)
// - atoi() alternative for use into execute command callback.
// base: 0-auto, 2, 8, 10, 16
int mrl_str2int(const char *str, int def_val, unsigned char base);
#endif // MRL_STR2INT
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // MRL_H

/*** end of "mrl.h" file ***/

