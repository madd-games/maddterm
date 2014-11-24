/*
	Copyright (c) 2014, Madd Games.
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	
	* Redistributions of source code must retain the above copyright notice, this
	  list of conditions and the following disclaimer.
	
	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LIBMADDTERM_H
#define LIBMADDTERM_H

#include <inttypes.h>
#include <sys/types.h>

typedef struct
{
	/**
	 * Size of the terminal.
	 */
	int width, height;
	
	/**
	 * Points to the terminal matrix.
	 */
	uint8_t *matrix;
	
	/**
	 * Child callback function. This function will be called from inside the child process
	 * which is controlled by this terminal. It should probably use one of the exec*() functions
	 * to load an executable such as the shell.
	 */
	void (*start)(void);
} MTPARAMS;

typedef struct
{
	int width, height;
	uint8_t *matrix;
	int fd;			// terminal FD
	pid_t pid;		// child PID
	int curX, curY;
	uint8_t curColor;	// current bg/fg.
	uint32_t attr;		// attributes.
} MTCONTEXT;

/**
 * Create a maddterm context, i.e. an isntance of a terminal, using the specifies params structure.
 * You may delete the params structure after a call to this function.
 */
MTCONTEXT *mtCreateContext(MTPARAMS *params);

/**
 * Clear the terminal, initialising it to the default state.
 */
void mtClear(MTCONTEXT *ctx);

/**
 * Put a character in the terminal.
 */
void mtPutChar(MTCONTEXT *ctx, char c);

#endif