/**
 * FreeRDP: A Remote Desktop Protocol Client
 * DirectFB Client
 *
 * Copyright 2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <pthread.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/vt.h>
#include <sys/ioctl.h>
#include <freerdp/utils/args.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/semaphore.h>
#include <freerdp/utils/event.h>
#include <freerdp/constants.h>
#include <freerdp/plugins/cliprdr.h>
#include <freerdp/gdi/region.h>

#include "df_event.h"
#include "df_graphics.h"

#include "dfreerdp.h"

#define BUSY_THRESHOLD					500 //milliseconds
#define BUSY_FRAMEDROP_INTERVAL			150 //milliseconds
#define BUSY_INPUT_DEFER_INTERVAL		500 //milliseconds


#define DF_LOCK_BIT_INIT		1
#define DF_LOCK_BIT_PAINT		2

static freerdp_sem g_sem;
static int g_thread_count = 0;

struct thread_data
{
	freerdp* instance;
};

void df_context_new(freerdp* instance, rdpContext* context)
{
	context->channels = freerdp_channels_new();
}

void df_context_free(freerdp* instance, rdpContext* context)
{

}

static boolean df_lock_fb(dfInfo *dfi, uint8 mask)
{
	if (!dfi->primary_locks)
	{
		dfi->err = dfi->secondary ? 
			dfi->secondary->Lock(dfi->secondary, DSLF_WRITE | DSLF_READ, (void **)&dfi->primary_data, &dfi->primary_pitch) : 
			dfi->primary->Lock(dfi->primary, DSLF_WRITE | DSLF_READ, (void **)&dfi->primary_data, &dfi->primary_pitch);

		if (dfi->err!=DFB_OK)
		{
			printf("DirectFB Lock failed! mask=0x%x err=0x%x\n",  mask, dfi->err);
			return false;
		}
	}

	dfi->primary_locks |= mask;
	return true;
}

static boolean df_unlock_fb(dfInfo *dfi, uint8 mask)
{
	if ((dfi->primary_locks&mask)==0)
	{
//		fprintf(stderr, "Mismatching df_uloock_fb, mask=0x%x locks=0x%x\n", mask, dfi->primary_locks);
		return false;
	}

	dfi->primary_locks &= ~mask;

	if (!dfi->primary_locks)
	{
		dfi->primary_data = 0;
		dfi->primary_pitch = 0;
		if (dfi->secondary)
			dfi->secondary->Unlock(dfi->secondary);
		else
			dfi->primary->Unlock(dfi->primary);
	}
	return true;
}


static uint64 get_ticks(void)
{
	uint64 rv;
	struct timeval tv ;
	gettimeofday(&tv, NULL);
	rv = tv.tv_sec;
	rv*= 1000;
	rv+= (uint64)(tv.tv_usec / 1000);
	return rv;
}

INLINE static int get_active_tty(int fd)
{
	struct vt_stat vts;
	if (ioctl (fd, VT_GETSTATE, &vts)==-1)
		return -1;
	return vts.v_active;
}

void df_begin_paint(rdpContext* context)
{
	int ninvalid;
	HGDI_RGN cinvalid;
	rdpGdi* gdi = context->gdi;
	dfInfo* dfi = ((dfContext*) context)->dfi;

	if (!((dfContext*) context)->endpaint_defer_ts)
	{
		if (dfi->tty_fd!=-1)
		{
			if (dfi->tty_background)
			{
				if (dfi->tty_mine==get_active_tty(dfi->tty_fd))
				{
					dfi->tty_background = false;
					printf("Entered foreground\n");
					if (dfi->secondary && ((dfContext*) context)->direct_fullscreen && ((dfContext*) context)->direct_surface)
					{
						cinvalid = gdi->primary->hdc->hwnd->cinvalid;
						ninvalid = gdi->primary->hdc->hwnd->ninvalid;

						for (; ninvalid>0; --ninvalid, ++cinvalid)
						{
							if (cinvalid->w>0 && cinvalid->h>0)
							{
								dfi->update_rect.x = cinvalid->x;
								dfi->update_rect.y = cinvalid->y;
								dfi->update_rect.w = cinvalid->w;
								dfi->update_rect.h = cinvalid->h;
								dfi->primary->Blit(dfi->primary, dfi->secondary, &(dfi->update_rect), dfi->update_rect.x, dfi->update_rect.y);
							}
						}

						dfi->secondary->Release(dfi->secondary);
						dfi->secondary = 0;
					}
				}
				else
					usleep(1000000);
			}
			else
			{
				if (dfi->tty_mine!=get_active_tty(dfi->tty_fd))
				{
					dfi->tty_background = true;
					printf("Entered background\n");
					if (((dfContext*) context)->direct_fullscreen && ((dfContext*) context)->direct_surface)
					{
						dfi->dsc.flags = DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
						dfi->dsc.caps = DSCAPS_SYSTEMONLY;
						dfi->dsc.width = gdi->width;
						dfi->dsc.height = gdi->height;

						if (gdi->dstBpp == 32 || gdi->dstBpp == 24)
							dfi->dsc.pixelformat = DSPF_AiRGB;
						else if (gdi->dstBpp == 16 || gdi->dstBpp == 15)
							dfi->dsc.pixelformat = DSPF_RGB16;
						else if (gdi->dstBpp == 8)
							dfi->dsc.pixelformat = DSPF_RGB332;
						else
							dfi->dsc.pixelformat = DSPF_AiRGB;

						dfi->dfb->CreateSurface(dfi->dfb, &(dfi->dsc), &(dfi->secondary));
						dfi->secondary->Blit(dfi->secondary, dfi->primary, NULL, 0, 0);
					}
				}
			}
		}

		if (((dfContext*) context)->direct_surface)
		{
			if (!df_lock_fb(dfi, DF_LOCK_BIT_PAINT))
				abort();//will die anyway

			gdi->primary_buffer = dfi->primary_data;
			gdi->primary->bitmap->data = dfi->primary_data;
//			gdi_reinit(dfi->instance, dfi->primary_data);
		}

		gdi->primary->hdc->hwnd->invalid->null = 1;
		gdi->primary->hdc->hwnd->ninvalid = 0;
	}

}

static void df_end_paint_inner(rdpContext* context)
{
	rdpGdi* gdi;
	dfInfo* dfi;
	int ninvalid;
	HGDI_RGN cinvalid;

	gdi = context->gdi;
	dfi = ((dfContext*) context)->dfi;
	cinvalid = gdi->primary->hdc->hwnd->cinvalid;
	ninvalid = gdi->primary->hdc->hwnd->ninvalid;

	if (((dfContext*) context)->direct_surface)
	{
		gdi_DecomposeInvalidArea(gdi->primary->hdc);
		if (df_unlock_fb(dfi, DF_LOCK_BIT_PAINT))
		{			
			if (!gdi->primary->hdc->hwnd->invalid->null)
			{
				for (; ninvalid>0; --ninvalid, ++cinvalid)
				{
					if (cinvalid->w>0 && cinvalid->h>0)
					{
						DFBRegion fdbr = {cinvalid->x, cinvalid->y,
							cinvalid->x + cinvalid->w - 1,
							cinvalid->y + cinvalid->h - 1};
						dfi->primary->Flip(dfi->primary, &fdbr, DSFLIP_ONSYNC | DSFLIP_PIPELINE);
					}
				}
			}
		}
		if (dfi->tty_background)
			return;
	}
	else if (!gdi->primary->hdc->hwnd->invalid->null)
	{ 
		gdi_DecomposeInvalidArea(gdi->primary->hdc);
		if (dfi->tty_background)
			return;

		for (; ninvalid>0; --ninvalid, ++cinvalid)
		{
			if (cinvalid->w>0 && cinvalid->h>0)
			{
				dfi->update_rect.x = cinvalid->x;
				dfi->update_rect.y = cinvalid->y;
				dfi->update_rect.w = cinvalid->w;
				dfi->update_rect.h = cinvalid->h;
				dfi->primary->Blit(dfi->primary, dfi->secondary, &(dfi->update_rect), dfi->update_rect.x, dfi->update_rect.y);
			}
		}
	}

	gdi->primary->hdc->hwnd->ninvalid = 0;
}

void df_end_paint(rdpContext* context)
{
	const uint64 now = get_ticks();
	if (((dfContext*) context)->endpaint_defer_ts)
	{
		if ((now - ((dfContext*) context)->endpaint_defer_ts)<BUSY_FRAMEDROP_INTERVAL)
			return;

		((dfContext*) context)->endpaint_defer_ts = 0;
	}
	else if (((dfContext*) context)->busy_ts)
	{
		if ((now - ((dfContext*) context)->busy_ts)>BUSY_THRESHOLD)
		{
			((dfContext*) context)->endpaint_defer_ts = now;
			return;
		}
	}
	else if (context->gdi->primary->hdc->hwnd->invalid->null)
		return;
	else
		((dfContext*) context)->busy_ts = now;

	df_end_paint_inner(context);
}

boolean df_get_fds(freerdp* instance, void** rfds, int* rcount, void** wfds, int* wcount)
{
	dfInfo* dfi;

	dfi = ((dfContext*) instance->context)->dfi;

	rfds[*rcount] = (void*)(long)(dfi->read_fds);
	(*rcount)++;

	return true;
}

boolean df_check_fds(freerdp* instance, fd_set* set)
{
	dfInfo* dfi;
	int r;

	dfi = ((dfContext*) instance->context)->dfi;

	if (!FD_ISSET(dfi->read_fds, set))
		return true;

	for (;;)
	{
		r = read(dfi->read_fds, 
			((char *)&(dfi->event)) + dfi->read_len_pending, 
			sizeof(dfi->event) - dfi->read_len_pending);
		if (r<=0) break;
		dfi->read_len_pending+= r;
		if (dfi->read_len_pending>=sizeof(dfi->event))
		{
			df_event_process(instance, &(dfi->event));
			dfi->read_len_pending = 0;
		}
	}

	return true;
}

boolean df_pre_connect(freerdp* instance)
{
	dfInfo* dfi;
	boolean bitmap_cache;
	dfContext* context;
	rdpSettings* settings;

	dfi = (dfInfo*) xzalloc(sizeof(dfInfo));
	context = ((dfContext*) instance->context);
	context->dfi = dfi;

	settings = instance->settings;
	bitmap_cache = settings->bitmap_cache;

	settings->order_support[NEG_DSTBLT_INDEX] = true;
	settings->order_support[NEG_PATBLT_INDEX] = true;
	settings->order_support[NEG_SCRBLT_INDEX] = true;
	settings->order_support[NEG_OPAQUE_RECT_INDEX] = true;
	settings->order_support[NEG_DRAWNINEGRID_INDEX] = false;
	settings->order_support[NEG_MULTIDSTBLT_INDEX] = false;
	settings->order_support[NEG_MULTIPATBLT_INDEX] = false;
	settings->order_support[NEG_MULTISCRBLT_INDEX] = false;
	settings->order_support[NEG_MULTIOPAQUERECT_INDEX] = true;
	settings->order_support[NEG_MULTI_DRAWNINEGRID_INDEX] = false;
	settings->order_support[NEG_LINETO_INDEX] = true;
	settings->order_support[NEG_POLYLINE_INDEX] = true;
	settings->order_support[NEG_MEMBLT_INDEX] = bitmap_cache;
	settings->order_support[NEG_MEM3BLT_INDEX] = false;
	settings->order_support[NEG_MEMBLT_V2_INDEX] = bitmap_cache;
	settings->order_support[NEG_MEM3BLT_V2_INDEX] = false;
	settings->order_support[NEG_SAVEBITMAP_INDEX] = false;
	settings->order_support[NEG_GLYPH_INDEX_INDEX] = false;
	settings->order_support[NEG_FAST_INDEX_INDEX] = false;
	settings->order_support[NEG_FAST_GLYPH_INDEX] = false;
	settings->order_support[NEG_POLYGON_SC_INDEX] = false;
	settings->order_support[NEG_POLYGON_CB_INDEX] = false;
	settings->order_support[NEG_ELLIPSE_SC_INDEX] = false;
	settings->order_support[NEG_ELLIPSE_CB_INDEX] = false;

	dfi->clrconv = xnew(CLRCONV);
	dfi->clrconv->alpha = 1;
	dfi->clrconv->invert = 0;
	dfi->clrconv->rgb555 = 0;
	dfi->clrconv->palette = xnew(rdpPalette);

	freerdp_channels_pre_connect(instance->context->channels, instance);

	return true;
}



boolean df_post_connect(freerdp* instance)
{
	rdpGdi* gdi;
	dfInfo* dfi;
	dfContext* context;
	int flags, i;
	char sz[32];

	context = ((dfContext*) instance->context);
	dfi = context->dfi;
	dfi->instance = instance;
	dfi->err = DirectFBCreate(&(dfi->dfb));
	if (dfi->err!=DFB_OK)
	{
		printf("DirectFB init failed! err=0x%x\n",  dfi->err);
		return false;
	}

	if (context->direct_fullscreen)
	{
		dfi->dfb->SetCooperativeLevel(dfi->dfb, DFSCL_FULLSCREEN);
		for (i = 0; i<12; ++i)
		{
			sprintf(sz, "/dev/tty%d", i);
			dfi->tty_fd = open(sz, O_RDWR);
			if (dfi->tty_fd!=-1)
			{
				dfi->tty_mine = get_active_tty(dfi->tty_fd);
				if (dfi->tty_mine!=-1)
				{
					printf("Current TTY is %d\n", dfi->tty_mine);
					break;
				}
				close(dfi->tty_fd);
				dfi->tty_fd = -1;
			}
		}
	}
	else	
		dfi->tty_fd = -1;

	dfi->dsc.flags = DSDESC_CAPS;
	dfi->dsc.caps = DSCAPS_PRIMARY;
	dfi->err = dfi->dfb->CreateSurface(dfi->dfb, &(dfi->dsc), &(dfi->primary));
	if (dfi->err!=DFB_OK)
	{
		printf("DirectFB surface failed! err=0x%x\n",  dfi->err);
		return false;
	}

	if (context->direct_surface)
	{
		if (!df_lock_fb(dfi, DF_LOCK_BIT_INIT))
			return false;

		gdi_init(instance, CLRCONV_ALPHA | CLRCONV_INVERT | CLRBUF_16BPP | CLRBUF_32BPP, dfi->primary_data);
		gdi = instance->context->gdi;

		dfi->err = dfi->primary->GetSize(dfi->primary, &(gdi->width), &(gdi->height));
		if (dfi->err!=DFB_OK)
		{
			printf("DirectFB query surface size failed! err=0x%x\n",  dfi->err);
			return false;
		}
		df_unlock_fb(dfi, DF_LOCK_BIT_INIT);

		dfi->err = dfi->dfb->SetVideoMode(dfi->dfb, gdi->width, gdi->height, gdi->dstBpp);
		printf("SetVideoMode %dx%dx%d 0x%x\n", gdi->width, gdi->height, gdi->dstBpp, dfi->err);


		dfi->dfb->CreateInputEventBuffer(dfi->dfb, DICAPS_ALL, DFB_TRUE, &(dfi->event_buffer));
		dfi->event_buffer->CreateFileDescriptor(dfi->event_buffer, &(dfi->read_fds));

		flags = fcntl(dfi->read_fds, F_GETFL, 0);
    	if ( flags == -1 || fcntl(dfi->read_fds, F_SETFL, flags | O_NONBLOCK) == -1 )
	    {
    	    perror("DirectFB non-blocking mode");
	        return false;
	    }
		dfi->read_len_pending = 0;

		dfi->dfb->GetDisplayLayer(dfi->dfb, 0, &(dfi->layer));
		dfi->layer->EnableCursor(dfi->layer, 1);
		if (!df_lock_fb(dfi, DF_LOCK_BIT_INIT))
			return false;

		instance->update->BeginPaint = df_begin_paint;
		instance->update->EndPaint = df_end_paint;

		df_keyboard_init();

		pointer_cache_register_callbacks(instance->update);
		df_register_graphics(instance->context->graphics);

		freerdp_channels_post_connect(instance->context->channels, instance);
		df_unlock_fb(dfi, DF_LOCK_BIT_INIT);
		printf("DirectFB client initialized in experimental single-surface mode!\n");
		return true;
	}

	gdi_init(instance, CLRCONV_ALPHA | CLRCONV_INVERT | CLRBUF_16BPP | CLRBUF_32BPP, NULL);
	gdi = instance->context->gdi;

	dfi->err = dfi->primary->GetSize(dfi->primary, &(gdi->width), &(gdi->height));
	if (dfi->err!=DFB_OK)
	{
		printf("DirectFB query surface size failed! err=0x%x\n",  dfi->err);
		return false;
	}

	dfi->err = dfi->dfb->SetVideoMode(dfi->dfb, gdi->width, gdi->height, gdi->dstBpp);

	printf("SetVideoMode %dx%dx%d 0x%x\n", gdi->width, gdi->height, gdi->dstBpp, dfi->err);

	dfi->dfb->CreateInputEventBuffer(dfi->dfb, DICAPS_ALL, DFB_TRUE, &(dfi->event_buffer));
	dfi->event_buffer->CreateFileDescriptor(dfi->event_buffer, &(dfi->read_fds));

	flags = fcntl(dfi->read_fds, F_GETFL, 0);
    if ( flags == -1 || fcntl(dfi->read_fds, F_SETFL, flags | O_NONBLOCK) == -1 )
    {
        perror("DirectFB non-blocking mode");
        return false;
    }
	dfi->read_len_pending = 0;

	dfi->dfb->GetDisplayLayer(dfi->dfb, 0, &(dfi->layer));
	dfi->layer->EnableCursor(dfi->layer, 1);

	dfi->dsc.flags = DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT;
	dfi->dsc.caps = DSCAPS_SYSTEMONLY;
	dfi->dsc.width = gdi->width;
	dfi->dsc.height = gdi->height;

	if (gdi->dstBpp == 32 || gdi->dstBpp == 24)
		dfi->dsc.pixelformat = DSPF_AiRGB;
	else if (gdi->dstBpp == 16 || gdi->dstBpp == 15)
		dfi->dsc.pixelformat = DSPF_RGB16;
	else if (gdi->dstBpp == 8)
		dfi->dsc.pixelformat = DSPF_RGB332;
	else
		dfi->dsc.pixelformat = DSPF_AiRGB;

	dfi->dsc.preallocated[0].data = gdi->primary_buffer;
	dfi->dsc.preallocated[0].pitch = gdi->width * gdi->bytesPerPixel;
	dfi->dfb->CreateSurface(dfi->dfb, &(dfi->dsc), &(dfi->secondary));

	instance->update->BeginPaint = df_begin_paint;
	instance->update->EndPaint = df_end_paint;

	df_keyboard_init();

	pointer_cache_register_callbacks(instance->update);
	df_register_graphics(instance->context->graphics);

	freerdp_channels_post_connect(instance->context->channels, instance);
	return true;
}

static int df_process_plugin_args(rdpSettings* settings, const char* name,
	RDP_PLUGIN_DATA* plugin_data, void* user_data)
{
	rdpChannels* channels = (rdpChannels*) user_data;

	printf("loading plugin %s\n", name);
	freerdp_channels_load_plugin(channels, settings, name, plugin_data);

	return 1;
}

boolean df_verify_certificate(freerdp* instance, char* subject, char* issuer, char* fingerprint)
{
	printf("Certificate details:\n");
	printf("\tSubject: %s\n", subject);
	printf("\tIssuer: %s\n", issuer);
	printf("\tThumbprint: %s\n", fingerprint);
	printf("The above X.509 certificate could not be verified, possibly because you do not have "
		"the CA certificate in your certificate store, or the certificate has expired. "
		"Please look at the documentation on how to create local certificate store for a private CA.\n");

	char answer;
	while (1)
	{
		printf("Do you trust the above certificate? (Y/N) ");
		answer = fgetc(stdin);

		if (answer == 'y' || answer == 'Y')
		{
			return true;
		}
		else if (answer == 'n' || answer == 'N')
		{
			break;
		}
	}

	return false;
}

static int
df_receive_channel_data(freerdp* instance, int channelId, uint8* data, int size, int flags, int total_size)
{
	return freerdp_channels_data(instance, channelId, data, size, flags, total_size);
}

static void
df_process_cb_monitor_ready_event(rdpChannels* channels, freerdp* instance)
{
	RDP_EVENT* event;
	RDP_CB_FORMAT_LIST_EVENT* format_list_event;

	event = freerdp_event_new(RDP_EVENT_CLASS_CLIPRDR, RDP_EVENT_TYPE_CB_FORMAT_LIST, NULL, NULL);

	format_list_event = (RDP_CB_FORMAT_LIST_EVENT*)event;
	format_list_event->num_formats = 0;

	freerdp_channels_send_event(channels, event);
}

static void
df_process_channel_event(rdpChannels* channels, freerdp* instance)
{
	RDP_EVENT* event;

	event = freerdp_channels_pop_event(channels);

	if (event)
	{
		switch (event->event_type)
		{
			case RDP_EVENT_TYPE_CB_MONITOR_READY:
				df_process_cb_monitor_ready_event(channels, instance);
				break;
			default:
				printf("df_process_channel_event: unknown event type %d\n", event->event_type);
				break;
		}

		freerdp_event_free(event);
	}
}

static void df_free(dfInfo* dfi)
{
	dfi->dfb->Release(dfi->dfb);
	xfree(dfi);
}

int dfreerdp_run(freerdp* instance)
{
	int i;
	int fds;
	int max_fds;
	int rcount;
	int wcount;
	void* rfds[32];
	void* wfds[32];
	fd_set rfds_set;
	fd_set wfds_set;
	struct timeval tv;
	dfInfo* dfi;
	dfContext* context;
	rdpChannels* channels;

	memset(rfds, 0, sizeof(rfds));
	memset(wfds, 0, sizeof(wfds));

	if (!freerdp_connect(instance))
		return 0;

	context = (dfContext*) instance->context;

	dfi = context->dfi;
	channels = instance->context->channels;

	while (1)
	{
		rcount = 0;
		wcount = 0;

		if (freerdp_get_fds(instance, rfds, &rcount, wfds, &wcount) != true)
		{
			printf("Failed to get FreeRDP file descriptor\n");
			break;
		}
		if (freerdp_channels_get_fds(channels, instance, rfds, &rcount, wfds, &wcount) != true)
		{
			printf("Failed to get channel manager file descriptor\n");
			break;
		}

		if (context->input_defer_ts)
		{
			if (!context->busy_ts)
			{
				df_events_commit(instance);
				context->input_defer_ts = 0;
			}
			else if (get_ticks()-context->input_defer_ts>=BUSY_INPUT_DEFER_INTERVAL)
			{
				df_events_commit(instance);
				context->input_defer_ts = get_ticks();
			}
		}
		else if (context->busy_ts && context->endpaint_defer_ts)
		{
			context->input_defer_ts = context->busy_ts;
		}

		if (df_get_fds(instance, rfds, &rcount, wfds, &wcount) != true)
		{
			printf("Failed to get dfreerdp file descriptor\n");
			break;
		}

		max_fds = 0;
		FD_ZERO(&rfds_set);
		FD_ZERO(&wfds_set);

		for (i = 0; i < rcount; i++)
		{
			fds = (int)(long)(rfds[i]);

			if (fds > max_fds)
				max_fds = fds;

			FD_SET(fds, &rfds_set);
		}

		if (max_fds == 0)
			break;

		if (context->busy_ts)
		{
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			i = select(max_fds + 1, &rfds_set, &wfds_set, NULL, &tv);
			if ( i == 0 )
			{
				context->busy_ts = 0;
				if (context->endpaint_defer_ts)
				{
					context->endpaint_defer_ts = 0;
					df_end_paint_inner((rdpContext*)context);
				}
			}
		}
		else
			i = select(max_fds + 1, &rfds_set, &wfds_set, NULL, NULL);


		if (i == -1)
		{
			/* these are not really errors */
			if (!((errno == EAGAIN) ||
				(errno == EWOULDBLOCK) ||
				(errno == EINPROGRESS) ||
				(errno == EINTR))) /* signal occurred */
			{
				printf("dfreerdp_run: select failed\n");
				break;
			}
		}

		if (freerdp_check_fds(instance) != true)
		{
			printf("Failed to check FreeRDP file descriptor\n");
			break;
		}

		if (df_check_fds(instance, &rfds_set) != true)
		{
			printf("Failed to check dfreerdp file descriptor\n");
			break;
		}
		if (!context->input_defer_ts)
			df_events_commit(instance);


		if (freerdp_channels_check_fds(channels, instance) != true)
		{
			printf("Failed to check channel manager file descriptor\n");
			break;
		}
		df_process_channel_event(channels, instance);
	}

	freerdp_channels_close(channels, instance);
	freerdp_channels_free(channels);
	df_free(dfi);
	gdi_free(instance);
	freerdp_disconnect(instance);
	freerdp_free(instance);

	return 0;
}

