/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Environment.cpp
 *  @brief  Manages environment-related reX-logic, e.g. world time and lighting.
 */

#include "StableHeaders.h"

#include "Environment.h"
#include "EnvironmentModule.h"

#include "EC_OgreEnvironment.h"
#include <SceneManager.h>
#include "NetworkMessages/NetInMessage.h"
#include "EC_WaterPlane.h"
#include "OgreRenderingModule.h"

#ifdef CAELUM
#include <Caelum.h>
#endif

namespace Environment
{

/// Utility tool for clamping fog distance
void ClampFog(float& start, float& end, float farclip)
{
    if (farclip < 10.0) 
        farclip = 10.0;
    if (end > farclip - 10.0)
        end = farclip - 10.0;
    if (start > farclip/3.0)
        start = farclip/3.0;
}


Environment::Environment(EnvironmentModule *owner) :
    owner_(owner),
    activeEnvEntity_(Scene::EntityWeakPtr()),
    time_override_(false),
    usecSinceStart_(0),
    secPerDay_(0),
    secPerYear_(0),
    sunDirection_(RexTypes::Vector3()),
    sunPhase_(0.0),
    sunAngVelocity_(RexTypes::Vector3())
  
{}

Environment::~Environment()
{
    if ( activeEnvEntity_.lock().get() != 0)
    {
        Scene::EntityPtr entity = activeEnvEntity_.lock();
        if ( entity != 0)
        {
            // Remove entity from scene.
            Scene::SceneManager* manager = entity->GetScene();
            if ( manager != 0)
            {
                manager->RemoveEntity(entity->GetId());
                
            }
        }
        entity.reset();
    }

    activeEnvEntity_.reset();
    owner_ = 0;
    activeFogComponent_ = 0;
}

OgreRenderer::EC_OgreEnvironment* Environment::GetEnvironmentComponent()
{
    if (activeEnvEntity_.expired())
        return 0;
    
    OgreRenderer::EC_OgreEnvironment* ec = activeEnvEntity_.lock()->GetComponent<OgreRenderer::EC_OgreEnvironment>().get();  
    return ec;
    
}


Scene::EntityWeakPtr Environment::GetEnvironmentEntity()
{
    return activeEnvEntity_;
}

void Environment::CreateEnvironment()
{
  
    Scene::ScenePtr active_scene = owner_->GetFramework()->GetDefaultWorldScene();
    Scene::EntityPtr entity = active_scene->CreateEntity(active_scene->GetNextFreeIdLocal());

    // Creates Caelum component!!
    entity->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent("EC_OgreEnvironment"));
    
    // Creates default fog component
    entity->AddComponent(owner_->GetFramework()->GetComponentManager()->CreateComponent(EC_Fog::TypeNameStatic()));
    activeFogComponent_ = entity->GetComponent<EC_Fog >().get();
    
    active_scene->EmitEntityCreated(entity);
    activeEnvEntity_ = entity;
    
}

bool Environment::HandleSimulatorViewerTimeMessage(ProtocolUtilities::NetworkEventInboundData *data)
{
    ProtocolUtilities::NetInMessage &msg = *data->message;
    msg.ResetReading();

    
    // SimulatorViewerTimeMessage seems to be corrupted with 0.4 & 0.5 servers.
    try
    {
        usecSinceStart_ = (time_t)msg.ReadU64();
        secPerDay_ = msg.ReadU32();
        secPerYear_ = msg.ReadU32();
        sunDirection_ = msg.ReadVector3();
        sunPhase_ = msg.ReadF32();
        sunAngVelocity_ = msg.ReadVector3();
    }
    catch(NetMessageException &)
    {
        //! todo crashes with 0.4 server add error message.
    }
    
    // Calculate time of day from sun phase, which seems the most reliable way to do it
    float dayphase;
    
    if (sunPhase_ < 1.25 * PI)
        dayphase = (sunPhase_ / PI + 1.0) * 0.33333333;
    else
        dayphase = -0.5 + (sunPhase_ / PI);

    
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return false; 

    // Update the sunlight direction and angle velocity.
    ///\note Not needed anymore as we use Caelum now.
    //OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
    //    (component.get());
    //env.SetSunDirection(-sunDirection_);

    if (!time_override_)
        env->SetTime(dayphase);
    
    return false;
}



