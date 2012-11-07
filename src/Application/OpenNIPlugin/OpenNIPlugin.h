#pragma once

#include "IModule.h"
#include "OpenNIAPI.h"
#include "OpenNIFwd.h"
#include <XnCppWrapper.h>

#include <QObject>
#include "Math/float2.h"

class OPENNI_PLUGIN_API OpenNIPlugin : public IModule
{

Q_OBJECT

public:
    /// Constructor
    OpenNIPlugin();

    /// Deconstructor
    virtual ~OpenNIPlugin();

    /// IModule override.
    void Load();

    /// IModule override
    void Initialize();

    /// IModule override
    void Uninitialize();

    /// IModule override
    void Update(f64 frametime);

public slots:
    float2 GetUserPos();

private:
    xn::Context context_;
    xn::UserGenerator userGenerator_;
    
    int init();
};