void* thread_func(void* param)
{
	struct thread_data* data;
	data = (struct thread_data*) param;

	dfreerdp_run(data->instance);

	xfree(data);

	pthread_detach(pthread_self());

	g_thread_count--;

        if (g_thread_count < 1)
                freerdp_sem_signal(g_sem);

	return NULL;
}

int main(int argc, char* argv[])
{
	int i;
	pthread_t thread;
	freerdp* instance;
	dfContext* context;
	rdpChannels* channels;
	struct thread_data* data;

	setlocale(LC_ALL, "");

	freerdp_channels_global_init();

	g_sem = freerdp_sem_new(1);

	instance = freerdp_new();
	instance->PreConnect = df_pre_connect;
	instance->PostConnect = df_post_connect;
	instance->VerifyCertificate = df_verify_certificate;
	instance->ReceiveChannelData = df_receive_channel_data;

	instance->context_size = sizeof(dfContext);
	instance->ContextNew = df_context_new;
	instance->ContextFree = df_context_free;
	freerdp_context_new(instance);

	context = (dfContext*) instance->context;
	channels = instance->context->channels;
	
	DirectFBInit(&argc, &argv);
	if (freerdp_parse_args(instance->settings, argc, argv, df_process_plugin_args, channels, NULL, NULL)==FREERDP_ARGS_PARSE_HELP)
	{
		printf("  --direct-surface: use only single DirectFB surface (faster, but repaints more 'visible')\n");
		printf("  --direct-fullscreen: set FULLSCREEN cooperative level (fast - primary surface is a screen, may cuase glitches on VT switch)\n");
		printf("\n");
		return 1;
	}

	for (i = 0; i<argc; ++i)
	{
		if (strcmp(argv[i], "--direct-surface")==0)
			context->direct_surface= true;
		else if (strcmp(argv[i], "--direct-fullscreen")==0)
			context->direct_fullscreen = true;
	}
	data = (struct thread_data*) xzalloc(sizeof(struct thread_data));
	data->instance = instance;

	g_thread_count++;
	pthread_create(&thread, 0, thread_func, data);

	while (g_thread_count > 0)
	{
                freerdp_sem_wait(g_sem);
	}

	freerdp_channels_global_uninit();

	return 0;
}