void Environment::SetGroundFog(float fogStart, float fogEnd, const QVector<float>& color)
{
    if ( activeFogComponent_ == 0)
        return;

    activeFogComponent_->startDistanceAttr.Set(fogStart,AttributeChange::Local);
    activeFogComponent_->endDistanceAttr.Set(fogEnd, AttributeChange::Local);
    activeFogComponent_->colorAttr.Set(Color(color[0], color[1], color[2], 1.0), AttributeChange::Local);

    emit GroundFogAdjusted(fogStart, fogEnd, color);
}

void Environment::SetFogColorOverride(bool enabled)
{
    if ( activeFogComponent_ == 0)
        return;
    
    activeFogComponent_->useAttr.Set(enabled, AttributeChange::Local);

    
}

bool Environment::GetFogColorOverride() 
{
    if ( activeFogComponent_ == 0)
        return false;
    
   return activeFogComponent_->useAttr.Get();


    
}

QVector<float> Environment::GetFogGroundColor()
{
    if ( activeFogComponent_ == 0)
        return QVector<float>();

    Ogre::ColourValue color = activeFogComponent_->GetColorAsOgreValue();
    QVector<float> vec; 
    vec<<color[0]<<color[1]<<color[2];
    return vec;
      
}



void Environment::Update(f64 frametime)
{
    // We are still little depend of old EC_OgreEnvironment component,
    /// todo refactor away depency of old EC_OgreEnvironmen component 

    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;


     // Currently updates other then water and fog.
    env->UpdateVisualEffects(frametime);

#ifdef CAELUM
    Caelum::CaelumSystem* caelumSystem = env->GetCaelum();
#endif

    boost::shared_ptr<OgreRenderer::Renderer> renderer = owner_->GetFramework()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    
    if (!renderer)
       return;
   
    
    Ogre::Camera *camera = renderer.get()->GetCurrentCamera();
    Ogre::Viewport *viewport = renderer.get()->GetViewport();
    Ogre::SceneManager *sceneManager = renderer.get()->GetSceneManager();
    float cameraFarClip = renderer.get()->GetViewDistance();
    
     
    // Go through all water components.

    Scene::ScenePtr scene = owner_->GetFramework()->GetDefaultWorldScene();
    Scene::EntityList lst = scene->GetEntitiesWithComponent(EC_WaterPlane::TypeNameStatic());
    Scene::EntityList::iterator iter = lst.begin();

    bool underWater = false;
    EC_WaterPlane* plane = 0;
    for (; iter != lst.end(); ++iter)
    {
        Scene::EntityPtr ent = *iter;
        Scene::Entity* e = ent.get();
        plane = e->GetComponent<EC_WaterPlane >().get();        
        if ( plane != 0 && plane->IsUnderWater() )
        {
            underWater = true;
            break;
        }
    }
    
    // Ground fog value. 
    // Caelum calculates this value depending of time ( sun position? )
    Ogre::ColourValue fogColor;

#ifdef CAELUM
       fogColor = caelumSystem->getGroundFog()->getColour();
#else
    if ( activeFogComponent_ != 0)
        fogColor = activeFogComponent_->GetColorAsOgreValue();
#endif 

    if ( activeFogComponent_ != 0 && activeFogComponent_->useAttr.Get())
    {
       fogColor = activeFogComponent_->GetColorAsOgreValue();
    }

    if ( underWater )
    {
       // We're below the water.
       
       float fogStart =plane->fogStartAttr.Get();
       float fogEnd = plane->fogEndAttr.Get();
       float farClip = fogEnd+10.f;
       Ogre::FogMode mode = static_cast<Ogre::FogMode>(plane->fogModeAttr.Get());
       
       if (farClip > cameraFarClip)
           farClip = cameraFarClip;            
    
       ClampFog(fogStart, fogEnd, farClip);            
#ifdef CAELUM
            // Hide the Caelum subsystems.
            caelumSystem->forceSubcomponentVisibilityFlags(Caelum::CaelumSystem::CAELUM_COMPONENTS_NONE);
#endif    
            Color col = plane->fogColorAttr.Get();
            Ogre::ColourValue color = plane->GetFogColorAsOgreValue();
            
            //@note default values are 0.2f, 0.4f, 0.35f
            sceneManager->setFog(mode, fogColor * color, 0.001f, fogStart, fogEnd);
            viewport->setBackgroundColour(fogColor *  color);
            camera->setFarClipDistance(farClip);

    }
    else
    {
            float fogStart = 100.f;
            float fogEnd = 2000.f;
            Ogre::FogMode mode = Ogre::FOG_LINEAR;
            if ( activeFogComponent_ != 0)
            {
                fogStart = activeFogComponent_->startDistanceAttr.Get();
                fogEnd = activeFogComponent_->endDistanceAttr.Get();
                mode = static_cast<Ogre::FogMode>(activeFogComponent_->modeAttr.Get());
            }
          
            ClampFog(fogStart, fogEnd, cameraFarClip);
//#ifdef CAELUM
//            caelumSystem_->forceSubcomponentVisibilityFlags(caelumComponents_);
//#endif
            sceneManager->setFog(mode, fogColor, 0.001f, fogStart, fogEnd);
            viewport->setBackgroundColour(fogColor);
            camera->setFarClipDistance(cameraFarClip);

    }
 

}

