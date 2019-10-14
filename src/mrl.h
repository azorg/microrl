/*
 * MicroRL library
 * File "mrl.h"
 */

#ifndef MRL_H
#define MRL_H
//-----------------------------------------------------------------------------
#include "mrl_conf.h"
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
// history struct, contain internal variable
// history store in static ring buffer for memory saving
typedef struct {
  char ring_buf [MRL_RING_HISTORY_LEN];
  int begin;
  int end;
  int cur;
} mrl_ring_history_t;
#endif // MRL_USE_HISTORY
//-----------------------------------------------------------------------------
// microrl struct, contain internal library data
typedef struct {
#ifdef MRL_USE_ESC_SEQ
  char escape_seq;
  char escape;
#endif

#if (defined(_ENDL_CRLF) || defined(_ENDL_LFCR))
  char tmpch;
#endif

#ifdef MRL_USE_HISTORY
  mrl_ring_history_t ring_hist; // history object
#endif

  char *prompt_str; // pointer to prompt string
  char cmdline [MRL_COMMAND_LINE_LEN]; // cmdline buffer
  int cmdlen; // last position in command line
  int cursor; // input cursor

  // ptr to 'print' callback
  void (*print) (const char *);

  // ptr to 'execute' callback
  int (*execute) (int argc, const char * const * argv);
  
  // ptr to 'completion' callback
  char ** (*get_completion) (int argc, const char * const * argv);

#ifdef MRL_USE_CTLR_C
  void (*sigint) (void);
#endif
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
// set echo mode (true/false), using for disabling echo for password input
// echo mode will enabled after user press Enter.
void mrl_set_echo(int);
//-----------------------------------------------------------------------------
// set pointer to callback complition func, that called when user press 'Tab'
// callback func description:
//   param: argc - argument count, argv - pointer array to token string
//   must return NULL-terminated string, contain complite variant splitted
//   by 'Whitespace'. If complite token found, it's must contain only one
//   token to be complitted Empty string if complite not found, and multiple
//   string if there are some token
void mrl_set_complete_cb(mrl_t *self,
                         char **(*get_completion)(int, const char* const*));
//-----------------------------------------------------------------------------
// pointer to callback func, that called when user press 'Enter'
// execute func param: argc - argument count, argv - pointer array to
// token string
void mrl_set_execute_cb(mrl_t *self, int (*execute)(int, const char* const*));
//-----------------------------------------------------------------------------
// set callback for Ctrl+C terminal signal
#ifdef MRL_USE_CTLR_C
void mrl_set_sigint_cb(mrl_t *self, void (*sigintf)(void));
#endif
//-----------------------------------------------------------------------------
// insert char to cmdline (for example call in usart RX interrupt)
// (return non zero key code if Ctrl+C or Ctrl+D pressed, else 0)
int mrl_insert_char(mrl_t *self, int ch);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------

#endif // MRL_H

/*** end of "mrl.h" file ***/


