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

#include "libmaddterm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __unix__
#include <pty.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <sys/wait.h>
#include <signal.h>
#endif

MTCONTEXT *mtCreateContext(MTPARAMS *params)
{
	MTCONTEXT *ctx = (MTCONTEXT*) malloc(sizeof(MTCONTEXT));
	ctx->width = params->width;
	ctx->height = params->height;
	ctx->matrix = params->matrix;
	ctx->fd = -1;

	if (params->start != NULL)
	{
#ifdef __unix__
		struct winsize winsz = {.ws_xpixel = params->width, .ws_ypixel = params->height};
		ctx->pid = forkpty(&ctx->fd, NULL, NULL, &winsz);
	
		if (ctx->pid == 0)
		{
			// we don't need the terminal structure on the child.
			free(ctx);

			signal(SIGINT, SIG_DFL);
			setenv("TERM", "xterm", 1);

			params->start();
			exit(0);
		};
#else
	free(ctx);
	return NULL;
#endif
	};
	
	mtClear(ctx);
	
	return ctx;
};

void mtClear(MTCONTEXT *ctx)
{
	ctx->curX = 0;
	ctx->curY = 0;
	ctx->curColor = 0x07;
	ctx->attr = 0;
	ctx->ctllen = 0;
	
	int i;
	for (i=0; i<ctx->width * ctx->height; i++)
	{
		ctx->matrix[i*2+0] = 0;
		ctx->matrix[i*2+1] = 0x07;
	};
};

static void mtScrollPRIV(MTCONTEXT *ctx)
{
	int i;
	for (i=0; i<2*ctx->width*(ctx->height-1); i++)
	{
		ctx->matrix[i] = ctx->matrix[i+2*ctx->width];
	};
	
	for (i=ctx->width*(ctx->height-1); i<ctx->width*ctx->height; i++)
	{
		ctx->matrix[2*i+0] = 0;
		ctx->matrix[2*i+1] = ctx->curColor;
	};
};

void mtPutChar(MTCONTEXT *ctx, char c)
{
	int index = ctx->curY * ctx->width + ctx->curX;
	if (index >= (ctx->width * ctx->height))
	{
		fprintf(stderr, "[maddterm] out of bound mtPutChar(): (%d, %d)", ctx->curX, ctx->curY);
		exit(1);
	};

	ctx->matrix[2*index+0] = c;
	ctx->matrix[2*index+1] = ctx->curColor;
	
	ctx->curX++;
	if (ctx->curX == ctx->width)
	{
		ctx->curX = 0;
		ctx->curY++;
		
		if (ctx->curY == ctx->height)
		{
			ctx->curY--;
			mtScrollPRIV(ctx);
		};
	};
};

void mtWrite(MTCONTEXT *ctx, const char *data, size_t size)
{
	while (size--)
	{
		char c = *data++;
		
		if (c == 0)
		{
			continue;
		}
		else if ((ctx->ctllen == 1) && (c == '7'))
		{
			// save cursor
			ctx->savCurX = ctx->curX;
			ctx->savCurY = ctx->curY;
			ctx->ctllen = 0;
		}
		else if ((ctx->ctllen == 1) && (c == '8'))
		{
			// restore cursor
			ctx->curX = ctx->savCurX;
			ctx->curY = ctx->savCurY;
			ctx->ctllen = 0;
		}
		else if ((ctx->ctllen == 1) && (c == 'c'))
		{
			// full reset
			mtClear(ctx);
			ctx->ctllen = 0;
		}
		else if (ctx->ctllen == 0)
		{
			if (c == '\e')
			{
				strcpy(ctx->ctlbuf, "\e");
				ctx->ctllen = 1;
			}
			else if (c == '\r')
			{
				ctx->curX = 0;
			}
			else if (c == '\n')
			{
				ctx->curY++;
				if (ctx->curY == ctx->height)
				{
					ctx->curY--;
					mtScrollPRIV(ctx);
				};
			}
			else if (c == '\b')
			{
				if ((ctx->curX != 0) || (ctx->curY != 0))
				{
					if (ctx->curX == 0)
					{
						ctx->curY--;
						ctx->curX = ctx->width;
					};

					ctx->curX--;
					ctx->matrix[2 * (ctx->curY * ctx->width + ctx->curX)] = 0;
				};
			}
			else
			{
				mtPutChar(ctx, c);
			};
		};
	};
};

void mtWriteKey(MTCONTEXT *ctx, unsigned int key, unsigned int mods)
{
	char *buf = NULL;

	switch (key)
	{
	case '\n':		buf = "\r";			break;
	case MT_KEY_UP:		buf = "\e[A";			break;
	case MT_KEY_DOWN:	buf = "\e[B";			break;
	case MT_KEY_RIGHT:	buf = "\e[C";			break;
	case MT_KEY_LEFT:	buf = "\e[D";			break;
	case '\b':		buf = "\b";			break;
	};

	if (buf == NULL)
	{
		char c = (char) key;
		write(ctx->fd, &c, 1);
	}
	else
	{
		write(ctx->fd, buf, strlen(buf));
	};
};

void mtDeleteContext(MTCONTEXT *ctx)
{
#ifdef __unix__
	if (ctx->fd != -1)
	{
		close(ctx->fd);
	};
#endif
	free(ctx);
};

#ifdef __unix__
void mtUpdate(MTCONTEXT *ctx)
{
	struct pollfd fdArray;
	char *buf = NULL;
	char *pos;
	int bytesPeek = 0;
	size_t bytesWaiting = 0;
	size_t bytesRead = 0;
	size_t bytesRemaining = 0;
	size_t bytesTotal = 0;
	int retval;
	int status;
	int errcpy = 0;
	pid_t childPid;

	childPid = waitpid(ctx->pid, &status, WNOHANG);
	if ((childPid == ctx->pid) || (childPid == -1))
	{
		ctx->attr |= MT_ATTR_DEAD;
	};

	fdArray.fd = ctx->fd;
	fdArray.events = POLLIN;

	retval = poll(&fdArray, 1, 10);
	if (retval <= 0)
	{
		return;
	};

#ifdef FIONREAD
	retval = ioctl(ctx->fd, FIONREAD, &bytesPeek);
#else
	retval = ioctl(ctx->fd, TIOCINQ, &bytesPeek);
#endif

	if (retval == -1) return;
	if (bytesPeek == 0) return;

	bytesWaiting = bytesPeek;
	bytesRemaining = bytesWaiting;

	buf = (char*) calloc(bytesWaiting+10, sizeof(char));
	pos = buf;

	do
	{
		bytesRead = read(ctx->fd, pos, bytesRemaining);
		if (bytesRead == -1)
		{
			free(buf);
			return;
		};

		if (bytesRead <= 0)
		{
			break;
		};

		bytesRemaining -= bytesRead;
		bytesTotal += bytesRead;
		pos += bytesRead;
	} while (bytesRemaining > 0);

	mtWrite(ctx, buf, bytesTotal);
	free(buf);		
};

#endif /* __unix__ */
