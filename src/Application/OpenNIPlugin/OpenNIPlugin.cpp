#include "StableHeaders.h"

#include "OpenNIPlugin.h"
#include "Framework.h"
#include "CoreDefines.h"

//#include <XnOpenNI.h>
//#include <XnCodecIDs.h>
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

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
{
	XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d New User %d\n", epochTime, nId);
	// New user found
	/*if (g_bNeedPose)
	{
		userGenerator_.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
	}
	else
	{
		userGenerator_.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }*/
}
// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
{
	XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d Lost user %d\n", epochTime, nId);	
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

void OpenNIPlugin::Load()
{
    framework_->RegisterDynamicObject("openni", this);
}

int OpenNIPlugin::init() {
    XnStatus nRetVal = XN_STATUS_OK;

    xn::ScriptNode g_ScriptNode;
    xn::DepthGenerator g_DepthGenerator;
    //xn::Recorder* g_pRecorder;
  
    XnCallbackHandle hUserCBs, hCalibrationStartCB, hCalibrationCompleteCB, hPoseCBs;
    /*userGenerator_.RegisterUserCallbacks(NewUser, LostUser, NULL, hUserCBs);
      rc = userGenerator_.GetSkeletonCap().RegisterToCalibrationStart(CalibrationStarted, NULL, hCalibrationStartCB);
      CHECK_RC(rc, "Register to calibration start");
      rc = userGenerator_.GetSkeletonCap().RegisterToCalibrationComplete(CalibrationCompleted, NULL, hCalibrationCompleteCB);
      CHECK_RC(rc, "Register to calibration complete");
      rc = userGenerator_.GetPoseDetectionCap().RegisterToPoseDetected(PoseDetected, NULL, hPoseCBs);
      CHECK_RC(rc, "Register to pose detected");*/

    XnStatus rc = XN_STATUS_OK;
    xn::EnumerationErrors errors;

    rc = context_.InitFromXmlFile(SAMPLE_XML_PATH, g_ScriptNode, &errors);
    CHECK_ERRORS(rc, errors, "InitFromXmlFile");
    CHECK_RC(rc, "InitFromXml");

    rc = context_.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
    CHECK_RC(rc, "Find depth generator");
    rc = context_.FindExistingNode(XN_NODE_TYPE_USER, userGenerator_);
    CHECK_RC(rc, "Find user generator");

    if (!userGenerator_.IsCapabilitySupported(XN_CAPABILITY_SKELETON) ||
        !userGenerator_.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
    {
        printf("User generator doesn't support either skeleton or pose detection.\n");
        return XN_STATUS_ERROR;
    }

    nRetVal = context_.FindExistingNode(XN_NODE_TYPE_USER, userGenerator_);
    if (nRetVal != XN_STATUS_OK)
    {
        nRetVal = userGenerator_.Create(context_);
        CHECK_RC(nRetVal, "Find user generator");
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected, hCalibrationInProgress, hPoseInProgress;
    if (!userGenerator_.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        printf("Supplied user generator doesn't support skeleton\n");
        return 1;
    }
    nRetVal = userGenerator_.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    /*nRetVal = userGenerator_.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = userGenerator_.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");*/

    userGenerator_.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
    rc = context_.StartGeneratingAll();
    CHECK_RC(rc, "StartGenerating");

    printf("OpenNI: init complete.\n");
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
  //g_scriptNode.Release();
  //g_DepthGenerator.Release();
  //userGenerator_.Release();
  //g_Player.Release();
  //context_.Release();
}

void OpenNIPlugin::Update(f64 frametime)
{
    context_.WaitOneUpdateAll(userGenerator_);
}

float2 OpenNIPlugin::GetUserPos()
{
    XnUserID aUsers[15];
    XnUInt16 nUsers = 15;
    userGenerator_.GetUsers(aUsers, nUsers);

    XnPoint3D com;
    for (int i = 0; i < nUsers; ++i)
    {
        userGenerator_.GetCoM(aUsers[i], com);
        //g_DepthGenerator.ConvertRealWorldToProjective(1, &com, &com);
        //glRasterPos2i(com.X, com.Y);
        //printf("%f -- ", com.X);
        //printf("%f\n", com.Y);
    }

    float2 pos;
    pos.x = com.X;
    pos.y = com.Y;
    return pos;
}

QVariantList OpenNIPlugin::test()
{
    QVariantList ret;

    XnUserID aUsers[15];
    XnUInt16 nUsers = 15;
    userGenerator_.GetUsers(aUsers, nUsers);

    XnPoint3D com;
    float2 pos;
    for (int i = 0; i < nUsers; ++i)
    {
        userGenerator_.GetCoM(aUsers[i], com);
        pos.x = com.X;
        pos.y = com.Y;
        ret.push_back(QVariant(pos));
    }
    
    return ret;
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
