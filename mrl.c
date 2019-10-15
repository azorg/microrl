/*
 * MicroRL library
 * File "mrl.c"
 */

//-----------------------------------------------------------------------------
#include "mrl.h"
#include "mrl_defs.h"
//-----------------------------------------------------------------------------
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef MRL_USE_LIBC_STDIO
#include <stdio.h>
#endif
//-----------------------------------------------------------------------------
#ifdef MRL_DEBUG
#  include <stdio.h>
#  define MRL_DBG(...) fprintf(stderr, "\033[31m");\
                       fprintf(stderr, __VA_ARGS__);\
                       fprintf(stderr, "\033[0m");
#  define  MRL_DEBUG_HISTORY
#endif // MRL_DEBUG
//-----------------------------------------------------------------------------
static char * const mrl_prompt_default = MRL_PROMPT_DEFAULT;
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
#ifdef MRL_DEBUG_HISTORY
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
    else if (self->ring_buf[i] <= 9)
      printf("%d", self->ring_buf[i]);
    else
      printf("|");
  }

  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
    printf("%c", i == self->end ? 'e' : ' ');

  printf(MRL_ENDL);
}
#endif // MRL_DEBUG_HISTORY
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
static bool mrl_hist_is_space_for_new(mrl_ring_history_t *self, int len)
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

#ifdef MRL_DEBUG_HISTORY
  mrl_print_hist(self);
