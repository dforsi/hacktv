/* hacktv - Analogue video transmitter for the HackRF                    */
/*=======================================================================*/
/* Copyright 2023 Philip Heron <phil@sanslogic.co.uk>                    */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stddef.h>
#include "av.h"

const av_frame_t av_frame_default = {
	.width = 0,
	.height = 0,
	.framebuffer = NULL,
	.pixel_stride = 0,
	.line_stride = 0,
	.aspect = { 4, 3 },
	.interlaced = 0,
};

int av_read_video(av_t *s, av_frame_t *frame)
{
	int r;
	
	if(s->read_video)
	{
		r = s->read_video(s->av_source_ctx, frame);
	}
	else
	{
		*frame = av_frame_default;
		r = AV_OK;
	}
	
	if(r == AV_OK) s->frames++;
	
	return(r);
}

int16_t *av_read_audio(av_t *s, size_t *samples)
{
	int16_t *r = NULL;
	
	if(s->read_audio)
	{
		r = s->read_audio(s->av_source_ctx, samples);
	}
	
	s->samples += *samples;
	
	return(r);
}

int av_eof(av_t *s)
{
	return(s->eof ? s->eof(s->av_source_ctx) : 0);
}

int av_close(av_t *s)
{
	int r;
	
	r = s->close ? s->close(s->av_source_ctx) : AV_ERROR;
	
	s->av_source_ctx = NULL;
	s->read_video = NULL;
	s->read_audio = NULL;
	s->eof = NULL;
	s->close = NULL;
	
	return(r);
}

void av_hflip_frame(av_frame_t *frame)
{
	frame->framebuffer += (frame->width - 1) * frame->pixel_stride;
	frame->pixel_stride = -frame->pixel_stride;
}

void av_vflip_frame(av_frame_t *frame)
{
	frame->framebuffer += (frame->height - 1) * frame->line_stride;
	frame->line_stride = -frame->line_stride;
}

void av_rotate_frame(av_frame_t *frame, int a)
{
	a = a % 4;
	
	if(a == 1 || a == 3)
	{
		int i;
		
		/* Rotate the frame 90 degrees clockwise */
		
		frame->framebuffer += (frame->height - 1) * frame->line_stride;
		
		i = frame->width;
		frame->width = frame->height;
		frame->height = i;
		
		i = frame->pixel_stride;
		frame->pixel_stride = -frame->line_stride;
		frame->line_stride = i;
        }
	
	if(a == 2 || a == 3)
	{
		/* Rotate the frame 180 degrees */
		av_hflip_frame(frame);
		av_vflip_frame(frame);
	}
}

