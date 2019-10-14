/*
 * MicroRL library
 * File "mrl.c"
*/

//-----------------------------------------------------------------------------
#include "mrl.h"
#include "mrl_defs.h"
//-----------------------------------------------------------------------------
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef MRL_USE_LIBC_STDIO
#include <stdio.h>
#endif
//-----------------------------------------------------------------------------
#ifdef MRL_DEBUG
#  include <stdio.h>
#  define MRL_DBG(...) fprintf(stderr, "\033[33m");\
                       fprintf(stderr, __VA_ARGS__);\
                       fprintf(stderr, "\033[0m");
#  define  MRL_HISTORY_DEBUG
#endif // MRL_DEBUG
//-----------------------------------------------------------------------------
#ifndef INLINE
#  define INLINE static inline
#endif
//-----------------------------------------------------------------------------
static char * const mrl_prompt_default = MRL_PROMPT_DEFAULT;
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
#ifdef MRL_HISTORY_DEBUG
//-----------------------------------------------------------------------------
// print buffer content on screen
static void mrl_print_hist(mrl_ring_history_t *self)
{
  int i;
  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
    printf("%c", i == self->begin ? 'b' : ' ');

  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
  {
    if (self->ring_buf[i] >= ' ')
      printf("%c", self->ring_buf[i]);
    else 
      printf("%d", self->ring_buf[i]);
  }

  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
    printf("%c", i == self->end ? 'e' : ' ');

  printf(MRL_ENDL);
}
#endif // MRL_HISTORY_DEBUG
//-----------------------------------------------------------------------------
// remove older message from ring buffer
static void mrl_hist_erase_older(mrl_ring_history_t *self)
{
  int new_pos = self->begin + self->ring_buf[self->begin] + 1;
  if (new_pos >= MRL_RING_HISTORY_LEN)
    new_pos -= MRL_RING_HISTORY_LEN;
  self->begin = new_pos;
}
//-----------------------------------------------------------------------------
// check space for new line, remove older while not space
static int mrl_hist_is_space_for_new(mrl_ring_history_t *self, int len)
{
  if (self->ring_buf[self->begin] == 0)
    return true;

  if (self->end >= self->begin)
  {
    if (MRL_RING_HISTORY_LEN - self->end + self->begin - 1 > len)
      return true;
  }
  else
  {
    if (self->begin - self->end - 1 > len)
      return true;
  }
  return false;
}
//-----------------------------------------------------------------------------
// put line to ring buffer
static void mrl_hist_save_line(mrl_ring_history_t *self, char *line, int len)
{
  if (len > MRL_RING_HISTORY_LEN - 2)
    return;

  while (!mrl_hist_is_space_for_new(self, len))
    mrl_hist_erase_older(self);
  
  // if it's first line
  if (self->ring_buf[self->begin] == 0) 
    self->ring_buf[self->begin] = len;
  
  // store line
  if (len < MRL_RING_HISTORY_LEN - self->end - 1)
    memcpy(self->ring_buf + self->end + 1, line, len);
  else
  {
    int part_len = MRL_RING_HISTORY_LEN - self->end - 1;
    memcpy(self->ring_buf + self->end + 1, line, part_len);
    memcpy(self->ring_buf, line + part_len, len - part_len);
  }

  self->ring_buf[self->end] = len;
  self->end = self->end + len + 1;

  if (self->end >= MRL_RING_HISTORY_LEN)
    self->end -= MRL_RING_HISTORY_LEN;

  self->ring_buf[self->end] = 0;
  self->cur = 0;

#ifdef MRL_HISTORY_DEBUG
  mrl_print_hist(self);
#endif
}
//-----------------------------------------------------------------------------
// copy saved line to 'line' and return size of line
static int mrl_hist_restore_line(mrl_ring_history_t *self, char *line, int dir)
{
  int j, cnt = 0; // count history record  
  int header = self->begin;
  while (self->ring_buf[header])
  {
    header += self->ring_buf[header] + 1;
    if (header >= MRL_RING_HISTORY_LEN)
      header -= MRL_RING_HISTORY_LEN; 
    cnt++;
  }

  if (dir == MRL_HIST_UP)
  {
    if (cnt >= self->cur)
    {
      j = 0;
      header = self->begin;
      // found record for 'self->cur' index
      while ((self->ring_buf[header]) && (cnt - j - 1 != self->cur))
      {
        header += self->ring_buf [header] + 1;
        if (header >= MRL_RING_HISTORY_LEN)
          header -= MRL_RING_HISTORY_LEN;
        j++;
      }

      if (self->ring_buf[header])
      {
        self->cur++; // obtain saved line
        if (self->ring_buf[header] + header < MRL_RING_HISTORY_LEN)
        {
          memset(line, 0, MRL_COMMAND_LINE_LEN);
          memcpy(line, self->ring_buf + header + 1, self->ring_buf[header]);
        }
        else
        {
          int part0 = MRL_RING_HISTORY_LEN - header - 1;
          memset(line, 0, MRL_COMMAND_LINE_LEN);
          memcpy(line, self->ring_buf + header + 1, part0);
          memcpy(line + part0, self->ring_buf, self->ring_buf[header] - part0);
        }
        return self->ring_buf[header];
      }
    }
  }
  else
  {
    if (self->cur > 0)
    {
      j = 0;
      header = self->begin;
      self->cur--;

      while ((self->ring_buf[header] != 0) && (cnt - j != self->cur))
      {
        header += self->ring_buf[header] + 1;
        if (header >= MRL_RING_HISTORY_LEN)
          header -= MRL_RING_HISTORY_LEN;
        j++;
      }

      if (self->ring_buf[header] + header < MRL_RING_HISTORY_LEN)
      {
        memcpy(line, self->ring_buf + header + 1, self->ring_buf[header]);
      }
      else
      {
        int part0 = MRL_RING_HISTORY_LEN - header - 1;
        memcpy(line, self->ring_buf + header + 1, part0);
        memcpy(line + part0, self->ring_buf, self->ring_buf[header] - part0);
      }
      return self->ring_buf[header];
    }
    else
    { // empty line
      return 0;
    }
  }
  return -1;
}
#endif // MRL_USE_HISTORY
//-----------------------------------------------------------------------------
// split cmdline to tkn array and return nmb of token
static int mrl_split(mrl_t *self, int limit, char const **tkn_arr)
{
  int i = 0;
  int ind = 0;
  while (1)
  {
    // go to the first whitespace (zerro for us)
    while ((self->cmdline[ind] == '\0') && (ind < limit))
      ind++;

    if (!(ind < limit)) return i;

    tkn_arr[i++] = self->cmdline + ind;

    if (i >= MRL_COMMAND_TOKEN_NMB) return -1;

    // go to the first NOT whitespace (not zerro for us)
    while ((self->cmdline[ind] != '\0') && (ind < limit))
      ind++;
    
    if (!(ind < limit)) return i;
  }
  return i;
}
//-----------------------------------------------------------------------------
INLINE void mrl_print_prompt(mrl_t *self)
{
  self->print(self->prompt_str);
}
//-----------------------------------------------------------------------------
INLINE void mrl_terminal_backspace (mrl_t *self)
{
  self->print("\033[D \033[D");
}
//-----------------------------------------------------------------------------
INLINE void mrl_terminal_newline (mrl_t *self)
{
  self->print(MRL_ENDL);
}
//-----------------------------------------------------------------------------
#ifndef MRL_USE_LIBC_STDIO
// convert 16 bit value to string
// 0 value not supported!!! just make empty string
// Returns pointer to a buffer tail
static char *mrl_u16bit_to_str(unsigned int nmb, char * buf)
{
  char tmp_str[6] = {0, };
  int i = 0, j;
  if (nmb <= 0xFFFF)
  {
    while (nmb > 0)
    {
      tmp_str[i++] = (nmb % 10) + '0';
      nmb /= 10;
    }
    for (j = 0; j < i; ++j)
      *(buf++) = tmp_str [i-j-1];
  }
  *buf = '\0';
  return buf;
}
#endif // !MRL_USE_LIBC_STDIO
//-----------------------------------------------------------------------------
// set cursor at position from begin cmdline (after prompt) + offset
static void mrl_terminal_move_cursor(mrl_t *self, int offset)
{
  char str[16] = {0, };
#ifdef MRL_USE_LIBC_STDIO 
  if (offset > 0)
    snprintf(str, sizeof(str) - 1, "\033[%dC", offset);
  else if (offset < 0)
    snprintf(str, sizeof(str) - 1, "\033[%dD", -offset);
#else 
  char *endstr;
  strcpy(str, "\033[");
  if (offset > 0)
  {
    endstr = mrl_u16bit_to_str(offset, str + 2);
    strcpy(endstr, "C");
  } else if (offset < 0)
  {
    endstr = mrl_u16bit_to_str(-offset, str + 2);
    strcpy(endstr, "D");
  }
  else
    return;
#endif  
  self->print(str);
}
//-----------------------------------------------------------------------------
static void mrl_terminal_reset_cursor(mrl_t *self)
{
  char str[16];
#ifdef MRL_USE_LIBC_STDIO
  snprintf(str, sizeof(str), "\033[%dD\033[%dC",
           MRL_COMMAND_LINE_LEN + MRL_PROMPT_LEN + 2, MRL_PROMPT_LEN);
#else
  char *endstr;
  strcpy(str, "\033[");
  endstr = mrl_u16bit_to_str(MRL_COMMAND_LINE_LEN + MRL_PROMPT_LEN + 2,
                             str + 2);
  strcpy(endstr, "D\033[");
  endstr += 3;
  endstr = mrl_u16bit_to_str(MRL_PROMPT_LEN, endstr);
  strcpy(endstr, "C");
#endif
  self->print(str);
}
//-----------------------------------------------------------------------------
// print cmdline to screen, replace '\0' to wihitespace 
static void mrl_terminal_print_line(mrl_t *self, int pos, int cursor)
{
  char nch[] = {0,0};
  int i;
  self->print("\033[K"); // delete all from cursor to end
  for (i = pos; i < self->cmdlen; i++)
  {
    nch [0] = self->cmdline[i];
    if (nch[0] == '\0')
      nch[0] = ' ';
    self->print(nch);
  }
  mrl_terminal_reset_cursor(self);
  mrl_terminal_move_cursor(self, cursor);
}
//-----------------------------------------------------------------------------
void mrl_init(mrl_t *self, void (*print)(const char *))
{
  memset(self->cmdline, 0, MRL_COMMAND_LINE_LEN);
#ifdef MRL_USE_HISTORY
  memset(self->ring_hist.ring_buf, 0, MRL_RING_HISTORY_LEN);
  self->ring_hist.begin = 0;
  self->ring_hist.end = 0;
  self->ring_hist.cur = 0;
#endif
  self->cmdlen = 0;
  self->cursor = 0;
  self->execute = NULL;
  self->get_completion = NULL;
#ifdef MRL_USE_CTLR_C
  self->sigint = NULL;
#endif
  self->prompt_str = mrl_prompt_default;
  self->print = print;
#ifdef MRL_ENABLE_INIT_PROMPT
  mrl_print_prompt(self);
#endif
}
//-----------------------------------------------------------------------------
void mrl_set_complete_cb(mrl_t *self,
                         char ** (*get_completion)(int, const char* const*))
{
  self->get_completion = get_completion;
}
//-----------------------------------------------------------------------------
void mrl_set_execute_cb(mrl_t *self, int (*execute)(int, const char* const*))
{
  self->execute = execute;
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_CTLR_C
void mrl_set_sigint_cb(mrl_t *self, void (*sigintf)(void))
{
  self->sigint = sigintf;
}
#endif // MRL_USE_CTRL_C
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
static void mrl_hist_search(mrl_t *self, int dir)
{
  int len = mrl_hist_restore_line(&self->ring_hist, self->cmdline, dir);
  if (len >= 0)
  {
    self->cmdline[len] = '\0';
    self->cursor = self->cmdlen = len;
    mrl_terminal_reset_cursor(self);
    mrl_terminal_print_line(self, 0, self->cursor);
  }
}
#endif // MRL_USE_HISTORY
//-----------------------------------------------------------------------------
// remove one char at cursor
static void mrl_backspace(mrl_t *self)
{
  if (self->cursor > 0)
  {
    mrl_terminal_backspace(self);
    memmove(self->cmdline + self->cursor - 1,
            self->cmdline + self->cursor,
            self->cmdlen  - self->cursor + 1);
    self->cursor--;
    self->cmdline[self->cmdlen] = '\0';
    self->cmdlen--;
  }
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_ESC_SEQ
// handling escape sequences
static bool mrl_escape_process(mrl_t *self, char ch)
{
  if (ch == '[')
  {
    self->escape_seq = MRL_ESC_BRACKET;
    return false;
  }
  else if (self->escape_seq == MRL_ESC_BRACKET)
  {
    if (ch == 'A')
    { // cursor up
#ifdef MRL_USE_HISTORY
      mrl_hist_search(self, MRL_HIST_UP);
#endif
      return true;
    }
    else if (ch == 'B')
    { // cursor down
#ifdef MRL_USE_HISTORY
      mrl_hist_search(self, MRL_HIST_DOWN);
#endif
      return true;
    }
    else if (ch == 'C')
    { // cursor forward
      if (self->cursor < self->cmdlen)
      {
        mrl_terminal_move_cursor(self, 1);
        self->cursor++;
      }
      return true;
    }
    else if (ch == 'D')
    { // cursor back
      if (self->cursor > 0)
      {
        mrl_terminal_move_cursor(self, -1);
        self->cursor--;
      }
      return true;
    }
    else if (ch == '7')
    {
      self->escape_seq = MRL_ESC_HOME;
      return false;
    }
    else if (ch == '8')
    {
      self->escape_seq = MRL_ESC_END;
      return false;
    } 
    else if (ch == '3')
    {
      self->escape_seq = MRL_ESC_DELETE;
      return false;
    } 
  }
  else if (ch == '~')
  {
    if (self->escape_seq == MRL_ESC_HOME)
    { // Home
      mrl_terminal_reset_cursor(self);
      self->cursor = 0;
      return true;
    }
    else if (self->escape_seq == MRL_ESC_END)
    { // End
      mrl_terminal_move_cursor(self, self->cmdlen-self->cursor);
      self->cursor = self->cmdlen;
      return true;
    }
    else if (self->escape_seq == MRL_ESC_DELETE)
    { // Delete
      if (self->cursor < self->cmdlen)
      {
        mrl_terminal_move_cursor(self, 1);
        self->cursor++;
        mrl_backspace(self);
        mrl_terminal_print_line(self, self->cursor, self->cursor);
      }
    }
  }

  // unknown escape sequence, stop
  return true;
}
#endif // MRL_USE_ESC_SEQ
//-----------------------------------------------------------------------------
// insert len char of text at cursor position
static bool mrl_insert_text(mrl_t *self, char *text, int len)
{
  int i;
  if (self->cmdlen + len < MRL_COMMAND_LINE_LEN)
  {
    memmove(self->cmdline + self->cursor + len,
            self->cmdline + self->cursor,
            self->cmdlen  - self->cursor);
    for (i = 0; i < len; i++)
    {
      self->cmdline[self->cursor + i] = text[i];
      if (self->cmdline[self->cursor + i] == ' ')
        self->cmdline [self->cursor + i] = 0;
    }
    self->cursor += len;
    self->cmdlen += len;
    self->cmdline[self->cmdlen] = '\0';
    return true;
  }
  return false;
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_COMPLETE
static int mrl_common_len(char **arr)
{
  int i;
  int j;
  char *shortest = arr[0];
  int shortlen = strlen(shortest);

  for (i = 0; arr[i] != NULL; ++i)
    if (strlen(arr[i]) < shortlen)
    {
      shortest = arr[i];
      shortlen = strlen(shortest);
    }

  for (i = 0; i < shortlen; ++i)
    for (j = 0; arr[j] != 0; ++j)
      if (shortest[i] != arr[j][i])
        return i;

  return i;
}
//-----------------------------------------------------------------------------
static void mrl_get_complite(mrl_t *self)
{
  int status;
  char const *tkn_arr[MRL_COMMAND_TOKEN_NMB];
  char **compl_token; 
  
  if (self->get_completion == NULL) return; // callback was not set
  
  status = mrl_split(self, self->cursor, tkn_arr);
  if (self->cmdline[self->cursor-1] == '\0')
    tkn_arr[status++] = "";
  compl_token = self->get_completion (status, tkn_arr);
  if (compl_token[0] != NULL)
  {
    int i = 0;
    int len;

    if (compl_token[1] == NULL)
    {
      len = strlen(compl_token[0]);
    }
    else
    {
      len = mrl_common_len(compl_token);
      mrl_terminal_newline(self);
      while (compl_token [i] != NULL)
      {
        self->print(compl_token[i]);
        self->print (" ");
        i++;
      }
      mrl_terminal_newline(self);
      mrl_print_prompt(self);
    }
    
    if (len)
    {
      mrl_insert_text(self, compl_token[0] + strlen(tkn_arr[status - 1]), 
                      len - strlen(tkn_arr[status - 1]));
      if (compl_token[1] == NULL) 
        mrl_insert_text(self, " ", 1);
    }
    mrl_terminal_reset_cursor (self);
    mrl_terminal_print_line(self, 0, self->cursor);
  } 
}
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
void mrl_new_line_handler(mrl_t * self)
{
  char const *tkn_arr[MRL_COMMAND_TOKEN_NMB];
  int status;

  mrl_terminal_newline(self);

#ifdef MRL_USE_HISTORY
  if (self->cmdlen > 0)
    mrl_hist_save_line(&self->ring_hist, self->cmdline, self->cmdlen);
#endif

  status = mrl_split(self, self->cmdlen, tkn_arr);
  if (status == -1)
  {
    self->print("ERROR: too many tokens");
    self->print(MRL_ENDL);
  }
  if ((status > 0) && (self->execute != NULL))
    self->execute (status, tkn_arr);
  mrl_print_prompt(self);
  self->cmdlen = 0;
  self->cursor = 0;
  memset(self->cmdline, 0, MRL_COMMAND_LINE_LEN);

#ifdef MRL_USE_HISTORY
  self->ring_hist.cur = 0;
#endif
}
//-----------------------------------------------------------------------------
int mrl_insert_char(mrl_t *self, int ch)
{
#ifdef MRL_USE_ESC_SEQ
  if (self->escape)
  {
    if (mrl_escape_process(self, ch))
      self->escape = 0;
    return 0;
  }
#endif
  switch (ch)
  {
#ifdef MRL_ENDL_CR
    case KEY_CR:
      mrl_new_line_handler(self);
    break;

    case KEY_LF:
    break;

#elif defined(MRL_ENDL_CRLF)
    case KEY_CR:
      self->tmpch = KEY_CR;
    break;

    case KEY_LF:
    if (self->tmpch == KEY_CR)
      mrl_new_line_handler(self);
    break;

#elif defined(MRL_ENDL_LFCR)
    case KEY_LF:
      self->tmpch = KEY_LF;
    break;

    case KEY_CR:
    if (self->tmpch == KEY_LF)
      mrl_new_line_handler(self);
    break;
#else
    case KEY_CR:
    break;

    case KEY_LF:
      mrl_new_line_handler(self);
    break;
#endif

#ifdef MRL_USE_COMPLETE
    case KEY_HT:
      mrl_get_complite(self);
    break;
#endif

    case KEY_ESC:
#ifdef MRL_USE_ESC_SEQ
      self->escape = 1;
#endif
    break;

    case KEY_NAK: // ^U
      while (self->cursor > 0)
        mrl_backspace(self);
      mrl_terminal_print_line(self, 0, self->cursor);
    break;

    case KEY_VT:  // ^K
      self->print("\033[K");
      self->cmdlen = self->cursor;
    break;

    case KEY_ENQ: // ^E
      mrl_terminal_move_cursor(self, self->cmdlen-self->cursor);
      self->cursor = self->cmdlen;
    break;

    case KEY_SOH: // ^A
      mrl_terminal_reset_cursor(self);
      self->cursor = 0;
    break;

    case KEY_ACK: // ^F
      if (self->cursor < self->cmdlen)
      {
        mrl_terminal_move_cursor(self, 1);
        self->cursor++;
      }
    break;

    case KEY_STX: // ^B
      if (self->cursor)
      {
        mrl_terminal_move_cursor (self, -1);
        self->cursor--;
      }
    break;

    case KEY_DLE: //^P
#ifdef MRL_USE_HISTORY
    mrl_hist_search(self, MRL_HIST_UP);
#endif
    break;

    case KEY_SO: //^N
#ifdef MRL_USE_HISTORY
    mrl_hist_search(self, MRL_HIST_DOWN);
#endif
    break;

    case KEY_DEL: // Backspace
    case KEY_BS: // ^U
      mrl_backspace(self);
      mrl_terminal_print_line(self, self->cursor, self->cursor);
    break;

    case KEY_DC2: // ^R
      mrl_terminal_newline (self);
      mrl_print_prompt (self);
      mrl_terminal_reset_cursor (self);
      mrl_terminal_print_line (self, 0, self->cursor);
    break;

    case KEY_ETX: // ^C
#ifdef MRL_USE_CTLR_C
      if (self->sigint != NULL)
        self->sigint();
#endif
      return KEY_ETX;

    case KEY_EOT: // ^D
      return KEY_EOT;

    default:
      if (((ch == ' ') && (self->cmdlen == 0)) || MRL_IS_CONTROL_CHAR(ch))
        break;
      if (mrl_insert_text(self, (char*) &ch, 1))
        mrl_terminal_print_line(self, self->cursor - 1, self->cursor);
    break;
  } // switch (ch)

  return 0;
}
//-----------------------------------------------------------------------------

/* end of "mrl.c" file ***/


