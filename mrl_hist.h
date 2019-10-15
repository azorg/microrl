/*
 * MicroRL library
 * File "mrl_hist.h"
 */

#ifndef MRL_HIST_H
#define MRL_HIST_H
//-----------------------------------------------------------------------------
#include "mrl_conf.h"
//-----------------------------------------------------------------------------
#ifdef MRL_USE_HISTORY
// history struct, contain internal variable
// history store in static ring buffer for memory saving
typedef struct {
  char buf[MRL_RING_HISTORY_LEN]; // ring buffer
  int begin; // begin index of olderst line
  int end;   // end index (index of future line) 
  int cur;
} mrl_hist_t;
#endif // MRL_USE_HISTORY
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
//-----------------------------------------------------------------------------
// init history structure
void mrl_hist_init(mrl_hist_t *self);
//-----------------------------------------------------------------------------
// put line to ring buffer
void mrl_hist_save_line(mrl_hist_t *self, char *line, int len);
//-----------------------------------------------------------------------------
// copy saved line to 'line' and return size of line
int mrl_hist_restore_line(mrl_hist_t *self, char *line, int dir);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus
//-----------------------------------------------------------------------------
#endif // MRL_HIST_H

/*** end of "mrl_hist.h" file ***/

