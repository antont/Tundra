#include "StableHeaders.h"

#include "OpenNIPlugin.h"
#include "Framework.h"
#include "CoreDefines.h"

#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>

#define SAMPLE_XML_PATH "data/openni/Sample-User.xml"

#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
		return rc;													\
	}

#define CHECK_ERRORS(rc, errors, what)		\
	if (rc == XN_STATUS_NO_NODE_PRESENT)	\
{										\
	XnChar strError[1024];				\
	errors.ToString(strError, 1024);	\
	printf("%s\n", strError);			\
	return (rc);						\
}


OpenNIPlugin::OpenNIPlugin() : 
  IModule("OpenNIPlugin")
{       
}

OpenNIPlugin::~OpenNIPlugin()
{
  /*SAFE_DELETE(videoPreviewLabel_);
    SAFE_DELETE(depthPreviewLabel_);
    SAFE_DELETE(skeletonPreviewLabel_);*/
}

int init() {
  xn::Context g_Context;
  xn::ScriptNode g_ScriptNode;
  xn::DepthGenerator g_DepthGenerator;
  xn::UserGenerator g_UserGenerator;
  xn::Recorder* g_pRecorder;
  
  XnCallbackHandle hUserCBs, hCalibrationStartCB, hCalibrationCompleteCB, hPoseCBs;
  /*g_UserGenerator.RegisterUserCallbacks(NewUser, LostUser, NULL, hUserCBs);
  rc = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(CalibrationStarted, NULL, hCalibrationStartCB);
  CHECK_RC(rc, "Register to calbiration start");
  rc = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(CalibrationCompleted, NULL, hCalibrationCompleteCB);
  CHECK_RC(rc, "Register to calibration complete");
  rc = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(PoseDetected, NULL, hPoseCBs);
  CHECK_RC(rc, "Register to pose detected");*/

  XnStatus rc = XN_STATUS_OK;
  xn::EnumerationErrors errors;

  rc = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, g_ScriptNode, &errors);
  CHECK_ERRORS(rc, errors, "InitFromXmlFile");
  CHECK_RC(rc, "InitFromXml");

  rc = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
  CHECK_RC(rc, "Find depth generator");
  rc = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
  CHECK_RC(rc, "Find user generator");

  if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON) ||
      !g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
    {
      printf("User generator doesn't support either skeleton or pose detection.\n");
      return XN_STATUS_ERROR;
    }

  g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
  rc = g_Context.StartGeneratingAll();
  CHECK_RC(rc, "StartGenerating");

  return 0;
}

void OpenNIPlugin::Initialize()
{
  init();

      XnUserID g_nPlayer = 0;
      XnBool g_bCalibrated = FALSE;
}

void OpenNIPlugin::Uninitialize()
{
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new OpenNIPlugin();
        fw->RegisterModule(module);
    }
}
