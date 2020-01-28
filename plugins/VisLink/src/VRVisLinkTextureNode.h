/*
 * Copyright Regents of the University of Minnesota, 2016.  This software is released under the following license: http://opensource.org/licenses/
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#ifndef VRVISLINKTEXTURENODE_H_
#define VRVISLINKTEXTURENODE_H_

#include <iostream>

#include "display/VRDisplayNode.h"
#include <main/VRFactory.h>
#include <main/VRError.h>
#include <OpenGL.h>
#include "sandbox/image/Image.h"

namespace MinVR {

class VRVisLinkTextureNode : public VRDisplayNode {
public:
	VRVisLinkTextureNode(const std::string &name);
	virtual ~VRVisLinkTextureNode();

	virtual std::string getType() const { return "VRVisLinkTextureNode"; }

	void render(VRDataIndex *renderState, VRRenderHandler* renderHandler);
	void displayFinishedRendering(VRDataIndex *renderState);

	static VRDisplayNode* create(VRMainInterface *vrMain, VRDataIndex *config, const std::string &nameSpace);

private:
	void renderTexture();

	struct TextureSet {
		GLuint left, right;
	} textures[2];

	sandbox::EntityNode mainImage;

	GLuint vbo, vao, vshader, fshader, shaderProgram;

	int frame;
};

} /* namespace MinVR */

#endif /* VRCONSOLENODE_H_ */
