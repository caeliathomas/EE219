//
// mxNI.cpp : A simple interface to the OpenNI 2.2 library
// Camillo J. Taylor
// GRASP Laboratory
// University of Pennsylvania
// June 6 2013
//
//Modified 
//Feb. 2022

#include "mex.h"
#include "math.h"
#include <OpenNI.h>

using namespace openni;

// *** Static global variables ***************************************/

static bool initialized = false;
static Device device;
static VideoStream depth, color, ir;

//********************************************************************/

static const char usage[] =  " \n\
The mxNI function operates as a switchyard function where the first operand, opid, should be an integer \n\
which selects the operation to be performed. \n\n\
Possible operations: \n\
\n\
mxNI(0) - Initializes the device - nothing returned \n\
mxNI(1) - Closes the device - nothing returned \n\
[frame, frame_index, timestamp] = mxNI(2) - Acquires a depth frame \n\
[frame, frame_index, timestamp] = mxNI(3) - Acquires an image frame \n\
mxNI(4) - Toggle image to depth registration off \n\
mxNI(5) - Toggle image to depth registration on \n\
[frame, frame_index, timestamp] = mxNI(6) - Acquires an ir frame \n\
mxNI(7) - Enumerate all available ir videoModes\n\
\n\
mxNI(10) - Print out current device settings \n\
mxNI(11) - Enumerate all available depth videoModes \n\
mxNI(12) - Enumerate all available color videoModes \n\
mxNI(13, <videoModeId>) - Set the depth video mode \n\
mxNI(14, <videoModeId>) - Set the color video mode \n\
mxNI(15, <videoModeId>) - Set the ir video mode \n\
";

//********************************************************************/

void close (void)
{
  mexPrintf ("Closing the NI device\n");

  depth.stop();
  depth.destroy();

  color.stop();
  color.destroy();

  ir.stop();
  ir.destroy();

  device.close();
  OpenNI::shutdown();

  initialized = false;  
}

void init (void)
{
  Status rc;

  if (!initialized) {

    rc = OpenNI::initialize();
    if (rc != STATUS_OK) {
      mexPrintf("Initialize failed\n %s \n", OpenNI::getExtendedError());
      mexErrMsgTxt("mxNI init failed");
    }

    rc = device.open(ANY_DEVICE);
    if (rc != STATUS_OK) {
      mexPrintf("Couldn't open device\n %s \n", OpenNI::getExtendedError());
      close();
      mexErrMsgTxt("mxNI init failed");
    }

    if (device.getSensorInfo(SENSOR_DEPTH) != NULL) {
      rc = depth.create(device, SENSOR_DEPTH);
      if (rc != STATUS_OK) {
	printf("Couldn't create depth stream\n %s \n", OpenNI::getExtendedError());
	close();
	mexErrMsgTxt("mxNI init failed");		
      }
    }

    rc = depth.start();
    if (rc != STATUS_OK) {
      printf("Couldn't start the depth stream\n %s \n", OpenNI::getExtendedError());
      close();
      mexErrMsgTxt("mxNI init failed");
    }


    if (device.getSensorInfo(SENSOR_COLOR) != NULL) {
      rc = color.create(device, SENSOR_COLOR);
      if (rc != STATUS_OK) {
	printf("Couldn't create color stream\n %s \n", OpenNI::getExtendedError());
	close();
	mexErrMsgTxt("mxNI init failed");		
      }
    }

    rc = color.start();
    if (rc != STATUS_OK) {
      printf("Couldn't start the color stream\n %s \n", OpenNI::getExtendedError());
      close();
      mexErrMsgTxt("mxNI init failed");
    }

 //IR videostream initialization

    if (device.getSensorInfo(SENSOR_IR) != NULL) {
      rc = ir.create(device, SENSOR_IR);
      if (rc != STATUS_OK) {
	printf("Couldn't create IR stream\n %s \n", OpenNI::getExtendedError());
	close();
	mexErrMsgTxt("mxNI init failed");		
      }
    }

    rc = ir.start();
    if (rc != STATUS_OK) {
      printf("Couldn't start the ir stream\n %s \n", OpenNI::getExtendedError());
      close();
      mexErrMsgTxt("mxNI init failed");
    }

    initialized = true;
    mexAtExit (close);
  }
}

