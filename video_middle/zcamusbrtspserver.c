/**
  * usb camera(yuv) -> h264enc -> rtsp server.
  * gcc zcamusbrtspserver.c -o zcamusbrtspserver.bin $(pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0)
  * tcp ports list
  * 6803: left view camera.(ethernet)
  * 6805: right view camera.(ethernet)
  * 6807: middle view camera.(usb)
  */

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEFAULT_RTSP_PORT "6807"
#define PIPE_LINE " ( v4l2src device=/dev/video1 ! video/x-raw,width=(int)640,height=(int)480,framerate=(fraction)30/1 ! queue ! nvvidconv ! omxh264enc ! rtph264pay name=pay0 pt=96 )"
static char *port=(char*)DEFAULT_RTSP_PORT;

static GOptionEntry entries[] = {
  {"port", 'p', 0, G_OPTION_ARG_STRING, &port,
      "Port to listen on (default: " DEFAULT_RTSP_PORT ")", "PORT"},
  {NULL}
};

int main(int argc,char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;
  GOptionContext *optctx;
  GError *error = NULL;
  int fd;
  char buffer[64];

  //create pid file.
  fd=open("/tmp/ZCamUsbRtspServer.pid",O_CREAT|O_TRUNC|O_WRONLY,0644);
  if(fd<0)
  {
      printf("<error>:failed to create /tmp/ZCamUsbRtspServer.pid file.");
      return -1;
  }
  memset(buffer,0,sizeof(buffer));
  sprintf(buffer,"%d",getpid());
  if(write(fd,buffer,strlen(buffer))<0)
  {
      printf("<error>:failed to write /tmp/ZCamUsbRtspServer.pid file.");
      return -1;
  }
  close(fd);

  //execute GStreamer.
  gst_init(NULL,NULL);

#if 0
  optctx = g_option_context_new ("<launch line> - Test RTSP Server, Launch\n\n"
      "Example: \"( videotestsrc ! x264enc ! rtph264pay name=pay0 pt=96 )\"");
  g_option_context_add_main_entries (optctx, entries, NULL);
  g_option_context_add_group (optctx, gst_init_get_option_group ());
//  if (!g_option_context_parse (optctx, &argc, &argv, &error)) {
//    g_printerr ("Error parsing options: %s\n", error->message);
//    g_option_context_free (optctx);
//    g_clear_error (&error);
//    return -1;
//  }
  g_option_context_free (optctx);
#endif


  loop = g_main_loop_new (NULL, FALSE);

  /* create a server instance */
  server = gst_rtsp_server_new ();
  g_object_set (server, "service", port, NULL);

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  mounts = gst_rtsp_server_get_mount_points (server);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  factory = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_launch (factory, PIPE_LINE/*argv[1]*/);
  gst_rtsp_media_factory_set_shared (factory, TRUE);

  /* attach the test factory to the /stream url */
  gst_rtsp_mount_points_add_factory (mounts, "/stream", factory);

  /* don't need the ref to the mapper anymore */
  g_object_unref (mounts);

  /* attach the server to the default maincontext */
  gst_rtsp_server_attach (server, NULL);

  /* start serving */
  g_print ("stream ready at rtsp://127.0.0.1:%s/stream\n", port);
  g_main_loop_run (loop);

  return 0;
}
