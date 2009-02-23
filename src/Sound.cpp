//
// C++ Implementation: Sound
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Sound.h"

#ifdef HAVE_GST
#include "Control.h"
#include "output.h"
#include "utils.h"
Sound::Sound():pipeline(NULL), filesrc(NULL), decode(NULL), convert(NULL)
{
	gettimeofday(&timestamp, NULL);
	persist = false;
}

Sound::~Sound()
{
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
}

void Sound::InitSelf()
{
	GstElement *sink;
	GstBus *bus;

	gst_init(NULL, NULL);
	pipeline = gst_pipeline_new("sound-system");
	filesrc = gst_element_factory_make("filesrc", "source");
	decode = gst_element_factory_make("decodebin", "decode");
	convert = gst_element_factory_make("audioconvert", "convert");
	sink = gst_element_factory_make("autoaudiosink", "output");

	gst_bin_add_many(GST_BIN(pipeline), filesrc, decode,
					 convert, sink, NULL);
	gst_element_link_many(filesrc, decode, NULL);
	gst_element_link_many(convert, sink, NULL);
	g_signal_connect_swapped(decode, "new-decoded-pad",
			 G_CALLBACK(NewDecodedPad), convert);

	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_add_signal_watch(GST_BUS(bus));
	g_signal_connect_swapped(bus, "message::error",
			 G_CALLBACK(ErrorMessageOccur), this);
	g_signal_connect_swapped(bus, "message::eos",
			 G_CALLBACK(EosMessageOccur), this);
	gst_object_unref(bus);
}

void Sound::Playing(const char *file)
{
	extern Control ctr;
	struct timeval time;

	gettimeofday(&time, NULL);
	if (!FLAG_ISSET(ctr.sndfgs, 0) || (difftimeval(time, timestamp) < 0.1))
		return;

	if (persist)
		EosMessageOccur(this);
	persist = true;
	g_object_set(filesrc, "location", file, NULL);
	gst_element_set_state(pipeline, GST_STATE_PLAYING);
	timestamp = time;
}

void Sound::Stop()
{
	EosMessageOccur(this);
}

void Sound::NewDecodedPad(GstElement *convert, GstPad* pad)
{
	GstCaps *caps;
	GstStructure *str;
	GstPad *spad;

	caps = gst_pad_get_caps(pad);
	str = gst_caps_get_structure(caps, 0);
	if(strcasestr(gst_structure_get_name(str), "audio")
	   &&(spad = gst_element_get_compatible_pad(convert, pad, caps)))
		gst_pad_link(pad, spad);
	gst_caps_unref(caps);
}

void Sound::ErrorMessageOccur(pointer data, GstMessage *message)
{
	GError *error;

	gst_message_parse_error(message, &error, NULL);
	pwarning(Fail, _("act: play the prompt tone, warning: %s\n"),
						 error->message);
	g_error_free(error);
	EosMessageOccur(data);
	gst_element_set_state(((Sound *) data)->pipeline, GST_STATE_NULL);
}

void Sound::EosMessageOccur(pointer data)
{
	Sound *snd;

	snd = (Sound*)data;
	gst_element_set_state(snd->pipeline, GST_STATE_READY);
	gst_element_unlink(snd->decode, snd->convert);
	snd->persist = false;
}
#else
Sound::Sound()
{}
Sound::~Sound()
{}
void Sound::InitSelf()
{}

#endif
