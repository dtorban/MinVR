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