void PrintPixelFormat (PixelFormat pf)
{
  switch(pf) {
  case PIXEL_FORMAT_DEPTH_1_MM: mexPrintf("PIXEL_FORMAT_DEPTH_1_MM"); break;
  case PIXEL_FORMAT_DEPTH_100_UM: mexPrintf("PIXEL_FORMAT_DEPTH_100_UM"); break;
  case PIXEL_FORMAT_SHIFT_9_2: mexPrintf("PIXEL_FORMAT_SHIFT_9_2"); break;
  case PIXEL_FORMAT_SHIFT_9_3: mexPrintf("PIXEL_FORMAT_SHIFT_9_3"); break;

  case PIXEL_FORMAT_RGB888: mexPrintf("PIXEL_FORMAT_RGB888"); break;
  case PIXEL_FORMAT_YUV422: mexPrintf("PIXEL_FORMAT_YUV422"); break;
  case PIXEL_FORMAT_GRAY8: mexPrintf("PIXEL_FORMAT_GRAY8"); break;
  case PIXEL_FORMAT_GRAY16: mexPrintf("PIXEL_FORMAT_GRAY16"); break;
  case PIXEL_FORMAT_JPEG: mexPrintf("PIXEL_FORMAT_JPEG"); break;
  case PIXEL_FORMAT_YUYV: mexPrintf("PIXEL_FORMAT_YUYV"); break;
  }
}

void PrintVideoMode (const VideoMode vmode)
{
  mexPrintf ("Fps = %d\n", vmode.getFps());
  mexPrintf ("pixelFormat = ");
  PrintPixelFormat(vmode.getPixelFormat());
  mexPrintf("\n");
  mexPrintf ("resX = %d\n", vmode.getResolutionX());
  mexPrintf ("resY = %d\n", vmode.getResolutionY());
}

void PrintAllVideoModes (const VideoStream &s)
{
  const Array<VideoMode> &VideoModes = (s.getSensorInfo()).getSupportedVideoModes();

  for (int i=0; i < VideoModes.getSize(); ++i) {
    mexPrintf ("\n\n Mode  %d\n\n", i);
    PrintVideoMode (VideoModes[i]);
  }
}

void PrintVideoStreamInfo (const VideoStream &s)
{
  mexPrintf ("HFOV = %8.5f\n", s.getHorizontalFieldOfView());
  mexPrintf ("VFOV = %8.5f\n", s.getVerticalFieldOfView());
  mexPrintf ("MaxPixelValue = %d\n", s.getMaxPixelValue());
  mexPrintf ("MinPixelValue = %d\n", s.getMinPixelValue());

  mexPrintf ("\nVideo Mode ...\n");
  PrintVideoMode(s.getVideoMode());
}

void SetVideoMode (VideoStream &s, int videoModeId)
{
  Status rc;
  const Array<VideoMode> &VideoModes = (s.getSensorInfo()).getSupportedVideoModes();

  if ( (videoModeId < 0) || (videoModeId >= VideoModes.getSize()) ) {
    mexErrMsgTxt("mxNI SetVideoMode invalid videoModeId");
  }
  
//  printf("%s", VideoModes.getSize());

  s.stop();
  rc = s.setVideoMode (VideoModes[videoModeId]);
  s.start();

  if (rc != STATUS_OK) {
    printf("Couldn't reset videoMode \n %s \n", OpenNI::getExtendedError());
    mexErrMsgTxt("mxNI failed");		
  }
}

