/*
  This bin adds DLNA playback capabilities to souphttpsrc
 */

/**
 * SECTION:element-dlnabin
 *
 * HTTP/DLNA client source
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch ...
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <gst/gst.h>
#include <glib-object.h>

#include "gstdlnabin.h"

GST_DEBUG_CATEGORY_STATIC (gst_dlna_bin_debug);
#define GST_CAT_DEFAULT gst_dlna_bin_debug

/* props */
enum
{
  ARG_0,
  ARG_URI,
  //...
};

/* signals */
/* (ew) may not be needed */
/*
enum
{
  //SIGNAL_NEW_DECODED_PAD,
  //SIGNAL_UNKNOWN_TYPE,
  LAST_SIGNAL
};
static guint gst_dlna_bin_signals[LAST_SIGNAL] = { 0 };
*/

GST_BOILERPLATE (GstDlnaBin, gst_dlna_bin, GstElement, GST_TYPE_BIN);

static void gst_dlna_bin_dispose (GObject * object);

static GstDlnaBin* gst_dlna_build_bin (GstDlnaBin *dlna_bin);

//GstDlnaBin* gst_dlna_rebuild_bin (GstDlnaBin *dlna_bin);

//static void gst_dlna_empty_bin (GstBin *bin);

//static GstElement* gst_ebif_build_eiss_pipeline (GstEbifBin *ebif_bin);

//static GstElement* gst_ebif_build_ebif_pipeline (GstEbifBin *ebif_bin);

static void gst_dlna_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * spec);

static void gst_dlna_bin_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * spec);

void
gst_play_marshal_VOID__OBJECT_BOOLEAN (GClosure *closure,
				       GValue *return_value G_GNUC_UNUSED,
				       guint n_param_values,
				       const GValue *param_values,
				       gpointer invocation_hint G_GNUC_UNUSED,
				       gpointer marshal_data);


const GstElementDetails gst_dlna_bin_details
= GST_ELEMENT_DETAILS("HTTP/DLNA client source -11/6 10:36",
		      "Source/Network",
		      "Receive data as a client via HTTP with DLNA extensions",
		      "Eric Winkelman <e.winkelman@cablelabs.com>");


static GstStaticPadTemplate gst_dlna_bin_src_pad_template =
GST_STATIC_PAD_TEMPLATE (
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);


static void
gst_dlna_bin_base_init (gpointer gclass)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

    gst_element_class_set_details_simple
      (element_class,
       "HTTP/DLNA client source",
       "Source/Network",
       "Receive data as a client via HTTP with DLNA extensions",
       "Eric Winkelman <e.winkelman@cablelabs.com>");

    // Add the src pad template
    gst_element_class_add_pad_template
      (element_class,
       gst_static_pad_template_get(&gst_dlna_bin_src_pad_template));
}