bool Environment::IsCaelum()
{
    
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return false;
        
    return env->IsCaelumUsed();
    
}

void Environment::SetGroundFogColor(const QVector<float>& color)
{
   if ( activeFogComponent_ == 0)
        return;

   activeFogComponent_->colorAttr.Set(Color(color[0], color[1], color[2], 1.0), AttributeChange::Local);
 
}



void Environment::SetGroundFogDistance(float fogStart, float fogEnd)
{
    if ( activeFogComponent_ == 0)
        return;

    activeFogComponent_->startDistanceAttr.Set(fogStart, AttributeChange::Local);
    activeFogComponent_->endDistanceAttr.Set(fogEnd, AttributeChange::Local);
     
}

float Environment::GetGroundFogStartDistance()
{
    
    if ( activeFogComponent_ == 0)
        return 0.f;
    
    return activeFogComponent_->startDistanceAttr.Get();

}

float Environment::GetGroundFogEndDistance()
{
    if ( activeFogComponent_ == 0)
        return 0.f;
    
    return activeFogComponent_->endDistanceAttr.Get();

}

void Environment::SetSunDirection(const QVector<float>& vector)
{
    
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;
    // Assure that we do not given too near of zero vector values, a la HACK. 
    float squaredLength = vector[0] * vector[0] + vector[1]* vector[1] + vector[2] * vector[2];
    // Length must be diffrent then zero, so we say that value must be higher then our tolerance. 
    float tolerance = 0.001f;

    if ( squaredLength > tolerance) 
       env->SetSunDirection(Vector3df(vector[0], vector[1], vector[2]));
    
    
}

QVector<float> Environment::GetSunDirection() 
{
    
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return QVector<float>(3);

    Vector3df vec = env->GetSunDirection();
    QVector<float> vector;
    vector.append(vec.x), vector.append(vec.y), vector.append(vec.z);
    return vector;
    
}

void Environment::SetSunColor(const QVector<float>& vector)
{
    
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;

    if ( vector.size() == 4)
    {
        Color color(vector[0], vector[1], vector[2], vector[3]);
        env->SetSunColor(color);
    }
    else if ( vector.size() == 3 )
    {
        Color color(vector[0], vector[1], vector[2], 1.0);
        env->SetSunColor(color);
    }
    
}

QVector<float> Environment::GetSunColor()
{
    
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return QVector<float>(4);

    Color color = env->GetSunColor();
    QVector<float> vec(4);
    vec[0] = color.r, vec[1] = color.g, vec[2] = color.b, vec[3] = color.a;
    return vec;
    
}

QVector<float> Environment::GetAmbientLight()
{
    
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return QVector<float>(3);

   Color color = env->GetAmbientLightColor(); 
   QVector<float> vec(3);
   vec[0] = color.r, vec[1] = color.g, vec[2] = color.b;
   return vec;
   
 
}

void Environment::SetAmbientLight(const QVector<float>& vector)
{
    
    OgreRenderer::EC_OgreEnvironment* env = GetEnvironmentComponent();
    if (!env)
        return;

    env->SetAmbientLightColor(Color(vector[0], vector[1], vector[2]));
    
 }

}
