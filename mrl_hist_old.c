/*
 * MicroRL library
 * File "mrl_hist.c"
 */

//-----------------------------------------------------------------------------
#include "mrl_hist.h"
#include "mrl_defs.h"
//-----------------------------------------------------------------------------
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
#ifdef MRL_DEBUG_HISTORY
#include <stdio.h>
//-----------------------------------------------------------------------------
// print buffer content on screen
static void mrl_hist_print(mrl_hist_t *self)
{
  int i;
  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
    printf("%c", i == self->begin ? 'b' : ' ');

  printf(MRL_ENDL);
  for (i = 0; i < MRL_RING_HISTORY_LEN; i++)
  {
    if (self->buf[i] >= ' ')
      printf("%c", self->buf[i]);
    else if (self->buf[i] <= 9)
      printf("%d", self->buf[i]);
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
static void mrl_hist_erase_older(mrl_hist_t *self)
{
  int new_pos = self->begin + self->buf[self->begin] + 1;
  if (new_pos >= MRL_RING_HISTORY_LEN)
    new_pos -= MRL_RING_HISTORY_LEN;
  self->begin = new_pos;
}
//-----------------------------------------------------------------------------
// check space for new line, remove older while not space
static bool mrl_hist_is_space_for_new(mrl_hist_t *self, int len)
{
  if (self->buf[self->begin] == 0)
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
void mrl_hist_init(mrl_hist_t *self)
{
  memset(self->buf, 0, MRL_RING_HISTORY_LEN);
  self->begin = 0;
  self->end = 0;
  self->cur = 0;
}
//-----------------------------------------------------------------------------
// put line to ring buffer
void mrl_hist_save(mrl_hist_t *self, const char *line, int len)
{
  if (len > MRL_RING_HISTORY_LEN - 2)
    return;

  while (!mrl_hist_is_space_for_new(self, len))
    mrl_hist_erase_older(self);
  
  // if it's first line
  if (self->buf[self->begin] == 0) 
    self->buf[self->begin] = len;
  
  // store line
  if (len < MRL_RING_HISTORY_LEN - self->end - 1)
    memcpy(self->buf + self->end + 1, line, len);
  else
  {
    int part_len = MRL_RING_HISTORY_LEN - self->end - 1;
    memcpy(self->buf + self->end + 1, line, part_len);
    memcpy(self->buf, line + part_len, len - part_len);
  }

  self->buf[self->end] = len;
  self->end = self->end + len + 1;

  if (self->end >= MRL_RING_HISTORY_LEN)
    self->end -= MRL_RING_HISTORY_LEN;

  self->buf[self->end] = 0;
  self->cur = 0;

#ifdef MRL_DEBUG_HISTORY
  mrl_hist_print(self);
#endif // MRL_DEBUG_HISTORY
}
//-----------------------------------------------------------------------------
// copy saved line to 'line' and return size of line
int mrl_hist_restore(mrl_hist_t *self, char *line, int dir)
{
  int j, cnt = 0; // count history record  
  int header = self->begin;
  while (self->buf[header])
  {
    header += self->buf[header] + 1;
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
      while ((self->buf[header]) && (cnt - j - 1 != self->cur))
      {
        header += self->buf [header] + 1;
        if (header >= MRL_RING_HISTORY_LEN)
          header -= MRL_RING_HISTORY_LEN;
        j++;
      }

      if (self->buf[header])
      {
        self->cur++; // obtain saved line
        if (self->buf[header] + header < MRL_RING_HISTORY_LEN)
        {
          memset(line, 0, MRL_COMMAND_LINE_LEN);
          memcpy(line, self->buf + header + 1, self->buf[header]);
        }
        else
        {
          int part0 = MRL_RING_HISTORY_LEN - header - 1;
          memset(line, 0, MRL_COMMAND_LINE_LEN);
          memcpy(line, self->buf + header + 1, part0);
          memcpy(line + part0, self->buf, self->buf[header] - part0);
        }
        return self->buf[header];
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

      while ((self->buf[header] != 0) && (cnt - j != self->cur))
      {
        header += self->buf[header] + 1;
        if (header >= MRL_RING_HISTORY_LEN)
          header -= MRL_RING_HISTORY_LEN;
        j++;
      }

      if (self->buf[header] + header < MRL_RING_HISTORY_LEN)
      {
        memcpy(line, self->buf + header + 1, self->buf[header]);
      }
      else
      {
        int part0 = MRL_RING_HISTORY_LEN - header - 1;
        memcpy(line, self->buf + header + 1, part0);
        memcpy(line + part0, self->buf, self->buf[header] - part0);
      }
      return self->buf[header];
    }
    else
    { // empty line
      return 0;
    }
  }
  return -1;
}
//-----------------------------------------------------------------------------
#endif // MRL_USE_HISTORY

/*** end of "mrl_hist.c" file ***/

