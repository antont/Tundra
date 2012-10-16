#pragma once

#include "IModule.h"
#include "OpenNIAPI.h"
#include "OpenNIFwd.h"

#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>

#include <QObject>

class OPENNI_PLUGIN_API OpenNIPlugin : public IModule
{

Q_OBJECT

public:
    /// Constructor
    OpenNIPlugin();

    /// Deconstructor
    virtual ~OpenNIPlugin();

    /// IModule override
    void Initialize();

    /// IModule override
    void Uninitialize();
};

