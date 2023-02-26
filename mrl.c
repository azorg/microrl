/*
 * MicroRL library
 * File "mrl.c"
 */

//-----------------------------------------------------------------------------
#include "mrl.h"
#include "mrl_defs.h"
//-----------------------------------------------------------------------------
#include <string.h>
#ifdef MRL_USE_LIBC_STDIO
#include <stdio.h>
#endif
//-----------------------------------------------------------------------------
const char * mrl_prompt_default     = MRL_PROMPT_DEFAULT;
const int    mrl_prompt_default_len = MRL_PROMPT_DEFAULT_LEN;
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
#ifdef MRL_DEBUG_HISTORY
#include <stdio.h>
//-----------------------------------------------------------------------------
// print buffer content on screen
static void mrl_hist_print(const mrl_hist_t *self)
{
  int i;
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
    printf("%c", i == self->begin ? 'B' : ' ');

  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
    printf("%c", i == self->cur ? 'C' : ' ');

  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
  {
    if (self->buf[i] >= ' ')
      printf("%c", self->buf[i]);
    else
      printf(".");
  }

  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
    printf("%c", i == self->last ? 'L' : ' ');

  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
    printf("%c", i == self->end ? 'E' : ' ');
  
  printf(MRL_ENDL);
}
//-----------------------------------------------------------------------------
static void mrl_cmd_print(const mrl_t *self)
{
  int i;
  printf(MRL_ENDL "cmd=");
  for (i = 0; i < MRL_COMMAND_LINE_LEN; i++)
  {
    char c = self->cmdline[i];
    printf("%c", c >= ' ' ? c : c == '\0' ? '.' : '#');
  }
  printf(MRL_ENDL);
}
#endif // MRL_DEBUG_HISTORY
//-----------------------------------------------------------------------------
// init history ring buffer
static void mrl_hist_init(mrl_hist_t *self)
{
#ifdef MRL_DEBUG_HISTORY
  memset(self->buf, 0, MRL_RING_HISTORY_LEN);
#endif // MRL_DEBUG_HISTORY
  self->begin = 0;
  self->end   = 0;
  self->last  = 0;
  self->cur   = -1;
}
//-----------------------------------------------------------------------------
// save string to ring buffer
static bool mrl_hist_save(mrl_hist_t *self, const char *str)
{
  char ch;
  
  // go out from history search mode
  self->cur = -1;

  // trim whitespaces on the begin
  while (*str == ' ') str++;

  // don't save empty string
  if (*str == '\0') return false;

  if (self->last != self->end)
  { // ring buffer not empty
    int i = self->last, j = 0;
    while (1)
    {
      ch = str[j++];
      if (ch != self->buf[i]) break;
      if (ch == '\0')
        return false; // don't save the same string
      if (++i == MRL_RING_HISTORY_LEN) i = 0;
    }
  }
  
  self->last = self->end;
  do {
    ch = *str++;
    self->buf[self->end++] = ch;
    if (self->end == MRL_RING_HISTORY_LEN) self->end = 0;
    if (self->end == self->begin)
    { // delete from history olderest string
      do {
        if (++self->begin == MRL_RING_HISTORY_LEN) self->begin = 0;
      } while (self->buf[self->begin] != '\0');
      if (++self->begin == MRL_RING_HISTORY_LEN) self->begin = 0;
    }
  } while (ch != '\0');

  return true;
}
//-----------------------------------------------------------------------------
// backward history
// [used inside mrl_hist_search() only]
INLINE bool mrl_hist_backward(mrl_hist_t *self)
{
  int i;
  if (self->cur == self->begin) return false; // already at the begin
   
  i = self->cur - 2;
  if (i < 0) i += MRL_RING_HISTORY_LEN;

  while (self->buf[i] != '\0')
    if (--i < 0) i += MRL_RING_HISTORY_LEN; 

  if (++i == MRL_RING_HISTORY_LEN) i = 0;
  self->cur = i;

  return true;
}
//-----------------------------------------------------------------------------
// forward history
// [used inside mrl_hist_search() only]
INLINE bool mrl_hist_forward(mrl_hist_t *self)
{
  if (self->cur == self->last) return false; // already at the end

  while (self->buf[self->cur] != '\0')
    if (++self->cur == MRL_RING_HISTORY_LEN) self->cur = 0;

  if (++self->cur == MRL_RING_HISTORY_LEN) self->cur = 0;

  return true;
}
//-----------------------------------------------------------------------------
// get string from history by current index (return string length)
// [used inside mrl_hist_search() only]
INLINE int mrl_hist_get(const mrl_hist_t *self, char *buf)
{
  int len = 0, i = self->cur;
    
  while ((*buf++ = self->buf[i]) != '\0')
  {
    if (++i == MRL_RING_HISTORY_LEN) i = 0;
    len++;
  }

  return len;
}
//-----------------------------------------------------------------------------
// search history
static int mrl_hist_search(mrl_hist_t *self, int dir, char *str)
{
  if (self->last == self->end) return -1; // ring buffer empty

  if (dir == MRL_HIST_BACKWARD)
  {
    if (self->cur < 0)
    { // enter to history search mode
      int cur = self->last;
      self->first_save = mrl_hist_save(self, str);
      self->cur = cur;
    }
    else
      if (!mrl_hist_backward(self)) return -1; // no olderst history
  }
  else // dir == MRL_HIST_FORWARD
  {
    if (self->cur < 0) return -1; // no history search mode
    if (!mrl_hist_forward(self))
    { // no newrest history
      if (!self->first_save)
      { // return empty string
        self->cur = -1; // go out from history search mode
        *str = '\0';
        return 0;
      }
      return -1;
    }
  }

  return mrl_hist_get(self, str);
}
//-----------------------------------------------------------------------------
#endif // MRL_USE_HISTORY
//-----------------------------------------------------------------------------
INLINE void mrl_terminal_prompt(const mrl_t *self)
{
  self->print(self->prompt);
}
//-----------------------------------------------------------------------------
INLINE void mrl_terminal_print(const mrl_t *self, const char *str)
{
#ifndef MRL_PRINT_ESC_OFF
  self->print("\033[K"); // delete all from cursor to end
#endif // MRL_PRINT_ESC_OFF
  self->print(str);
}
//-----------------------------------------------------------------------------
INLINE void mrl_terminal_newline(const mrl_t *self)
{
  self->print(MRL_ENDL);
}
//-----------------------------------------------------------------------------
// set cursor position after prompt
static void mrl_terminal_cursor(const mrl_t *self, int cursor)
{
#ifndef MRL_PRINT_ESC_OFF
  char str[16];
  cursor += self->prompt_len;
#ifdef MRL_USE_LIBC_STDIO
  if (cursor == 0) strcpy(str, "\r");
  else snprintf(str, sizeof(str) - 1, "\r\033[%iC", cursor);
  //else snprintf(str, sizeof(str) - 1, "\033[%iG", cursor + 1);
#else
  char *ptr = str;
  *ptr++ = '\r';
  if (cursor)
  {
    *ptr++ = '\033';
    *ptr++ = '[';
     ptr  += mrl_uint2str(cursor, ptr);
    *ptr++ = 'C';
  }
  *ptr  = '\0';
#endif // MRL_USE_LIBC_STDIO
  self->print(str);

#else // MRL_PRINT_ESC_OFF
  int i;
  char tmp[2];
  tmp[1] = '\0';
  self->print("\r");
  mrl_terminal_prompt(self);
  for (i = 0; i < self->cmdlen && i < cursor; i++)
  {
    tmp[0] = self->cmdline[i];
    self->print(tmp);
  }
  for (; i < cursor; i++)
    self->print(" ");
#endif // MRL_PRINT_ESC_OFF
}
//-----------------------------------------------------------------------------
INLINE void mrl_terminal_cursor_back(const mrl_t *self)
{
#ifndef MRL_PRINT_ESC_OFF
  self->print("\033[D");
#else
  mrl_terminal_cursor(self, self->cursor - 1);
#endif // MRL_PRINT_ESC_OFF
}
//-----------------------------------------------------------------------------
INLINE void mrl_terminal_cursor_forward(const mrl_t *self)
{
#ifndef MRL_PRINT_ESC_OFF
  self->print("\033[C");
#else
  mrl_terminal_cursor(self, self->cursor + 1);
#endif // MRL_PRINT_ESC_OFF
}
//-----------------------------------------------------------------------------
// cursor LEFT or Ctrl+B pressed
static void mrl_cursor_back(mrl_t *self)
{
#ifndef MRL_ECHO_OFF
  if (self->cursor > 0)
  {
    mrl_terminal_cursor_back(self);
    self->cursor--;
  }
#else
  if (self->cursor > 0)
    self->cursor--;
  mrl_terminal_cursor(self, self->cursor);
#endif // MRL_ECHO_OFF
}
//-----------------------------------------------------------------------------
// cursor RIGHT or Ctrl+F pressed
static void mrl_cursor_forward(mrl_t *self)
{
#ifndef MRL_ECHO_OFF
  if (self->cursor < self->cmdlen)
  {
    mrl_terminal_cursor_forward(self);
    self->cursor++;
  }
#else
  if (self->cursor < self->cmdlen)
    self->cursor++;

  mrl_terminal_cursor(self, self->cursor);
#endif // MRL_ECHO_OFF
}
//-----------------------------------------------------------------------------
// HOME or Ctrl+A pressed
INLINE void mrl_cursor_home(mrl_t *self)
{
  mrl_terminal_cursor(self, self->cursor = 0);
}
//-----------------------------------------------------------------------------
// END or Ctrl+E pressed
INLINE void mrl_cursor_end(mrl_t *self)
{
  mrl_terminal_cursor(self, self->cursor = self->cmdlen);
}
//-----------------------------------------------------------------------------
// BACKSPACE or Ctrl+H pressed
INLINE void mrl_backspace(mrl_t *self)
{
  if (self->cursor > 0)
  {
#ifndef MRL_ECHO_OFF
    mrl_terminal_cursor_back(self);
#endif // !MRL_ECHO_OFF
    memmove(self->cmdline + self->cursor - 1,
            self->cmdline + self->cursor,
            self->cmdlen  - self->cursor + 1);
    self->cursor--;
    self->cmdlen--;
    mrl_terminal_print(self, self->cmdline + self->cursor);
#ifdef MRL_PRINT_ESC_OFF
    self->print(" ");
#endif // MRL_PRINT_ESC_OFF
    mrl_terminal_cursor(self, self->cursor);
  }
#ifdef MRL_ECHO_OFF
  else
  {
    mrl_terminal_cursor_forward(self);
    mrl_terminal_print(self, self->cmdline + self->cursor);
#ifdef MRL_PRINT_ESC_OFF
    self->print(" ");
#endif // MRL_PRINT_ESC_OFF
    mrl_terminal_cursor(self, self->cursor);
  }
#endif // MRL_ECHO_OFF
}
//-----------------------------------------------------------------------------
// DELETE pressed
INLINE void mrl_delete(mrl_t *self)
{
  if (self->cursor < self->cmdlen)
  {
    memmove(self->cmdline + self->cursor,
            self->cmdline + self->cursor + 1,
            self->cmdlen  - self->cursor + 1);
    self->cmdlen--;
    mrl_terminal_print(self, self->cmdline + self->cursor);
#ifdef MRL_PRINT_ESC_OFF
    self->print(" ");
#endif // MRL_PRINT_ESC_OFF
    mrl_terminal_cursor(self, self->cursor);
  }
}
//-----------------------------------------------------------------------------
// Alt+BACKSPACE or Ctrl+U presed
static void mrl_del_before_cursor(mrl_t *self)
{
#ifdef MRL_PRINT_ESC_OFF
  int i = self->cursor;
#endif // MRL_PRINT_ESC_OFF
  memmove(self->cmdline,
          self->cmdline + self->cursor,
          self->cmdlen - self->cursor + 1);
  self->cmdlen -= self->cursor;
  self->cursor = 0;
  mrl_terminal_cursor(self, 0);
  mrl_terminal_print(self, self->cmdline);
#ifdef MRL_PRINT_ESC_OFF
  while (i > 0)
  {
    self->print(" ");
    i--;
  }
#endif // MRL_PRINT_ESC_OFF
  mrl_terminal_cursor(self, self->cursor = 0);
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
// search history by UP/DOWN or Ctrl-P/Ctrl-N
static void mrl_hist(mrl_t *self, int dir)
{
  int len;
  len = mrl_hist_search(&self->hist, dir, self->cmdline);
  if (len >= 0)
  {
#ifdef MRL_PRINT_ESC_OFF
    int i = self->cmdlen;
    self->cursor = self->cmdlen = len;
    mrl_terminal_cursor(self, 0);
    while (i > 0)
    {
      self->print(" ");
      i--;
    }
#else
    self->cursor = self->cmdlen = len;
#endif // MRL_PRINT_ESC_OFF
    mrl_terminal_cursor(self, 0);
    mrl_terminal_print(self, self->cmdline);
  }
}
#endif // MRL_USE_HISTORY
//-----------------------------------------------------------------------------
// split cmdline to tkn array and return nmb of token
// replace all whitespaces to '\0'
static int mrl_split(char *str, int len, char **argv)
{
  int argc = 0, i = 0;
  while (1)
  {
    // skip whitespaces and replace them to '\0'
    while ((str[i] == ' ') && (i < len))
      str[i++] = '\0';

    if (i >= len) break;

    argv[argc++] = str + i;

    if (argc >= MRL_COMMAND_TOKEN_NUM) break;

    // skip NOT whitespaces
    while ((str[i] != ' ') && (i < len))
      i++;
    
    if (i >= len) break;
  }
  argv[argc] = (char*) NULL;
  return argc;
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_COMPLETE
// insert len char of text at cursor position
static void mrl_insert_text(mrl_t *self, const char *text, int len)
{
  if (len > MRL_COMMAND_LINE_LEN - 1 - self->cmdlen) 
    len = MRL_COMMAND_LINE_LEN - 1 - self->cmdlen;
  
  if (len > 0)
  {
    char *p1 = self->cmdline + self->cursor;
    char *p2 = p1 + len;
    memmove(p2, p1, self->cmdlen - self->cursor + 1);

    memcpy(p1, text, len);
    
    self->cmdlen += len;

    mrl_terminal_cursor(self, self->cursor);
    mrl_terminal_print(self, p1);
    
    self->cursor += len;
    
    mrl_terminal_cursor(self, self->cursor);
  }
#ifdef MRL_ECHO_OFF
  else
  {
    mrl_terminal_print(self, self->cmdline + self->cursor);
    mrl_terminal_cursor(self, self->cursor);
  }
#endif // MRL_ECHO_OFF
}
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
// insert one char to cursor position
static void mrl_insert_chr(mrl_t *self, char chr)
{
  if (self->cmdlen < MRL_COMMAND_LINE_LEN - 1) 
  {
    char *ptr = self->cmdline + self->cursor;
    memmove(ptr + 1, ptr, self->cmdlen - self->cursor + 1);
    *ptr = chr;

#ifdef MRL_ECHO_OFF
    if (++self->cursor < ++self->cmdlen)
    {
      mrl_terminal_cursor(self, self->cursor);
      mrl_terminal_print(self, ptr + 1);
      mrl_terminal_cursor(self, self->cursor);
    }
#else
    self->cmdlen++;
    mrl_terminal_cursor(self, self->cursor);
    mrl_terminal_print(self, ptr);
    self->cursor++;
    mrl_terminal_cursor(self, self->cursor);
#endif // MRL_ECHO_OFF
  }
#ifdef MRL_ECHO_OFF
  else
  {
    mrl_terminal_cursor_back(self);
    mrl_terminal_print(self, self->cmdline + self->cursor);
    mrl_terminal_print(self, " ");
    mrl_terminal_cursor(self, self->cursor);
  }
#endif // MRL_ECHO_OFF
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_COMPLETE
// back replace '\0' to whitespaces
INLINE void mrl_back_replace_spaces(char *str, int len)
{
  int i;
  for (i = 0; i < len; i++)
    if (str[i] == '\0')
      str[i] = ' ';
}
//-----------------------------------------------------------------------------
static int mrl_common_len(const char * const argv[])
{
  int i, j;
  const char *shortest = argv[0];
  int shortlen = strlen(shortest);

  for (i = 1; argv[i] != NULL; i++)
  {
    int len = strlen(argv[i]);
    if (shortlen > len)
    {
      shortest = argv[i];
      shortlen = len;
    }
  }

  for (i = 0; i < shortlen; i++)
    for (j = 0; argv[j] != NULL; j++)
      if (shortest[i] != argv[j][i])
        return i;

  return shortlen;
}
//-----------------------------------------------------------------------------
// TAB pressed
INLINE void mrl_get_complite(mrl_t *self)
{
  int argc;
  char *argv[MRL_COMMAND_TOKEN_NUM + 2];
  const char **compl_argv;
  
  if (self->get_completion == NULL) return; // callback was not set
  
  argc = mrl_split(self->cmdline, self->cursor, argv);

  if (self->cmdline[self->cursor - 1] == '\0')
  { // last char WAS whitespace
    argv[argc++] = "";
    argv[argc]   = NULL;
  }

  compl_argv = self->get_completion(argc, argv);

  mrl_back_replace_spaces(self->cmdline, self->cursor);

  if (compl_argv[0] != NULL)
  {
    int len = 0;

    if (compl_argv[1] == NULL)
    { // only one variant
      len = strlen(compl_argv[0]);
    }
    else
    { // some variants
      int i = 0;
#ifdef MRL_COMPLETE_COLS
      int num = 0;
#endif
      mrl_terminal_newline(self);
      len = mrl_common_len(compl_argv);
      while (compl_argv[i] != NULL)
      {
#ifdef MRL_COMPLETE_COLS
        if (++num > MRL_COMPLETE_COLS)
        {
          num = 1;
          mrl_terminal_newline(self);
        }
#endif
        self->print(compl_argv[i]);
        self->print(" ");
        i++;
      }
      mrl_terminal_newline(self);
      mrl_terminal_prompt(self);
    }
    
    if (len)
    { // insert completion
      mrl_insert_text(self, compl_argv[0] + strlen(argv[argc - 1]), 
                      len - strlen(argv[argc - 1]));

      if (compl_argv[1] == NULL &&            // only one variant and
          self->cmdline[self->cursor] != ' ') // no space at cursor
        mrl_insert_text(self, " ", 1);        // => append space
    }

    mrl_terminal_cursor(self, 0);
    mrl_terminal_print(self, self->cmdline);
    mrl_terminal_cursor(self, self->cursor);
  } 
#ifdef MRL_ECHO_OFF
  else
  {
    mrl_terminal_cursor(self, self->cursor);
    mrl_terminal_print(self, self->cmdline + self->cursor);
    mrl_terminal_cursor(self, self->cursor);
  }
#endif // MRL_ECHO_OFF
}
#endif // MRL_USE_COMPLETE
//-----------------------------------------------------------------------------
static void mrl_new_line_handler(mrl_t *self)
{
  int argc;
  char *argv[MRL_COMMAND_TOKEN_NUM + 1];

#ifdef MRL_USE_HISTORY
  mrl_hist_save(&self->hist, self->cmdline);

#ifdef MRL_DEBUG_HISTORY
  mrl_cmd_print(self);
  mrl_hist_print(&self->hist);
#endif // MRL_DEBUG_HISTORY
#endif // MRL_USE_HISTORY

  mrl_terminal_newline(self);
  
  argc = mrl_split(self->cmdline, self->cmdlen, argv);

  if (argc > 0 && self->execute != NULL) self->execute(argc, argv);
  
  self->cmdline[0] = '\0';
  self->cmdlen = 0;
  self->cursor = 0;
  mrl_terminal_prompt(self);
}
//-----------------------------------------------------------------------------
void mrl_init(mrl_t *self, void (*print)(const char*))
{
#ifdef MRL_USE_HISTORY
  mrl_hist_init(&self->hist);
#endif

#ifdef MRL_USE_ESC_SEQ
  self->escape_seq = MRL_ESC_STOP;
#endif

#if (defined(MRL_ENDL_CRLF) || defined(MRL_ENDL_LFCR))
  self->tmpch = '\0';
#endif
  
  self->prompt     = mrl_prompt_default;
  self->prompt_len = mrl_prompt_default_len;

#ifdef MRL_DEBUG_HISTORY
  memset(self->cmdline, '~', MRL_COMMAND_LINE_LEN);
#endif

  self->cmdline[0] = '\0';
  self->cmdlen = 0;
  self->cursor = 0;

  self->print = print;
  self->execute = NULL;
#ifdef MRL_USE_COMPLETE
  self->get_completion = NULL;
#endif

#ifdef MRL_USE_CTLR_C
  self->sigint = NULL;
#endif

#ifdef MRL_ENABLE_INIT_ROMPT
#ifndef MRL_PRINT_ESC_OFF
  self->print("\r\033[K"); // erase all string and go to begin
#else
  self->print("\r"); // start from begin of line
#endif // MRL_PRINT_ESC_OFF
  mrl_terminal_prompt(self);
#endif
}
//-----------------------------------------------------------------------------
void mrl_clear(mrl_t *self)
{
#ifndef MRL_PRINT_ESC_OFF
  self->print("\r\033[K");
#else
  self->print("\r"); // FIXME
#endif // MRL_PRINT_ESC_OFF
}
//-----------------------------------------------------------------------------
void mrl_prompt(mrl_t *self)
{
  self->cmdline[0] = '\0';
  self->cmdlen = 0;
  self->cursor = 0;
  mrl_clear(self);
  mrl_terminal_prompt(self);
}
//-----------------------------------------------------------------------------
void mrl_refresh(mrl_t *self)
{
  mrl_clear(self);
  mrl_terminal_prompt(self);
  mrl_terminal_print(self, self->cmdline);
  mrl_terminal_cursor(self, self->cursor);
}
//-----------------------------------------------------------------------------
#ifdef MRL_USE_ESC_SEQ
// handling escape sequences
static void mrl_escape_process(mrl_t *self, char ch)
{
  if (ch == '[')
    self->escape_seq = MRL_ESC_BRACKET;
  else if (ch == MRL_KEY_DEL)
  { // Alt+BACKSPACE
    mrl_del_before_cursor(self);
    self->escape_seq = MRL_ESC_STOP;
  }
  else if (self->escape_seq == MRL_ESC_BRACKET)
  {
    if (ch == 'A')
    { // cursor UP
#ifdef MRL_USE_HISTORY
#  ifdef MRL_ECHO_OFF
      self->print("\n");
#  endif // MRL_ECHO_OFF
      mrl_hist(self, MRL_HIST_BACKWARD);
#endif // MRL_USE_HISTORY 
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (ch == 'B')
    { // cursor DOWN
#ifdef MRL_USE_HISTORY
      mrl_hist(self, MRL_HIST_FORWARD);
#endif // MRL_USE_HISTORY 
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (ch == 'C')
    { // cursor RIGHT
      mrl_cursor_forward(self);
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (ch == 'D')
    { // cursor LEFT
      mrl_cursor_back(self);
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (ch == 'H')
    { // HOME
      mrl_cursor_home(self);
      self->escape_seq = MRL_ESC_STOP;
    } 
    else if (ch == 'F')
    { // END
#ifdef MRL_ECHO_OFF
      mrl_terminal_cursor_back(self);
      mrl_terminal_print(self, self->cmdline + self->cursor);
#endif // MRL_ECHO_OFF
      mrl_cursor_end(self);
      self->escape_seq = MRL_ESC_STOP;
    } 
    else if (ch == '7' || ch == '1')
      self->escape_seq = MRL_ESC_HOME;
    else if (ch == '8')
      self->escape_seq = MRL_ESC_END;
    else if (ch == '3')
      self->escape_seq = MRL_ESC_DELETE;
    else
      self->escape_seq = MRL_ESC_STOP; // unknown escape sequence, stop
  }
  else if (ch == '~')
  {
    if (self->escape_seq == MRL_ESC_HOME)
    { // HOME
      mrl_cursor_home(self);
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (self->escape_seq == MRL_ESC_END)
    { // END
#ifdef MRL_ECHO_OFF
      mrl_terminal_cursor_back(self);
      mrl_terminal_print(self, self->cmdline + self->cursor);
#endif // MRL_ECHO_OFF
      mrl_cursor_end(self);
      self->escape_seq = MRL_ESC_STOP;
    }
    else if (self->escape_seq == MRL_ESC_DELETE)
    { // DELETE
      mrl_delete(self);
      self->escape_seq = MRL_ESC_STOP;
    }
  }
  else if (ch == 'O')
    self->escape_seq = MRL_ESC_O;
  else if (self->escape_seq == MRL_ESC_O && ch == 'F')
  { // END
#ifdef MRL_ECHO_OFF
    mrl_terminal_cursor_back(self);
    mrl_terminal_print(self, self->cmdline + self->cursor);
#endif // MRL_ECHO_OFF
    mrl_cursor_end(self);
    self->escape_seq = MRL_ESC_STOP;
  }
  else
    self->escape_seq = MRL_ESC_STOP; // unknown escape sequence, stop
}
#endif // MRL_USE_ESC_SEQ
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
#elif defined(MRL_ENDL_LF)
    case MRL_KEY_CR:
      break;

    case MRL_KEY_LF:
      mrl_new_line_handler(self);
      break;
#else // defined(MRL_ENDL_CR_LF)
    case MRL_KEY_CR:
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

    case MRL_KEY_NAK: // Ctrl+U
      mrl_del_before_cursor(self);
      break;

    case MRL_KEY_VT:  // Ctrl+K
#ifdef MRL_ECHO_OFF
      self->print("\n");
#endif // MRL_ECHO_OFF
#ifndef MRL_PRINT_ESC_OFF
      self->print("\033[K");
      self->cmdlen = self->cursor;
      self->cmdline[self->cmdlen] = '\0';
#else
      mrl_terminal_newline(self);
      mrl_terminal_prompt(self);
      self->cmdlen = self->cursor;
      self->cmdline[self->cmdlen] = '\0';
      mrl_terminal_print(self, self->cmdline);
      mrl_terminal_cursor(self, self->cursor);
      break;
#endif // MRL_PRINT_ESC_OFF
      break;

    case MRL_KEY_BEL: // Ctrl+G
#ifndef MRL_PRINT_ESC_OFF
      self->print("\r");
      self->print("\033[K");
#else
      mrl_terminal_newline(self);
#endif // MRL_PRINT_ESC_OFF
      mrl_terminal_prompt(self);
      self->cmdlen = self->cursor = 0;
      self->cmdline[0] = '\0';
      break;

    case MRL_KEY_ENQ: // Ctrl+E
      mrl_cursor_end(self);
      break;

    case MRL_KEY_SOH: // Ctrl+A
      mrl_cursor_home(self);
      break;

    case MRL_KEY_ACK: // Ctrl+F
      mrl_cursor_forward(self);
      break;

    case MRL_KEY_STX: // Ctrl+B
      mrl_cursor_back(self);
      break;

    case MRL_KEY_DLE: // Ctrl+P
#ifdef MRL_USE_HISTORY
      mrl_hist(self, MRL_HIST_BACKWARD);
#endif
      break;

    case MRL_KEY_SO: // Ctrl+N
#ifdef MRL_USE_HISTORY
      mrl_hist(self, MRL_HIST_FORWARD);
#endif
      break;

    case MRL_KEY_DEL: // BACKSPACE
    case MRL_KEY_BS:  // CTLR+H
      mrl_backspace(self);
      break;

    case MRL_KEY_DC2: // Ctrl+R
      mrl_terminal_newline(self);
      mrl_terminal_prompt(self);
      //mrl_terminal_cursor(self, 0);
      mrl_terminal_print(self, self->cmdline);
      mrl_terminal_cursor(self, self->cursor);
      break;

    case MRL_KEY_ETX: // Ctrl+C
#ifdef MRL_USE_CTRL_C
      if (self->sigint != NULL) self->sigint();
#endif // MRL_USE_CTRL_C
      return MRL_KEY_ETX;

    case MRL_KEY_FF: // Ctrl+L
#ifndef MRL_PRINT_ESC_OFF
      self->print("\033[2J"  // ESC seq for clear entire screen
                  "\033[H"); // ESC seq for move cursor at left-top corner
#else
      self->print(MRL_ENDL); // go to new line only
#endif // MRL_PRINT_ESC_OFF
      mrl_terminal_prompt(self);
      mrl_terminal_print(self, self->cmdline);
      mrl_terminal_cursor(self, self->cursor);
      break;
    
    case MRL_KEY_EOT: // Ctrl+D
    case MRL_KEY_DC3: // Ctrl+S
    case MRL_KEY_DC4: // Ctrl+T
    case MRL_KEY_CAN: // Ctrl+X
    case MRL_KEY_EM:  // Ctrl+Y
    case MRL_KEY_SUB: // Ctrl+Z
    case MRL_KEY_SYN: // Ctrl+V
    case MRL_KEY_ETB: // Ctrl+W
    case MRL_KEY_DC1: // Ctrl+Q
    case MRL_KEY_SI:  // Ctrl+O
      return ch;

    default:
      if (!MRL_IS_CONTROL_CHAR(ch))
        mrl_insert_chr(self, ch);
      break;
  } // switch (ch)
  
  return 0;
}
//----------------------------------------------------------------------------
#if defined(MRL_UINT2STR) || defined(MRL_INT2STR) || !defined(MRL_USE_LIBC_STDIO)
int mrl_uint2str(unsigned value, char *buf)
{
  int i = 0, j = 0, n = 0;
  do {
    buf[i++] = (value % 10) + '0';
    value /= 10;
  } while (value);
  buf[n = i--] = '\0';
  while (j < i)
  { // exchange bytes
    char ch  = buf[j];
    buf[j++] = buf[i];
    buf[i--] = ch;
  }
  return n;
}
#endif // MRL_UINT2STR || MRL_INT2STR || !MRL_USE_LIBC_STDIO
//----------------------------------------------------------------------------
#ifdef MRL_INT2STR
int mrl_int2str(int value, char *buf)
{
  int n = 0;
  if (value < 0)
  {
    value = -value;
    *buf++ = '-';
    n++;
  }
  return n + mrl_uint2str((unsigned) value, buf);
}
#endif // MRL_INT2STR
//----------------------------------------------------------------------------
#ifdef MRL_STR2INT
int mrl_str2int(const char *str, int def_val, unsigned char base)
{
  unsigned char c, sign = 0;
  const unsigned char *p = (const unsigned char*) str;
  int retv = 0;
  
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
    { // 0xHHHH - HEX
      base = 16;
      p++;
    }
    else if (*p == 'b' || *p == 'B')
    { // 0bBBBB - BIN
      base = 2;
      p++;
    }
    else if (*p == 'o' || *p == 'O')
    { // 0oOOOO - OCT
      base = 8;
      p++;
    }
  }

  if (*p < '0') return def_val;
  if (base == 0) base = 10;

  while (1)
  {
    c = *p++;
    if (c == '\'' || c == '"' || c == '`' || c == '_') continue;
    if (c < '0') break;
    if (base > 10)
    {
      if      (c >= 'A' && c <= 'Z') c -= 'A' - '9' - 1;
      else if (c >= 'a' && c <= 'z') c -= 'a' - '9' - 1;
    }
    c -= '0';
    if (c >= base) return def_val; // illegal char
    retv *= (int) base;
    retv += (int) c;
  }

  return (sign == 0) ? retv : -retv;
}
#endif // MRL_STR2INT
//-----------------------------------------------------------------------------

/* end of "mrl.c" file ***/