static void
gst_dlna_bin_class_init (GstDlnaBinClass * klass)
{
  GObjectClass *gobject_klass;
  GstElementClass *gstelement_klass;
  //GstBinClass *gstbin_klass;

  gobject_klass = (GObjectClass *) klass;
  gstelement_klass = (GstElementClass *) klass;
  //gstbin_klass = (GstBinClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_klass->set_property = gst_dlna_bin_set_property;
  gobject_klass->get_property = gst_dlna_bin_get_property;

  g_object_class_install_property (gobject_klass, ARG_URI,
      g_param_spec_string ("uri", "Stream URI",
			    "Sets URI A/V stream",
			   NULL, G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  gobject_klass->dispose = GST_DEBUG_FUNCPTR (gst_dlna_bin_dispose);

  gst_element_class_set_details (gstelement_klass, &gst_dlna_bin_details);

  /**
   * GstDlnaBin::new-decoded-pad:
   * @bin: The dlnabin
   * @pad: The available pad
   * @islast: #TRUE if this is the last pad to be added. Deprecated.
   *
   * This signal gets emitted as soon as a new pad of the same type as one of
   * the valid 'raw' types is added.
   */
  /*
  gst_dlna_bin_signals[SIGNAL_NEW_DECODED_PAD] =
      g_signal_new ("new-decoded-pad", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstDlnaBinClass, new_decoded_pad), NULL, NULL,
      gst_play_marshal_VOID__OBJECT_BOOLEAN, G_TYPE_NONE, 2, GST_TYPE_PAD,
      G_TYPE_BOOLEAN);
  */

  // Unused, but here to mimic decodebin
  /*
  gst_dlna_bin_signals[SIGNAL_UNKNOWN_TYPE] =
      g_signal_new ("unknown-type", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstDlnaBinClass, unknown_type), NULL, NULL,
      gst_marshal_VOID__OBJECT_BOXED, G_TYPE_NONE, 2, GST_TYPE_PAD,
      GST_TYPE_CAPS);
  */
}

static void
gst_dlna_bin_init (GstDlnaBin * dlna_bin,
		   GstDlnaBinClass * gclass)
{
  printf("\n(ew) Initializing the dlna bin\n");

  gst_dlna_build_bin(dlna_bin);

  printf("\n(ew) Initializing the dlna bin - done\n");
}

static void
gst_dlna_bin_dispose (GObject * object)
{
  printf("\n(ew) Disposing the dlna bin\n");

  G_OBJECT_CLASS (parent_class)->dispose (object);
}


static void 
gst_dlna_bin_set_property (GObject * object, guint prop_id,
			   const GValue * value, GParamSpec * pspec)
{
  GstDlnaBin *dlna_bin;

  dlna_bin = GST_DLNA_BIN (object);

  switch (prop_id) {

  case ARG_URI:
    {
      GstElement *elem;

      printf("(ew) Setting the URI property\n");

      // Set the uri in the bin
      // (ew) Do we need to free the old value?
      if (dlna_bin->uri) {
	free(dlna_bin->uri);
      }
      dlna_bin->uri = g_value_dup_string(value);
      
      // Get the http source
      elem = gst_bin_get_by_name(&dlna_bin->bin, "http-source");

      printf("(ew) Setting the URI to %s\n", dlna_bin->uri);

      // Set the URI
      g_object_set(G_OBJECT(elem), "location", dlna_bin->uri, NULL);
    }
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
gst_dlna_bin_get_property (GObject * object, guint prop_id, GValue * value,
			   GParamSpec * pspec)
{
  GstDlnaBin *dlna_bin;

  dlna_bin = GST_DLNA_BIN (object);

  switch (prop_id) {

  case ARG_URI:
    g_value_set_pointer(value, dlna_bin->uri);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}


/*
  gst_dlna_build_source_tee

  Create the source tee for the streams
 */
/* (ew) delete me...
static GstElement*
gst_dlna_build_source_tee (GstDlnaBin *dlna_bin)
{
  //GstElement *source = NULL;
  GstElement *tee;

  // Create a splitter for multiple streams
  tee = gst_element_factory_make ("tee", "stream-splitter");

  if (!tee) {
    g_printerr ("The source tee could not be created. Exiting.\n");
    exit(1);
  }

  // Add the element into the bin
  gst_bin_add_many(GST_BIN(&dlna_bin->bin), tee, NULL);

  return tee;
}
*/

/*
  gst_dlna_build_video_pipeline

  queue -> pidfilter -> esassembler -> mpegdecoder
 */
 /* (ew) delete me
static GstElement*
gst_dlna_build_video_pipeline (GstDlnaBin *dlna_bin)
{
  GstElement *videoQueue, *videoPF, *videoAssem, *videoParse;

  // Create the standard video stream elements
  videoQueue = gst_element_factory_make ("queue",       "video-queue");
  videoPF    = gst_element_factory_make ("pidfilter",   "video-pidfilter");
  videoAssem = gst_element_factory_make ("esassembler", "video-assembler");
  //videoParse = gst_element_factory_make ("ffdec_h264", "video-parser");
  videoParse = gst_element_factory_make ("mpeg2dec", "video-parser");

  // Make sure everything is good
  if (!videoQueue || !videoPF || !videoAssem || !videoParse) {
    g_printerr ("A video element could not be created. Exiting.\n");
    exit(1);
  }

  // Up the buffer size so the pipeline doesn't block
  g_object_set (G_OBJECT (videoQueue), "max-size-buffers", QUEUE_BUFF_SIZE,
		NULL);  
  //g_object_set (G_OBJECT (videoQueue), "leaky", 2, NULL);  

  // Set up the video filter
  g_object_set (G_OBJECT (videoPF), "pidlist", dlna_bin->video_pid, NULL);

  // Add the elements to the bin 
  gst_bin_add_many(GST_BIN(&dlna_bin->bin), videoQueue, videoPF,
		   videoAssem, videoParse, NULL);

  // Video bin
  gst_element_link_many(videoQueue, videoPF, videoAssem, videoParse, NULL);

  return videoQueue;
}
*/

/*
  gst_dlna_build_audio_pipeline

  queue -> pidfilter -> esassembler -> audioparse
 */
  /* (ew) delete me
static GstElement*
gst_dlna_build_audio_pipeline (GstDlnaBin *dlna_bin)
{
  GstElement *audioQueue;
  GstElement *audioPF, *audioAssem, *audioParse;

  // Create the standard audio elements
  audioQueue  = gst_element_factory_make ("queue",       "audio-queue");
  audioPF     = gst_element_factory_make ("pidfilter",   "audio-pidfilter");
  audioAssem  = gst_element_factory_make ("esassembler", "audio-assembler");
  //audioParse  = gst_element_factory_make ("faad",        "audio-parser");
  //audioParse  = gst_element_factory_make ("audioparse",  "audio-parser");
  //audioParse  = gst_element_factory_make ("mad",  "audio-parser");
  audioParse  = gst_element_factory_make ("a52dec",  "audio-parser");

  // Make sure everything is good
  if (!audioQueue || !audioPF || !audioAssem || !audioParse) {
    g_printerr ("An audio element could not be created. Exiting.\n");
    exit(1);
  }

  // Up the buffer size so the pipeline doesn't block
  g_object_set (G_OBJECT (audioQueue), "max-size-buffers", QUEUE_BUFF_SIZE,
		NULL);  
  //g_object_set (G_OBJECT (audioQueue), "leaky", 2, NULL);  

  // Set up the audio filter
  g_object_set (G_OBJECT (audioPF), "pidlist", dlna_bin->audio_pid, NULL);

  // Add the elements to the bin 
  gst_bin_add_many(GST_BIN(&dlna_bin->bin), audioQueue, audioPF,
		   audioAssem, audioParse, NULL);

  // Audio pipeline
  gst_element_link_many(audioQueue, audioPF, audioAssem, audioParse, NULL);

  return audioQueue;
}
*/

/*
  cb_dlna

  This function gets called by the sectionsink when there is a new section.
 */
   /* (ew) example callback
void cb_dlna (GstElement *eissSink, guint arg0, gpointer section,
	      gpointer dlnaBin)
{
  // int n;
  char *data = (char *)GST_BUFFER_DATA((GstBuffer *)section);
  int size = GST_BUFFER_SIZE((GstBuffer *)section);

  //printf("\ncb_eiss called!\n");
  //printf("arg0: %i\n", arg0);
  //printf("section: %p\n", section);
  //printf("ebif_bin: %p\n", ebifBin);
  //printf("eissSink: %p\n", eissSink);
  //printf("data address: %p\n", data);
  //printf("data size: %i\n", size);
  //printf("section:\n");
  //int n;
  //for(n = 1; n <= size; n++) {
  //  printf("%02x ", ((char *)data)[n-1] & 0xff);
  //  if (n % 16 == 0) printf("\n");
  //}
  //printf("\n");

  if (GST_DLNA_BIN(dlnaBin)->dlna_callback != NULL)
    ((gst_dlna_bin_callback_f)
     (GST_DLNA_BIN(dlnaBin)->dlna_callback))(data, size);

  //eiss_callback(data, size);
}
   */

/*
  gst_dlna_build_ebif_pipeline

  queue -> pidfilter -> sectionassembler -> sectionsink
*/
/* (ew) delete me
static GstElement*
gst_dlna_build_ebif_pipeline (GstDlnaBin *dlna_bin)
{
  GstElement *ebifQueue;
  GstElement *ebifPF, *ebifAssem, *ebifSink;
  GstElement *ebifDebug;

  // The ebif stream
  ebifQueue = gst_element_factory_make ("queue",            "ebif-queue");
  ebifPF    = gst_element_factory_make ("pidfilter",        "ebif-pidfilter");
  ebifAssem = gst_element_factory_make ("sectionassembler", "ebif-assembler");
  ebifDebug = gst_element_factory_make ("identity",         "ebif-debug");
  ebifSink  = gst_element_factory_make ("sectionsink",      "ebif-sink");

  if (!ebifQueue   || !ebifPF     || !ebifAssem   || !ebifSink) {
    g_printerr ("One EBIF element could not be created. Exiting.\n");
    exit(1);
  }

  // Up the buffer size so the pipeline doesn't block
  g_object_set (G_OBJECT (ebifQueue), "max-size-buffers", QUEUE_BUFF_SIZE,
		NULL);  
  //g_object_set (G_OBJECT (ebifQueue), "leaky", 2, NULL);  

  // Set up the EBIF filter
  g_object_set (G_OBJECT (ebifPF), "pidlist", dlna_bin->ebif_pid, NULL);
  g_object_set (G_OBJECT (ebifAssem), "assemble-all", 1, NULL);

  // Dump debugging info
  g_object_set (G_OBJECT (ebifDebug), "dump", FALSE, NULL);

  // Add the elements to the bin
  gst_bin_add_many(GST_BIN(&dlna_bin->bin), ebifQueue, ebifPF,
		   ebifDebug,
		   ebifAssem, ebifSink, NULL);

  // EBIF pipeline
  gst_element_link_many(ebifQueue, ebifPF, ebifAssem,
			ebifDebug,
			ebifSink, NULL);

  // Connect the eiss signal handler
  g_signal_connect(G_OBJECT(ebifSink), "section-available",
		   G_CALLBACK(cb_dlna), dlna_bin);

  return ebifQueue;
}
*/


/*
  gst_dlna_bin_eiss_callback_f

  The callback function for eiss sections
 */
//gst_dlna_bin_eiss_callback_f eiss_callback = NULL; //print_eiss;


/*
  gst_dlna_bin_set_eiss_callback

  Set the callback function for eiss sections
 */
/*
void gst_dlna_bin_set_eiss_callback (gst_dlna_bin_eiss_callback_f func)
{
  eiss_callback = func;
}
*/

/*
  cb_eiss

  This function gets called by the sectionsink when there is a new section.
 */
 /* (ew) example callback
void cb_eiss (GstElement *eissSink, guint arg0, gpointer section,
	      gpointer dlnaBin)
{
  // int n;
  char *data = (char *)GST_BUFFER_DATA((GstBuffer *)section);
  int size = GST_BUFFER_SIZE((GstBuffer *)section);

  //printf("\ncb_eiss called!\n");
  //printf("arg0: %i\n", arg0);
  //printf("section: %p\n", section);
  //printf("ebif_bin: %p\n", ebifBin);
  //printf("eissSink: %p\n", eissSink);
  //printf("data address: %p\n", data);
  //printf("data size: %i\n", size);
  //printf("section:\n");
  //int n;
  //for(n = 1; n <= size; n++) {
  //  printf("%02x ", ((char *)data)[n-1] & 0xff);
  //  if (n % 16 == 0) printf("\n");
  //}
  //printf("\n");

  if (GST_DLNA_BIN(dlnaBin)->eiss_callback != NULL)
    ((gst_dlna_bin_callback_f)
     (GST_DLNA_BIN(dlnaBin)->eiss_callback))(data, size);

  //eiss_callback(data, size);
}
 */

/*
  gst_dlna_build_eiss_pipeline

  queue -> pidfilter -> sectionassembler -> sectionsink
*/
/* (ew) delete me
static GstElement*
gst_dlna_build_eiss_pipeline (GstDlnaBin *dlna_bin)
{
  GstElement *eissQueue;
  GstElement *eissPF, *eissAssem, *eissSink;
  GstElement *eissDebug;

  // The eiss pipeline
  eissQueue = gst_element_factory_make ("queue",            "eiss-queue");
  eissPF    = gst_element_factory_make ("pidfilter",        "eiss-pidfilter");
  eissAssem = gst_element_factory_make ("sectionassembler", "eiss-assembler");
  eissDebug = gst_element_factory_make ("identity",         "eiss-debug");
  eissSink  = gst_element_factory_make ("sectionsink",      "eiss-sink");

  if (!eissQueue || !eissPF || !eissAssem || !eissSink) {
    g_printerr ("One EISS element could not be created. Exiting.\n");
    exit(1);
  }

  // Up the buffer size so the pipeline doesn't block
  g_object_set (G_OBJECT (eissQueue), "max-size-buffers", QUEUE_BUFF_SIZE,
		NULL);  
  //g_object_set (G_OBJECT (eissQueue), "leaky", 2, NULL);  

  // Set up the EISS filter
  g_object_set (G_OBJECT (eissPF), "pidlist", dlna_bin->eiss_pid, NULL);

  // Set up the EISS assembler
  g_object_set (G_OBJECT (eissAssem), "assemble-all", 1, NULL);

  // Dump debugging info
  g_object_set (G_OBJECT (eissDebug), "dump", FALSE, NULL);

  // Add the elements to the bin 
  gst_bin_add_many(GST_BIN(&dlna_bin->bin), eissQueue, eissPF,
		   eissDebug,
		   eissAssem, eissSink, NULL);

  // EISS pipeline
  gst_element_link_many(eissQueue, eissPF, eissAssem,
			eissDebug,
			eissSink, NULL);

  printf("dlna_bin: %p\n", dlna_bin);
  printf("eissSink: %p\n", eissSink);

  // Connect the eiss signal handler
  g_signal_connect(G_OBJECT(eissSink), "section-available",
		   G_CALLBACK(cb_eiss), dlna_bin);

  return eissQueue;
}
*/


/*
  cb_source_linked

  A call back function for when the source pad is connected.

  It's purpose is to advertise the availability of the sink pads in a way
  that is compatible with "decodebin"
*/
 /* (ew) delete me
static void
cb_source_linked (GstPad * pad, GstPad * peerpad, GstElement * dlna_bin)
{
  GstPad *srcPad;

  g_print("gstdlnabin - cb_source_linked: the source pad was just linked\n");

  // Check the video pad
  srcPad = gst_element_get_pad(dlna_bin, "src-video");
  if (! srcPad) {
    g_printerr ("cb_source_linked: Could not get dlnabin src-video.\n");
    exit(1);
  }
  if (! gst_pad_is_linked(srcPad)) {

    g_print("emitting the signal for the video\n");

    g_signal_emit (G_OBJECT (dlna_bin),
		   gst_dlna_bin_signals[SIGNAL_NEW_DECODED_PAD],
		   0, srcPad, FALSE);
  }
  gst_object_unref(srcPad);

  // Check the audio pad
  srcPad = gst_element_get_pad(dlna_bin, "src-audio");
  if (! srcPad) {
    g_printerr ("cb_source_linked: Could not get dlnabin src-audio.\n");
    exit(1);
  }
  if (! gst_pad_is_linked(srcPad)) {

    g_print("emitting the signal for the audio\n");

    g_signal_emit (G_OBJECT (dlna_bin),
		   gst_dlna_bin_signals[SIGNAL_NEW_DECODED_PAD],
		   0, srcPad, FALSE);
  }
  gst_object_unref(srcPad);
}
 */

/*
  gst_dlna_build_bin

    tee -> videoQueue
        -> audioQueue
	-> ebifQueue
	-> eissQueue
 */
static GstDlnaBin*
gst_dlna_build_bin (GstDlnaBin *dlna_bin)
{
  GstElement *source;
  GstPad *pad, *gpad;

  // Create the source element
  source = gst_element_factory_make ("souphttpsrc", "http-source");

  // Make sure everything is good
  if (!source) {
    g_printerr ("The source element could not be created. Exiting.\n");
    exit(1);
  }

  // Add the elements to the bin 
  gst_bin_add_many(GST_BIN(&dlna_bin->bin), source, NULL);

  // Create the sink ghost pad
  pad = gst_element_get_static_pad(source, "src");
  if (!pad) {
    g_printerr ("Could not get pad for souphttpsrc\n");
    exit(1);
  }
  //gpad = gst_element_get_static_pad(GST_ELEMENT (&dlna_bin->bin), "src");
  gpad = gst_ghost_pad_new("src", pad);
  gst_pad_set_active (gpad, TRUE);
  gst_element_add_pad (GST_ELEMENT (&dlna_bin->bin), gpad);

  gst_object_unref (pad);
  //gst_object_unref (gpad);

  return dlna_bin;
}

/*
static GstDlnaBin*
old_gst_dlna_build_bin (GstDlnaBin *dlna_bin)
{
  GstElement *tee, *videoQueue, *audioQueue, *src;
  GstElement *ebifQueue;
  GstElement *eissQueue;
  GstPad *pad, *gpad;

  // Create the source tee
  tee = gst_dlna_build_source_tee(dlna_bin);

  // Create the video pipeline
  videoQueue = gst_dlna_build_video_pipeline(dlna_bin);

  // Create the audio pipeline
  audioQueue = gst_dlna_build_audio_pipeline(dlna_bin);
  
  // Create the dlna pipeline
  ebifQueue = gst_dlna_build_ebif_pipeline(dlna_bin);
  
  // Create the eiss pipeline
  eissQueue = gst_dlna_build_eiss_pipeline(dlna_bin);

  // Link the elements together
  gst_element_link(tee, videoQueue);
  gst_element_link(tee, audioQueue);
  gst_element_link(tee, ebifQueue);
  gst_element_link(tee, eissQueue);

  // Create the sink ghost pad
  pad = gst_element_get_static_pad(tee, "sink");
  if (!pad) {
    g_printerr ("Could not get pad for tee\n");
    exit(1);
  }
  gpad = gst_ghost_pad_new("sink", pad);
  gst_pad_set_active (gpad, TRUE);
  gst_element_add_pad (GST_ELEMENT (&dlna_bin->bin), gpad);
  gst_object_unref (pad);

  // Catch the linking events
  g_signal_connect (G_OBJECT (gpad), "linked",
		    G_CALLBACK (cb_source_linked), dlna_bin);

  // Create the video src pad
  src = gst_bin_get_by_name(GST_BIN (&dlna_bin->bin), "video-parser");
  if (! src) {
    g_printerr ("Could not find the video src\n");
    exit(1);
  }
  pad = gst_element_get_static_pad(src, "src");
  if (!pad) {
    g_printerr ("Could not get pad for video\n");
    exit(1);
  }
  gpad = gst_ghost_pad_new("src-video", pad);
  gst_pad_set_active (gpad, TRUE);
  gst_element_add_pad (GST_ELEMENT (&dlna_bin->bin), gpad);
  gst_object_unref (pad);

  // Create the audio src pad
  src = gst_bin_get_by_name(GST_BIN (&dlna_bin->bin), "audio-parser");
  if (! src) {
    g_printerr ("Could not find the audio src\n");
    exit(1);
  }
  pad = gst_element_get_static_pad(src, "src");
  if (!pad) {
    g_printerr ("Could not get pad for audio\n");
    exit(1);
  }
  gpad = gst_ghost_pad_new("src-audio", pad);
  gst_pad_set_active (gpad, TRUE);
  gst_element_add_pad (GST_ELEMENT (&dlna_bin->bin), gpad);
  gst_object_unref (pad);

  return dlna_bin;
}
*/


/*
GstDlnaBin*
gst_dlna_rebuild_bin (GstDlnaBin *dlna_bin)
{
  // Release the elements in the old bin
  gst_dlna_empty_bin(&dlna_bin->bin);

  // Rebuild a new bin
  gst_dlna_build_bin(dlna_bin);
  
  return dlna_bin;
}
*/

/*
  gst_dlna_empty_bin

  Remove, and release the elements in the bin
 */
 /*
static void
gst_dlna_empty_bin (GstBin *bin)
{
  GstIterator *it;
  GstElement *item;
  int done = FALSE;

  it = gst_bin_iterate_elements(GST_BIN(bin));
  while (!done) {
    switch (gst_iterator_next (it, (gpointer *)&item)) {
    case GST_ITERATOR_OK:
      gst_bin_remove(GST_BIN (bin), item);
      gst_object_unref (item);
      break;
    case GST_ITERATOR_RESYNC:
      gst_iterator_resync (it);
      break;
    case GST_ITERATOR_ERROR:
      g_printerr
	("gst_dlna_empty_bin - error in the iterator. Exiting.\n");
      exit(1);
      break;
    case GST_ITERATOR_DONE:
      done = TRUE;
      break;
    }
  }
  gst_iterator_free (it);

}
 */

/* 
 * The following section supports the GStreamer auto plugging infrastructure. 
 * Set to 0 if this is done on a package level using (ie gstelements.[hc])
 */
#if 1

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
dlna_bin_init (GstPlugin * dlna_bin)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template ' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_dlna_bin_debug, "dlnabin",
			   0, "MPEG+DLNA Player");

  return gst_element_register ((GstPlugin *)dlna_bin, "dlnabin",
			       GST_RANK_NONE, GST_TYPE_DLNA_BIN);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "dlnabin"
#endif

/* gstreamer looks for this structure to register eisss
 *
 * exchange the string 'Template eiss' with your eiss description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "dlnabin",
    "MPEG+DLNA Decoder",
    (GstPluginInitFunc)dlna_bin_init,
    VERSION,
    "LGPL",
    "gst-cablelabs_ri",
    "http://gstreamer.net/"
)

#endif

/*
static gboolean
gst_dlna_bin_plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_dlna_bin_debug, "dlnabin", 0, "dlna bin");

  return gst_element_register (plugin, "dlnabin", GST_RANK_NONE,
			       GST_TYPE_DLNA_BIN);
}
*/


/*
  Function for marshaling the callback arguments into a function closure.

  Taken from the decodebin code, so we can replicate the interface.
 */

#define g_marshal_value_peek_boolean(v) (v)->data[0].v_int
#define g_marshal_value_peek_object(v) (v)->data[0].v_pointer
#define g_marshal_value_peek_boxed(v) g_value_get_boxed(v)

void
gst_play_marshal_VOID__OBJECT_BOOLEAN (GClosure *closure,
				       GValue *return_value G_GNUC_UNUSED,
				       guint n_param_values,
				       const GValue *param_values,
				       gpointer invocation_hint G_GNUC_UNUSED,
				       gpointer marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_BOOLEAN) (gpointer data1,
						     gpointer arg_1,
						     gboolean arg_2,
						     gpointer data2);
  register GMarshalFunc_VOID__OBJECT_BOOLEAN callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  
  g_return_if_fail (n_param_values == 3);
  
  if (G_CCLOSURE_SWAP_DATA (closure)) {
    data1 = closure->data;
    data2 = g_value_peek_pointer (param_values + 0);
  } else {
    data1 = g_value_peek_pointer (param_values + 0);
    data2 = closure->data;
  }
  callback = (GMarshalFunc_VOID__OBJECT_BOOLEAN)
    (marshal_data ? marshal_data : cc->callback);
  
  callback (data1,
	    g_marshal_value_peek_object (param_values + 1),
	    g_marshal_value_peek_boolean (param_values + 2),
	    data2);
}

void
gst_play_marshal_BOXED__OBJECT_BOXED (GClosure *closure,
				      GValue *return_value G_GNUC_UNUSED,
				      guint n_param_values,
				      const GValue *param_values,
				      gpointer invocation_hint G_GNUC_UNUSED,
				      gpointer marshal_data)
{
  typedef gpointer (*GMarshalFunc_BOXED__OBJECT_BOXED) (gpointer data1,
							gpointer arg_1,
							gpointer arg_2,
							gpointer data2);
  register GMarshalFunc_BOXED__OBJECT_BOXED callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gpointer v_return;
			
  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);
  
  if (G_CCLOSURE_SWAP_DATA (closure)) {
    data1 = closure->data;
    data2 = g_value_peek_pointer (param_values + 0);
  } else {
    data1 = g_value_peek_pointer (param_values + 0);
    data2 = closure->data;
  }
  callback = (GMarshalFunc_BOXED__OBJECT_BOXED)
    (marshal_data ? marshal_data : cc->callback);
			
  v_return = callback (data1,
		       g_marshal_value_peek_object (param_values + 1),
		       g_marshal_value_peek_boxed (param_values + 2),
		       data2);
			
  g_value_take_boxed (return_value, v_return);
}
