/*
 * MicroRL library (internal defines)
 * File "mrl_defs.h"
 */

#pragma once
#ifndef MRL_DEFS_H
#define MRL_DEFS_H
//-----------------------------------------------------------------------------
// key codes
#define MRL_KEY_NUL   0 // ^@ Null character
#define MRL_KEY_SOH   1 // ^A Start of heading, = console interrupt
#define MRL_KEY_STX   2 // ^B Start of text, maintenance mode on HP console
#define MRL_KEY_ETX   3 // ^C End of text
#define MRL_KEY_EOT   4 // ^D End of transmission, not the same as ETB
#define MRL_KEY_ENQ   5 // ^E Enquiry, goes with ACK; old HP flow control
#define MRL_KEY_ACK   6 // ^F Acknowledge, clears ENQ logon hand
#define MRL_KEY_BEL   7 // ^G Bell, rings the bell...
#define MRL_KEY_BS    8 // ^H Backspace, works on HP terminals/computers
#define MRL_KEY_HT    9 // ^I Horizontal tab, move to next tab stop
#define MRL_KEY_LF   10 // ^J Line Feed
#define MRL_KEY_VT   11 // ^K Vertical tab
#define MRL_KEY_FF   12 // ^L Form Feed, page eject
#define MRL_KEY_CR   13 // ^M Carriage Return
#define MRL_KEY_SO   14 // ^N Shift Out, alternate character set
#define MRL_KEY_SI   15 // ^O Shift In, resume defaultn character set
#define MRL_KEY_DLE  16 // ^P Data link escape
#define MRL_KEY_DC1  17 // ^Q XON, with XOFF to pause listings; "OK to send".
#define MRL_KEY_DC2  18 // ^R Device control 2, block-mode flow control
#define MRL_KEY_DC3  19 // ^S XOFF, with XON is TERM=18 flow control
#define MRL_KEY_DC4  20 // ^T Device control 4
#define MRL_KEY_NAK  21 // ^U Negative acknowledge
#define MRL_KEY_SYN  22 // ^V Synchronous idle
#define MRL_KEY_ETB  23 // ^W End transmission block, not the same as EOT
#define MRL_KEY_CAN  24 // ^X Cancel line, MPE echoes !!!
#define MRL_KEY_EM   25 // ^Y End of medium, Control-Y interrupt
#define MRL_KEY_SUB  26 // ^Z Substitute
#define MRL_KEY_ESC  27 // ^[ Escape, next character is not echoed
#define MRL_KEY_FS   28 // ^\ File separator
#define MRL_KEY_GS   29 // ^] Group separator
#define MRL_KEY_RS   30 // ^^ Record separator, block-mode terminator
#define MRL_KEY_US   31 // ^_ Unit separator
#define MRL_KEY_DEL 127 // BACKSPACE (not a real control character)
//-----------------------------------------------------------------------------
#define MRL_IS_CONTROL_CHAR(chr) ((chr) <= 31)
//-----------------------------------------------------------------------------
// direction of history navigation
#define MRL_HIST_BACKWARD 0
#define MRL_HIST_FORWARD  1
//-----------------------------------------------------------------------------
// ESC seq internal codes
#define MRL_ESC_STOP    0
#define MRL_ESC_START   1
#define MRL_ESC_BRACKET 2
#define MRL_ESC_O       3
#define MRL_ESC_HOME    4
#define MRL_ESC_END     5
#define MRL_ESC_DELETE  6
//-----------------------------------------------------------------------------
#endif // MRL_DEFS_H

/*** end of "mrl_defs.h" file ***/