/*
void SetPixelFormat (VideoStream &s, int Pixelformat)
{
  Status rc;
  const Array<VideoMode> &VideoModes = (s.getSensorInfo()).getSupportedVideoModes();

  if ( (videoModeId < 0) || (videoModeId >= VideoModes.getSize()) ) {
    mexErrMsgTxt("mxNI SetVideoMode invalid videoModeId");
  }

  s.stop();
  rc = s.setVideoMode (VideoModes[videoModeId]);
  s.start();

  if (rc != STATUS_OK) {
    printf("Couldn't reset videoMode \n %s \n", OpenNI::getExtendedError());
    mexErrMsgTxt("mxNI failed");		
  }
}

*/
void getFrame (VideoStream &s,  int nlhs, mxArray *plhs[])
{
  Status rc;
  VideoFrameRef frame;
  int w, h, i, j, k;
  mwSize dims[3];
  char *ptr8_1, *ptr8_2;
  uint16_t *ptr16_1, *ptr16_2;

  if (nlhs > 3) {
    mexErrMsgTxt("mxNI getFrame returns at most 3 values: frame, frame_index, timestamp");		
  }

  rc = s.readFrame(&frame);

  if (rc != STATUS_OK) {
    printf("mxNI problems reading a frame \n %s \n", OpenNI::getExtendedError());
    mexErrMsgTxt("mxNI failed");		
  }

  if (nlhs > 0) { // return frame
    w = frame.getWidth();
    h = frame.getHeight();

    switch ((frame.getVideoMode()).getPixelFormat()) {

    case PIXEL_FORMAT_DEPTH_1_MM:
    case PIXEL_FORMAT_DEPTH_100_UM:
    case PIXEL_FORMAT_GRAY16:
      plhs[0] = mxCreateNumericMatrix(h, w, mxUINT16_CLASS, mxREAL);
      ptr16_1 = (uint16_t*) mxGetData(plhs[0]);
      ptr16_2 = (uint16_t*) frame.getData();

      for (i=0; i < h; ++i)
	for (j=0; j < w; ++j)
	  ptr16_1[j*h + i] = ptr16_2[i*w + j];
 
      break;

    case PIXEL_FORMAT_YUV422:
    case PIXEL_FORMAT_GRAY8:
      plhs[0] = mxCreateNumericMatrix(h, w, mxUINT8_CLASS, mxREAL);
      ptr8_1 = (char*) mxGetData(plhs[0]);
      ptr8_2 = (char*) frame.getData();

      for (i=0; i < h; ++i)
	for (j=0; j < w; ++j)
	  ptr8_1[j*h + i] = ptr8_2[i*w + j];

      break;

    case PIXEL_FORMAT_RGB888:
      dims[0] = h; dims[1] = w; dims[2] = 3;
      plhs[0] = mxCreateNumericArray(3, dims, mxUINT8_CLASS, mxREAL);
      ptr8_1 = (char*) mxGetData(plhs[0]);
      ptr8_2 = (char*) frame.getData();

      for (i=0; i < h; ++i)
	for (j=0; j < w; ++j)
	  for (k = 0; k < 3; ++k)
	    ptr8_1[k*h*w + j*h + i] = ptr8_2[i*3*w + j*3 + k];

      break;

    default:
      mexErrMsgTxt("mxNI: getFrame don't know what to do with this frames' pixelFormat");		
      break;
    }
  }

  if (nlhs > 1) { // return frame index
    plhs[1] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
    *((uint64_t*) mxGetData(plhs[1])) = frame.getFrameIndex();
  }

  if (nlhs > 2) { // return frame timestamp
    plhs[2] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
    *((uint64_t*) mxGetData(plhs[2])) = frame.getTimestamp();
  }
}



#define MSG_ID "mxNI:arg"

/*
 * This function tests whether the specified argument is a 2D double array or scalar. You can
 * use the two arguments, nrows and ncols, to test the size of the array or set them to 0 if you don't care.
 */

