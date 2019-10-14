MicroRL - read line library for small/embedded devices with basic VT100 support
===============================================================================
This library forked from https://github.com/Helius/microrl

## 1. Description

Microrl library is designed to help implement command line interfacev in small
and embedded devices. Main goal is to write compact, small memory consuming
but powerful interfaces, with support navigation through command line with
cursor, HOME, END, DEL, BACKSPACE keys, hot key like Ctrl+U and other,
history and completion feature.

## 2. Feature

	** config.h file
	 - Turn on/off feature for add functional/decrease memory via config files.

	** hot keys support
	 - backspace, cursor arrow, HOME, END and DELETE keys
	 - Ctrl+U (cut line from cursor to begin) 
	 - Ctrl+K (cut line from cursor to end) 
	 - Ctrl+A (like HOME) 
	 - Ctrl+E (like END)
	 - Ctrl+H (like backspace)
	 - Ctrl+B (like cursor arrow left) 
	 - Ctrl+F (like cursor arrow right)
	 - Ctrl+P (like cursor arrow up)
	 - Ctrl+N (like cursor arrow down)
	 - Ctrl+R (retype prompt and partial command)
	 - Ctrl+C (call 'sigint' callback, only for embedded system)

	** history
	 - Static ring buffer history for memory saving. Number of commands
     saved to history depends from commands length and buffer size
     (defined in config)

	** completion
	 - via completion callback

## 3. SRC structure

```
src/             - library source
  mrl.h          - external lib interface and data types
  mrl_conf.h     - customisation config file
  mrl_defs.h     - internal defines
  mrl.c          - microrl routines
test/            - library usage example and test for GNU/Linux
  mrl_test.c     - source code of example and test
  Makefile       - Makefile for build test
  Makefile.kel   - helper Makefile (included from Makefile)
```

## 4. Install

Requirements: C compiler with support for C99 standard with standard C library
(libc, uClibc, newlib or other compatible). Also you have to implement several
routines in your own code for library to work. 

NOTE: need add -std=gnu99 arg for gcc (FIXME: why?)

For embed lib to you project, you need to do few simple steps:

* a) Include microrl.h file to you project.

* b) Create 'mrl_t' object, and call 'mrl_init' func, with print
     callback pointer. Print callback pointer is pointer to function that
     call by library if it's need to put text to terminal. Text string
     always is null terminated.

For example on linux PC print callback may be:
```
// print callback for MicroRL library
void print(const char *str)
{
  fprintf(stdout, "%s", str);
}
```

* c) Call 'mrl_set_execute_cb' with pointer to you routine, what will be
     called if user press enter in terminal. Execute callback give a 'argc',
     'argv' parametrs, like 'main' func in application. All token in 'argv'
     is null terminated. So you can simply walk through argv and handle
     commands.

* d) If you want completion support if user press TAB key, call
     'mrl_set_complete_cb' and set you callback. It also give 'argc' and
     'argv' arguments, so iterate through it and return set of complete
     variants. 

* e) Look at 'mrl_conf.h' file, for tune library for you requiring.

* f) Now you just call 'mrl_insert_char' on each char received from
     input stream (USART, network, etc).

Example of code:
```
//-----------------------------------------------------------------------------
int main(int argc, char ** argv)
{
	microrl_t mrl; // MicroRL object
	
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
    if (rv) break; // exit if CTRL+C pressed
	}

	return 0;
}
//-----------------------------------------------------------------------------
```
See example library usage in test folder.

## 5. License

Licensed under the Apache License, Version 2.0 (see "LICENSE" and "NOTICE").

