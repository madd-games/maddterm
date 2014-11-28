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
#include <stdio.h>

static void handleCSI_EL(MTCONTEXT *ctx)
{
	int startidx, endidx;

	switch (ctx->ctlbuf[2])
	{
	case '1':
		startidx = ctx->curY * ctx->width;
		endidx = ctx->curY * ctx->width + ctx->curX;
		break;
	case '2':
		startidx = ctx->curY * ctx->width;
		endidx = (ctx->curY+1) * ctx->width - 1;
		break;
	default:
		startidx = ctx->curY * ctx->width + ctx->curX;
		endidx = (ctx->curY+1) * ctx->width - 1;
		break;
	};

	int i;
	for (i=startidx; i<=endidx; i++)
	{
		ctx->matrix[2*i+0] = 0;
		ctx->matrix[2*i+1] = ctx->curColor;
	};

	ctx->ctllen = 0;
};

static int isCSI_EL(MTCONTEXT *ctx)
{
	if (ctx->ctllen == 3)
	{
		if (memcmp(ctx->ctlbuf, "\e[K", 3) == 0)
		{
			return 1;
		};
	};

	if (ctx->ctllen != 4)
	{
		return 0;
	};

	if (memcmp(ctx->ctlbuf, "\e[", 2) != 0)
	{
		return 0;
	};

	if (ctx->ctlbuf[ctx->ctllen-1] != 'K')
	{
		return 0;
	};

	return ((ctx->ctlbuf[2] >= '0') && (ctx->ctlbuf[2] <= 2));
};

void mtHandleCSIPRIV(MTCONTEXT *ctx)
{
	if (isCSI_EL(ctx))
	{
		handleCSI_EL(ctx);
	};
};