double *checkArgDoubleArray(const mxArray *prhs[], int arg_index, const char *error_msg, int nrows, int ncols) {
  const mxArray *in = prhs[arg_index];
    
  if (!mxIsDouble(in) || mxIsComplex(in) || (mxGetNumberOfDimensions(in) != 2)) {
    mexErrMsgIdAndTxt(MSG_ID, error_msg);
  }
    
  if ((nrows >= 1) && (mxGetM(in) != nrows)) {
    mexErrMsgIdAndTxt(MSG_ID, error_msg);
  }
    
  if ((ncols >= 1) && (mxGetN(in) != ncols)) {
    mexErrMsgIdAndTxt(MSG_ID, error_msg);
  }
    
  return (mxGetPr(in));
}

/* The matlab mex function */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] ) {
  int opid, videoModeId, fps, pF, xres, yres;

  if (nrhs < 1) {
    mexPrintf("\n%s\n", usage);
    mexErrMsgIdAndTxt("mxNI:nrhs", "At least one input required opid");
  }

  // First argument is an integer indicating the operation to be performed

  opid = (int) ( *checkArgDoubleArray(prhs, 0, "First argument, opid, should be a scalar index", 1, 1) );

  switch (opid) {

  case 0:
    // Init
    if (initialized) {
      mexPrintf("Already Initialized\n");
    } else {
      init();
      mexPrintf("Initialized\n");
    }

    break;

  case 1:  
    // Close
    close();
    break;

  case 2:
    // Get Depth Image
    init();
    getFrame (depth, nlhs, plhs); 
    break;

  case 3:
    // Get Color Image
    init();
    getFrame (color, nlhs, plhs); 
    break;

  case 4:
    device.setImageRegistrationMode (IMAGE_REGISTRATION_OFF);
    break;

  case 5:
    device.setImageRegistrationMode (IMAGE_REGISTRATION_DEPTH_TO_COLOR);
    break;

  case 6:
    // Get IR Image
    init();
    getFrame (ir, nlhs, plhs); 
    break;
  
  case 7:
    // Print info about all of the available videoModes
    mexPrintf ("\nIR Video Modes ... \n");
    PrintAllVideoModes(ir);
    break;

/*  case 8:
    //Print info about Image registration
    device.isImageRegistrationModeSupported()
    break;
*/

  case 10:
    // Print info about the current device settings
    mexPrintf ("Device Name = %s \n", device.getDeviceInfo().getName());
    mexPrintf ("Device Vendor = %s \n", device.getDeviceInfo().getVendor());

    if (device.getImageRegistrationMode() == IMAGE_REGISTRATION_DEPTH_TO_COLOR)
      mexPrintf ("Image Registration Mode : DEPTH TO COLOR\n");
    else
      mexPrintf ("Image Registration Mode : OFF\n");


    mexPrintf ("\nDepth Sensor ...\n");
    PrintVideoStreamInfo (depth);

    mexPrintf ("\nColor Sensor ...\n");
    PrintVideoStreamInfo (color);

    mexPrintf ("\nIR Sensor ...\n");
    PrintVideoStreamInfo (ir);

    break;

  case 11:
    // Print info about all of the available videoModes
    mexPrintf ("\nDepth Video Modes ... \n");
    PrintAllVideoModes(depth);
    break;

  case 12:
    // Print info about all of the available videoModes
    mexPrintf ("\nColor Video Modes ... \n");
    PrintAllVideoModes(color);
    break;

  case 13:
  case 14:
  case 15:
    // Set video mode
    if (nrhs < 2) {
      mexErrMsgIdAndTxt("mxNI:nrhs", "At least two inputs required opid and videoModeId");
    }

    // Second argument is an integer indicating the desired video mode
    videoModeId = (int) ( *checkArgDoubleArray(prhs, 1, "Second argument, videoModeId, should be a scalar index", 1, 1) );

    if (opid == 13)
      SetVideoMode (depth, videoModeId);
    //Add a success message here
    else if (opid == 14)
      SetVideoMode (color, videoModeId);
    else 
      SetVideoMode (ir, videoModeId);

    break;


  default:
    mexPrintf ("Invalid opid %d\n", opid);
    mexPrintf("\n%s\n", usage);
    break;
  }
}