#endif // MRL_DEBUG_HISTORY
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
// replace all whitespaces to '\0'
static int mrl_split(mrl_t *self, int limit, char const **tkn_arr)
{
  int i = 0;
  int ind = 0;
  while (1)
  {
    // skip whitespaces and replace them to '\0'
    while ((self->cmdline[ind] == ' ') && (ind < limit))
      self->cmdline[ind++] = '\0';

    if (ind >= limit) return i;

    tkn_arr[i++] = self->cmdline + ind;

    if (i >= MRL_COMMAND_TOKEN_NMB) return -1;

    // skip NOT whitespaces
    while ((self->cmdline[ind] != ' ') && (ind < limit))
      ind++;
    
    if (ind >= limit) return i;
  }
  return i;
}
//-----------------------------------------------------------------------------
// back replace '\0' to whitespaces
static void mrl_back_replace_spaces(mrl_t *self, int limit)
{
  int i;
  for (i = 0; i < limit; i++)
    if (self->cmdline[i] == '\0')
      self->cmdline[i] = ' ';
}
//-----------------------------------------------------------------------------
INLINE void mrl_print_prompt(mrl_t *self)
{
  self->print(self->prompt_str);
}
//-----------------------------------------------------------------------------
INLINE void mrl_terminal_backspace(mrl_t *self)
{
  self->print("\033[D \033[D");
}
//-----------------------------------------------------------------------------
INLINE void mrl_terminal_newline(mrl_t *self)
{
  self->print(MRL_ENDL);
}
//-----------------------------------------------------------------------------
#ifndef MRL_USE_LIBC_STDIO
// convert unsigned int value to string (return string length)
static int mrl_uint2str(unsigned int nmb, char *buf)
{
  int i = 0, j = 0, n;
  do {
    buf[i++] = (nmb % 10) + '0';
    nmb /= 10;
  } while (nmb);
  buf[n = i--] = '\0';
  while (j < i)
  { // echange bytes
    char c   = buf[j];
    buf[j++] = buf[i];
    buf[i--] = c;
  }
  return n;
}
#endif // !MRL_USE_LIBC_STDIO
//-----------------------------------------------------------------------------
// set cursor at position from begin cmdline (after prompt) + offset
static void mrl_terminal_move_cursor(mrl_t *self, int offset)
{
  char str[16];
#ifdef MRL_USE_LIBC_STDIO 
  if (offset > 0)
    snprintf(str, sizeof(str) - 1, "\033[%dC", offset);
  else if (offset < 0)
    snprintf(str, sizeof(str) - 1, "\033[%dD", -offset);
#else 
  char *ptr = str;

  *ptr++ = '\033';
  *ptr++ = '[';
  if (offset > 0)
  { // "\033[nC"
    ptr += mrl_uint2str((unsigned int) offset, ptr);
    *ptr++ = 'C';
  }
  else if (offset < 0)
  { // "\033[nD"
    ptr += mrl_uint2str((unsigned int) -offset, ptr);
    *ptr++ = 'D';
  }
  else // offset == 0
    return;

  *ptr = '\0';
#endif  
  self->print(str);
}
//-----------------------------------------------------------------------------
// set cursor to start position after prompt
static void mrl_terminal_reset_cursor(mrl_t *self)
{
  char str[16];
#ifdef MRL_USE_LIBC_STDIO
  snprintf(str, sizeof(str), "\033[%dD\033[%dC",
           MRL_COMMAND_LINE_LEN + MRL_PROMPT_LEN + 2, MRL_PROMPT_LEN);
#else
  char *ptr = str;

  *ptr++ = '\033';
  *ptr++ = '[';
  ptr += mrl_uint2str(MRL_COMMAND_LINE_LEN + MRL_PROMPT_LEN + 2, ptr);
  *ptr++ = 'D';
  
  *ptr++ = '\033';
  *ptr++ = '[';
  ptr += mrl_uint2str(MRL_PROMPT_LEN, ptr);
  *ptr++ = 'C';
    
  *ptr  = '\0';
#endif
  self->print(str);
}
//-----------------------------------------------------------------------------
// print cmdline to screen
static void mrl_terminal_print_line(mrl_t *self, int pos, int cursor)
{
  self->print("\033[K"); // delete all from cursor to end

  self->cmdline[self->cmdlen] = '\0'; // FIXME ?!

  if (pos <= self->cmdlen) // FIXME !!!
    self->print(self->cmdline + pos);

  mrl_terminal_reset_cursor(self);
  mrl_terminal_move_cursor(self, cursor);
}
//-----------------------------------------------------------------------------
void mrl_init(mrl_t *self, void (*print)(const char *))
{
#ifdef MRL_USE_HISTORY
  memset(self->ring_hist.ring_buf, 0, MRL_RING_HISTORY_LEN);
  self->ring_hist.begin = 0;
  self->ring_hist.end = 0;
  self->ring_hist.cur = 0;
#endif

#ifdef MRL_USE_ESC_SEQ
  self->escape_seq = MRL_ESC_STOP;
#endif

#if (defined(MRL_ENDL_CRLF) || defined(MRL_ENDL_LFCR))
  self->tmpch = '\0';
#endif

  memset(self->cmdline, 0, MRL_COMMAND_LINE_LEN);
  self->prompt_str = mrl_prompt_default;
  self->cmdlen = 0;
  self->cursor = 0;

  self->print = print;
  self->execute = NULL;
  self->get_completion = NULL;

#ifdef MRL_USE_CTLR_C
  self->sigint = NULL;
#endif

#ifdef MRL_ENABLE_INIT_PROMPT
  mrl_print_prompt(self);
#endif
}
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
// cursor left pressed or CTRL-B
static void mrl_cursor_back(mrl_t *self)
{
  if (self->cursor > 0)
  {
    mrl_terminal_move_cursor(self, -1);
    self->cursor--;
  }
}
//-----------------------------------------------------------------------------
// cursor right pressed or CTRL-F
static void mrl_cursor_forward(mrl_t *self)
{
  if (self->cursor < self->cmdlen)
  {
    mrl_terminal_move_cursor(self, 1);
    self->cursor++;
  }
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_ESC_SEQ
// handling escape sequences
static void mrl_escape_process(mrl_t *self, char ch)
{
  if (ch == '[')
  {
    self->escape_seq = MRL_ESC_BRACKET;
  }
  else if (self->escape_seq == MRL_ESC_BRACKET)
  {
    if (ch == 'A')
    { // cursor up
#ifdef MRL_USE_HISTORY
      mrl_hist_search(self, MRL_HIST_UP);
#endif
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (ch == 'B')
    { // cursor down
#ifdef MRL_USE_HISTORY
      mrl_hist_search(self, MRL_HIST_DOWN);
#endif
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (ch == 'C')
    { // cursor forward
      mrl_cursor_forward(self);
      self->escape_seq = 0;
    }
    else if (ch == 'D')
    { // cursor back
      mrl_cursor_back(self);
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (ch == '7')
    {
      self->escape_seq = MRL_ESC_HOME;
    }
    else if (ch == '8')
    {
      self->escape_seq = MRL_ESC_END;
    } 
    else if (ch == '3')
    {
      self->escape_seq = MRL_ESC_DELETE;
    } 
    else
      self->escape_seq = MRL_ESC_STOP; // unknown escape sequence, stop
  }
  else if (ch == '~')
  {
    if (self->escape_seq == MRL_ESC_HOME)
    { // Home
      mrl_terminal_reset_cursor(self);
      self->cursor = 0;
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (self->escape_seq == MRL_ESC_END)
    { // End
      mrl_terminal_move_cursor(self, self->cmdlen-self->cursor);
      self->cursor = self->cmdlen;
      self->escape_seq = MRL_ESC_STOP;
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
      self->escape_seq = MRL_ESC_STOP;
    }
  }
  else
    self->escape_seq = MRL_ESC_STOP; // unknown escape sequence, stop
}
#endif // MRL_USE_ESC_SEQ
//-----------------------------------------------------------------------------
// insert len char of text at cursor position
static bool mrl_insert_text(mrl_t *self, const char *text, int len)
{
  if (len > MRL_COMMAND_LINE_LEN - self->cmdlen) 
    len = MRL_COMMAND_LINE_LEN - self->cmdlen;
  
  if (len > 0)
  {
    memmove(self->cmdline + self->cursor + len,
	    self->cmdline + self->cursor,
	    self->cmdlen  - self->cursor);

    memcpy(self->cmdline + self->cursor, text, len);
    
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

  if (self->cmdline[self->cursor - 1] == '\0')
    tkn_arr[status++] = "";

  compl_token = self->get_completion(status, tkn_arr);

  mrl_back_replace_spaces(self, self->cursor);

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
    mrl_terminal_reset_cursor(self);
    mrl_terminal_print_line(self, 0, self->cursor);
  } 
}
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
void mrl_new_line_handler(mrl_t *self)
{
  char const *tkn_arr[MRL_COMMAND_TOKEN_NMB];
  int status;

  mrl_terminal_newline(self);

#ifdef MRL_USE_HISTORY
  if (self->cmdlen > 0)
    mrl_hist_save_line(&self->ring_hist, self->cmdline, self->cmdlen);
#endif

  status = mrl_split(self, self->cmdlen, tkn_arr);

  if (status < 0)
    self->print("ERROR: too many tokens" MRL_ENDL);
  else if ((status > 0) && (self->execute != NULL))
    self->execute(status, tkn_arr);
  
  mrl_back_replace_spaces(self, self->cmdlen);

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
  if (self->escape_seq)
  {
    mrl_escape_process(self, ch);
    return 0;
  }
#endif
  switch (ch)
  {
#if defined(MRL_ENDL_CR)
    case MRL_KEY_CR:
      mrl_new_line_handler(self);
    break;

    case MRL_KEY_LF:
    break;

#elif defined(MRL_ENDL_CRLF)
    case MRL_KEY_CR:
      self->tmpch = MRL_KEY_CR;
    break;

    case MRL_KEY_LF:
    if (self->tmpch == MRL_KEY_CR)
      mrl_new_line_handler(self);
    break;

#elif defined(MRL_ENDL_LFCR)
    case MRL_KEY_LF:
      self->tmpch = MRL_KEY_LF;
    break;

    case MRL_KEY_CR:
    if (self->tmpch == MRL_KEY_LF)
      mrl_new_line_handler(self);
    break;
#else
    case MRL_KEY_CR:
    break;

    case MRL_KEY_LF:
      mrl_new_line_handler(self);
    break;
#endif

#ifdef MRL_USE_COMPLETE
    case MRL_KEY_HT: // TAB
      mrl_get_complite(self);
    break;
#endif

    case MRL_KEY_ESC: // ESC
#ifdef MRL_USE_ESC_SEQ
      self->escape_seq = MRL_ESC_START;
#endif
    break;

    case MRL_KEY_NAK: // ^U
      while (self->cursor > 0)
        mrl_backspace(self);
      mrl_terminal_print_line(self, 0, self->cursor);
    break;

    case MRL_KEY_VT:  // ^K
      self->print("\033[K");
      self->cmdlen = self->cursor;
    break;

    case MRL_KEY_ENQ: // ^E
      mrl_terminal_move_cursor(self, self->cmdlen-self->cursor);
      self->cursor = self->cmdlen;
    break;

    case MRL_KEY_SOH: // ^A
      mrl_terminal_reset_cursor(self);
      self->cursor = 0;
    break;

    case MRL_KEY_ACK: // ^F
      mrl_cursor_forward(self);
    break;

    case MRL_KEY_STX: // ^B
      mrl_cursor_back(self);
    break;

    case MRL_KEY_DLE: //^P
#ifdef MRL_USE_HISTORY
    mrl_hist_search(self, MRL_HIST_UP);
#endif
    break;

    case MRL_KEY_SO: //^N
#ifdef MRL_USE_HISTORY
    mrl_hist_search(self, MRL_HIST_DOWN);
#endif
    break;

    case MRL_KEY_DEL: // Backspace
    case MRL_KEY_BS: // ^U
      mrl_backspace(self);
      mrl_terminal_print_line(self, self->cursor, self->cursor);
    break;

    case MRL_KEY_DC2: // ^R
      mrl_terminal_newline(self);
      mrl_print_prompt(self);
      mrl_terminal_reset_cursor(self);
      mrl_terminal_print_line(self, 0, self->cursor);
    break;

    case MRL_KEY_ETX: // ^C
#ifdef MRL_USE_CTLR_C
      if (self->sigint != NULL)
        self->sigint();
#endif
      return MRL_KEY_ETX;

    case MRL_KEY_EOT: // ^D
      return MRL_KEY_EOT;

    default:
      if (((ch == ' ') && (self->cmdlen == 0)) || MRL_IS_CONTROL_CHAR(ch))
        break;
      if (mrl_insert_text(self, (const char*) &ch, 1))
        mrl_terminal_print_line(self, self->cursor - 1, self->cursor);
    break;
  } // switch (ch)

  return 0;
}
//----------------------------------------------------------------------------
#ifdef MRL_STR2LONG
long mrl_str2long(const char *str, long def_val, unsigned char base)
{
  unsigned char c, sign = 0;
  const unsigned char *p = (const unsigned char*) str;
  long retv = 0;
  
  if (str == (const char*) NULL) return def_val;

  while (1)
  {
    c = *p;
    if (c <  ' ') return def_val;
    if (c != ' ' && c != '\t') break;
    p++;
  }

  if (*p == '-')
  {
    p++; sign = 1;
  }
  else if (*p == '+')
  {
    p++; // sign = 0;
  }

  if (*p < '0') return def_val;

  else if (*p == '0')
  {
    if (p[1] < '0') return 0;
    p++;
    if (base == 0) base = 8; // OCT (ANSI C)
    if (*p == 'x' || *p == 'X')
    { // HEX
      base = 16;
      p++;
    }
    else if (*p == 'b' || *p == 'B')
    { // BIN
      base = 2;
      p++;
    }
    else if (*p == 'o' || *p == 'O')
    { // OCT
      base = 8;
      p++;
    }
  }

  if (*p < '0') return def_val;
  if (base == 0) base = 10;

  while (1)
  {
    c = *p++;
    if (c < '0') break;
    if (base > 10)
    {
      if      (c >= 'A' && c <= 'Z') c -= 'A' - '9' - 1;
      else if (c >= 'a' && c <= 'z') c -= 'a' - '9' - 1;
    }
    c -= '0';
    if (c >= base) return def_val; // illegal char
    retv *= (long) base;
    retv += (long) c;
  }

  return (sign == 0) ? retv : -retv;
}
#endif // MRL_STR2LONG
//-----------------------------------------------------------------------------

/* end of "mrl.c" file ***/

