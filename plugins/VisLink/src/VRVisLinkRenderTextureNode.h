/*
 * Copyright Regents of the University of Minnesota, 2016.  This software is released under the following license: http://opensource.org/licenses/
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#ifndef VRVISLINKRENDERTEXTURENODE_H_
#define VRVISLINKRENDERTEXTURENODE_H_

#include <iostream>

#include "display/VRDisplayNode.h"
#include <main/VRFactory.h>
#include <main/VRError.h>
#include <OpenGL.h>
#include "sandbox/image/Image.h"
#include <VisLink/VisLinkAPI.h>
#include <display/VRWindowToolkit.h>

namespace MinVR {

class VRVisLinkRenderTextureNode : public VRDisplayNode {
public:
	VRVisLinkRenderTextureNode(const std::string &name, const std::string &textureNode, vislink::VisLinkAPI* api);
	virtual ~VRVisLinkRenderTextureNode();

	virtual std::string getType() const { return "VRVisLinkRenderTextureNode"; }

	void render(VRDataIndex *renderState, VRRenderHandler* renderHandler);
	void displayFinishedRendering(VRDataIndex *renderState);

	static VRDisplayNode* create(VRMainInterface *vrMain, VRDataIndex *config, const std::string &nameSpace, void* plugin);

private:
	int frame;
	std::string textureNode;
	vislink::VisLinkAPI* api;
};

} /* namespace MinVR */

#endif /* VRCONSOLENODE_H_ */
