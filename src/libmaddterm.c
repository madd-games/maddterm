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
#include <pty.h>
#include <stdlib.h>
#include <sys/ioctl.h>

MTCONTEXT *mtCreateContext(MTPARAMS *params)
{
	MTCONTEXT *ctx = (MTCONTEXT*) malloc(sizeof(ctx));
	ctx->width = params->width;
	ctx->height = params->height;
	ctx->matrix = params->matrix;
	struct winsize winsz = {.ws_xpixel = params->width, .ws_ypixel = params->height};
	ctx->pid = forkpty(&ctx->fd, NULL, NULL, &winsz);
	
	if (ctx->pid == 0)
	{
		// we don't need the terminal structure on the child.
		free(ctx);
		params->start();
		exit(0);
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
	
	int i;
	for (i=0; i<ctx->width * ctx->height; i++)
	{
		ctx->matrix[i*2+0] = 0;
		ctx->matrix[i*2+1] = 0x07;
	};
};

void mtScrollPRIV(MTCONTEXT *ctx)
{
	int i;
	for (i=0; i<2*ctx->width*(ctx->height-1); i++)
	{
		ctx->matrix[i-ctx->width] = ctx->matrix[i];
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
	ctx->matrix[2*index+0] = c;
	ctx->matrix[2*index+0] = ctx->curColor;
	
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
		else
		{
			mtPutChar(ctx, c);
		};
	};
};
