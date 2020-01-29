/*
 * Copyright Regents of the University of Minnesota, 2016.  This software is released under the following license: http://opensource.org/licenses/
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#include <VRVisLinkRenderTextureNode.h>

using namespace sandbox; 

namespace MinVR {


VRVisLinkRenderTextureNode::VRVisLinkRenderTextureNode(const std::string &name, const std::string& textureNode, vislink::VisLinkAPI* api) : VRDisplayNode(name), textureNode(textureNode), frame(0), api(api)  {
}

VRVisLinkRenderTextureNode::~VRVisLinkRenderTextureNode() {}

  
void VRVisLinkRenderTextureNode::render(VRDataIndex *renderState, VRRenderHandler *renderHandler) {
    renderState->pushState();
    
    renderState->addData("TextureFrame", (frame+2-1) % 2);

    if ((int)renderState->getValue("InitRender") == 1) {
        std::string eyeType = renderState->getValue("Eye");
        renderState->addData("VisLinkAPI", "VisLinkAPI");

        std::string textureName = textureNode + "/" + (eyeType == "Right" ? "Right" : "Left") + "/";
        renderState->addData("SharedTexturePrefix", textureName);
        std::cout << renderState->serialize() << std::endl;
    }


    renderHandler->onVRRenderScene(*renderState);
    renderState->popState();
}

void VRVisLinkRenderTextureNode::displayFinishedRendering(VRDataIndex *renderState) {
	frame++;
}


VRDisplayNode*
VRVisLinkRenderTextureNode::create(VRMainInterface *vrMain, VRDataIndex *config, const std::string &nameSpace, void* param) {
	std::string nodeNameSpace = nameSpace;
	vislink::VisLinkAPI* api = static_cast<vislink::VisLinkAPI*>(param);

    std::string textureNode;
    if (config->exists("WindowTextures", nameSpace)) {
        textureNode = (std::string)config->getValue("WindowTextures", nameSpace);
    }

	return new VRVisLinkRenderTextureNode(nameSpace, textureNode, api);
}


} /* namespace MinVR */


