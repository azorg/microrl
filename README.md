MicroRL - read line library for small/embedded devices with basic VT100 support
===============================================================================
This library forked from https://github.com/Helius/microrl
and last version pulled to https://github.com/azorg/microrl

## 1. Description

MicroRL library is designed to help implement command line interface in small
and embedded devices. Main goal is to write compact, small memory consuming
but powerful interfaces, with support navigation through command line with
cursor, HOME, END, DEL, BACKSPACE keys, hot key like Ctrl+U and other,
history and completion feature.

## 2. Feature

	** config.h file
	 - Turn on/off feature for add functional/decrease memory via config files.

	** hot keys support
	 - BACKSPACE, cursor arrow, HOME, END and DELETE keys
	 - Ctrl+U (like Alt+BACKSPACE - cut line from cursor to begin) 
	 - Ctrl+K (cut line from cursor to end) 
	 - Ctrl+A (like HOME) 
	 - Ctrl+E (like END)
	 - Ctrl+H (like BACKSPACE)
	 - Ctrl+B (like cursor arrow left) 
	 - Ctrl+F (like cursor arrow right)
	 - Ctrl+P (like cursor arrow up)
	 - Ctrl+N (like cursor arrow down)
	 - Ctrl+R (retype prompt and partial command)
	 - Ctrl+C (call 'sigint' callback, only for embedded system)
	 - Ctrl+L (clear terminal window)
	 - Ctrl+G (clear all line)
	 - Ctrl+D, Ctrl+S, Ctr+V, Ctrl+X, Ctrl+Y, Ctrl+Z, Ctrl+Q, Ctrl+W, Ctrl+T (return non zero key code)

	** history
	 - Static ring buffer history for memory saving. Number of commands
     saved to history depends from commands length and buffer size
     (defined in config)

	** completion
	 - via completion callback

## 3. Source code structure

```
mrl.h          - external MicroRL interface and data types
mrl_conf.h     - customization MicroRL config file
mrl_defs.h     - internal MicroRL defines
mrl.c          - source code of MicroRL routines
test/            - library usage example and tests for GNU/Linux
  Makefile.skel  - helper Makefile (included from Makefile's)
  test1/  
    mrl_test1.c    - source code of example and test #1
    Makefile       - Makefile for build test #1
  test2/  
    mrl_test2.c    - source code of example and test #2
    Makefile       - Makefile for build test #1
  test3/
    mrl_test3.c    - source code of example and test #3
    Makefile       - Makefile for build test #1
```

## 4. Install

Requirements: C compiler with support for C99 standard with standard C library
(libc, uClibc, newlib or other compatible). Also you have to implement several
routines in your own code for library to work. 

For embed lib to you project, you need to do few simple steps:

* a) Include "mrl.h" file to you project.

* b) Create `mrl_t` object, and call `mrl_init()` func, with print
     callback pointer. Print callback pointer is pointer to function that
     call by library if it's need to put text to terminal. Text string
     always is null terminated.

For example on Linux PC print callback may be:
```
// print callback for MicroRL library
void print(const char *str)
{
  fprintf(stdout, "%s", str);
}
```

* c) Call `mrl_set_execute_cb()` with pointer to you routine, what will be
     called if user press enter in terminal. Execute callback give a 'argc',
     'argv' parametrs, like 'main' func in application. All token in 'argv'
     is null terminated. So you can simply walk through argv and handle
     commands.

* d) If you want completion support if user press TAB key, call
     `mrl_set_complete_cb()` and set you callback. It also give 'argc' and
     'argv' arguments, so iterate through it and return set of complete
     variants. 

* e) Look at `mrl_conf.h` file, for tune library for you requiring.

* f) Now you just call `mrl_insert_char()` on each char received from
     input stream (USART, network, etc).

Example of code:
```
//-----------------------------------------------------------------------------
int main(int argc, char ** argv)
{
  mrl_t mrl; // MicroRL object
  
  // call init with print callback
  mrl_init(&mrl, print);

  // set callback for execute
  mrl_set_execute_cb(&mrl, execute);

  // set callback for completion (optionally)
  mrl_set_complete_cb(&mrl, complet);

  // set callback for Ctrl+C handling (optionally)
  mrl_set_sigint_cb(&mrl, sigint);
  
  while (1)
  {
    // put received char from stdin to MicroRL lib
    char ch = get_char();
    int rv = mrl_insert_char(&mrl, ch);
    if (rv) break; // exit if CTRL+D pressed
  }

  return 0;
}
//-----------------------------------------------------------------------------
```
See example library usage in test folder.


## 5. MicroRL fork tasks ([+]FIXED, [-]TODO and [~]WORK)

 * [+] add DELETE key support

 * [+] add Crtl+D key

 * [+] add Alt+BACKSPACE key

 * [+] replace `u16bit_to_str()` to `mrl_uint2str()` - more traditional API

 * [+] add `mrl_str2int()` function as atoi() alternative

 * [+] new unlimit (>256 bytes) history ring buffer

 * [+] fix reset cmdline by cursor UP/Ctrl+P (copy cmdline to history befor)

 * [~] refactor sources, fix old bugs and add new :-)

 * [+] add `MRL_ENDL_CR_LF` config (one CR or one LF - in, CR+LF - out)

 * [-] auto detection end of line (CR/LF/CR+LF/LF+CR) 

 * [+] add `mrl_test2`
 
 * [+] add some escape key codes

 * [+] return some key codes (Ctr+Q/S/W/Z/X/Y/V)

## 6. Notes

 * for using ANSI colors start `minicom` with `-c on` option

 * I use MicroRL in my STM32 projects

## 7. License

Licensed under the Apache License, Version 2.0 (see "LICENSE" and "NOTICE").

