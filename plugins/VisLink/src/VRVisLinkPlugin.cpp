/*
 * Copyright Regents of the University of Minnesota, 2015.  This software is released under the following license: http://opensource.org/licenses/GPL-2.0.
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#include <iostream>
#include <plugin/VRPlugin.h>
#include "VRVisLinkTextureNode.h"
#include "VRVisLinkRenderTextureNode.h"

// special: include this only once in one .cpp file per plugin
#include <plugin/VRPluginVersion.h>
#include <VisLink/impl/VisLinkAPIImpl.h>

namespace MinVR {

class VRVisLinkPlugin : public VRPlugin {
public:
	PLUGIN_API VRVisLinkPlugin() {
      //std::cout << "OpenGLPlugin created." << std::endl;
	}
	PLUGIN_API virtual ~VRVisLinkPlugin() {
      //std::cout << "OpenGLPlugin destroyed." << std::endl;
	}

	PLUGIN_API void registerWithMinVR(VRMainInterface *vrMain)
	{
      std::cout << "Registering VRVisLinkPlugin." << std::endl;
      api = new vislink::VisLinkAPIImpl();
      vrMain->setObject("VisLinkAPI", api);
	  vrMain->getFactory()->registerItemTypeWithParam<VRDisplayNode, VRVisLinkTextureNode, void*>("VRVisLinkTextureNode", api);
	  vrMain->getFactory()->registerItemTypeWithParam<VRDisplayNode, VRVisLinkRenderTextureNode, void*>("VRVisLinkRenderTextureNode", api);
	}

	PLUGIN_API void unregisterWithMinVR(VRMainInterface *vrMain)
	{
      vrMain->removeObject("VisLinkAPI");
      //std::cout << "Unregistering GlfwPlugin." << std::endl;
		// TODO
	}

private:
	vislink::VisLinkAPI* api;
};

} // end namespace

extern "C"
{
	PLUGIN_API MinVR::VRPlugin* createPlugin() {
		return new MinVR::VRVisLinkPlugin();
	}
}




#include "IUnityInterface.h"
#include "IUnityGraphics.h"

struct UnityTextureInfo {
	UnityTextureInfo() : texture(NULL) {}

	vislink::VisLinkAPI* api;
	std::string name;
	vislink::OpenGLTexture* texture;
};

class UnityTextureManager {

};

static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;
static UnityGfxRenderer s_RendererType = kUnityGfxRendererNull;

static void UNITY_INTERFACE_API
OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	switch (eventType)
	{
	case kUnityGfxDeviceEventInitialize:
	{
		s_RendererType = s_Graphics->GetRenderer();

		if (s_RendererType == kUnityGfxRendererOpenGLCore) {
			initializeGLExtentions();

			/*vislink::Client* client = new vislink::Client();
			vislink::VisLinkAPI* api = client;
			api = new vislink::VisLinkOpenGL(api);
			tex = api->getSharedTexture("test.png");*/
		}

		break;
	}
	case kUnityGfxDeviceEventShutdown:
	{
		s_RendererType = kUnityGfxRendererNull;
		break;
	}
	case kUnityGfxDeviceEventBeforeReset:
	{
		break;
	}
	case kUnityGfxDeviceEventAfterReset:
	{
		break;
	}
	};
}

// Unity plugin load event
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	s_Graphics = unityInterfaces->Get<IUnityGraphics>();

	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

	// Run OnGraphicsDeviceEvent(initialize) manually on plugin load
	// to not miss the event in case the graphics device is already initialized
	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

// Unity plugin unload event
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

// Plugin function to handle a specific rendering event
static void UNITY_INTERFACE_API OnCreateTextures(int eventID)
{
	/*if (currentDisplayManager) {
		s_RendererType = s_Graphics->GetRenderer();
		//if (s_RendererType == kUnityGfxRendererOpenGLCore) {
		currentDisplayManager->createContextTextures();
		//}
	}*/
}

// Freely defined function to pass a callback to plugin-specific scripts
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetCreateTexturesFunc()
{
	return OnCreateTextures;
}
