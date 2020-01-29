/*
 * Copyright Regents of the University of Minnesota, 2016.  This software is released under the following license: http://opensource.org/licenses/
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#include <VRVisLinkTextureNode.h>
#include <VisLink/impl/VisLinkAPIImpl.h>
#include <VisLink/image/Texture.h>

using namespace sandbox; 

namespace MinVR {

class VRVisLinkProcLoader : public vislink::ProcLoader {
public:
    VRVisLinkProcLoader(VRWindowToolkit* winToolkit) : winToolkit(winToolkit) {}
    vislink::VLProc getProc(const char* name) {
        return winToolkit->getProcAddress(name);
    }

private:
    VRWindowToolkit* winToolkit;
};

VRVisLinkTextureNode::VRVisLinkTextureNode(const std::string &name, bool stereo, vislink::VisLinkAPI* api, VRWindowToolkit* winToolkit) : VRDisplayNode(name), frame(0), stereo(stereo), api(api), winToolkit(winToolkit)  {
    this->api = new vislink::VisLinkOpenGL(*api, new VRVisLinkProcLoader(winToolkit));

    mainImage.addComponent(new Image("../cmake/framework/external/VisLinkLib/src/app/textures/test.png"));
    mainImage.update();
    
    std::cout << name << " " << getName() << std::endl; 
    api->createSharedTexture(getName() + "/Left/0", vislink::Texture());
    api->createSharedTexture(getName() + "/Right/0", vislink::Texture());
    api->createSharedTexture(getName() + "/Left/1", vislink::Texture());
    api->createSharedTexture(getName() + "/Right/1", vislink::Texture());
}

VRVisLinkTextureNode::~VRVisLinkTextureNode() {
	delete api;
}

	
/// Compiles shader
GLuint VRVisLinkTextureNodecompileShader(const std::string& shaderText, GLuint shaderType) {
    const char* source = shaderText.c_str();
    int length = (int)shaderText.size();
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetShaderInfoLog(shader, length, &length, &log[0]);
        std::cerr << &log[0];
    }

    return shader;
}

/// links shader program
void VRVisLinkTextureNodelinkShaderProgram(GLuint shaderProgram) {
    glLinkProgram(shaderProgram);
    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(shaderProgram, length, &length, &log[0]);
        std::cerr << "Error compiling program: " << &log[0] << std::endl;
    }
}

  
void VRVisLinkTextureNode::render(VRDataIndex *renderState, VRRenderHandler *renderHandler) {
	if ((int)renderState->getValue("InitRender") == 1) {
		initializeGLExtentions();

        glDisable(GL_DEPTH_TEST);
        glClearColor(0, 0, 0, 1);

        // Create VBO
        GLfloat vertices[]  = { 
            -1.0f, -1.0f, 0.0f, 
            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 
            -1.0f, -1.0f, 0.0f};

        // normal array
        GLfloat normals[]   = { 0, 0, 1,   0, 0, 1,   0, 0, 1,    0, 0, 1,   0, 0, 1,  0, 0, 1    };    // v6-v5-v4


        // color array
        GLfloat colors[]    = { 0, 0, 0,   1, 0, 0,   1, 1, 0,   1, 1, 0,   0, 1, 0 ,  0, 0, 0};    // v6-v5-v4

        // Allocate space and send Vertex Data
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals)+sizeof(colors), 0, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals), sizeof(colors), colors);

        // Create vao
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (char*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (char*)0 + sizeof(vertices));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (char*)0 + sizeof(vertices) + sizeof(normals));

        // Create shader
        std::string vertexShader =
                "#version 330 \n"
                "layout(location = 0) in vec3 position; "
                "layout(location = 1) in vec3 normal; "
                "layout(location = 2) in vec3 color; "
                ""
               // "uniform mat4 ProjectionMatrix; "
               // "uniform mat4 ViewMatrix; "
               // "uniform mat4 ModelMatrix; "
               // "uniform mat4 NormalMatrix; "
                ""
                "out vec3 col;"
                ""
                "void main() { "
                "   gl_Position = vec4(position, 1.0); "
                "   col = color;"
                "}";
        vshader = VRVisLinkTextureNodecompileShader(vertexShader, GL_VERTEX_SHADER);

        std::string fragmentShader =
                "#version 330 \n"
                "in vec3 col;"
                "out vec4 colorOut;"
                "uniform sampler2D tex; "
                ""
                "void main() { "
                "   vec2 coord = vec2(col.x, col.y);"
                "   vec4 texColor = texture(tex, coord);"
                "   colorOut = texColor; "
                "}";
        fshader = VRVisLinkTextureNodecompileShader(fragmentShader, GL_FRAGMENT_SHADER); 

        // Create shader program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vshader);
        glAttachShader(shaderProgram, fshader);
        VRVisLinkTextureNodelinkShaderProgram(shaderProgram);

		glActiveTexture(GL_TEXTURE0 + 0);
		GLint loc = glGetUniformLocation(shaderProgram, "tex");
		glUniform1i(loc, 0);

        GLuint format = GL_RGBA;
        GLuint internalFormat = GL_RGBA;
        GLuint type = GL_UNSIGNED_BYTE;

	    textures[0].left = api->getSharedTexture(getName() + "/Left/0").id;
	    textures[0].right = api->getSharedTexture(getName() + "/Right/0").id;
	    textures[1].left = api->getSharedTexture(getName() + "/Left/1").id;
	    textures[1].right = api->getSharedTexture(getName() + "/Right/1").id;

        Image* image = mainImage.getComponent<Image>();  
        glBindTexture(GL_TEXTURE_2D, textures[0].left);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->getWidth(), image->getHeight(), internalFormat, type, image->getData());
        //glBindTexture(GL_TEXTURE_2D, textures[1].left);
        //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->getWidth(), image->getHeight(), internalFormat, type, image->getData());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    if (stereo) {
    	glDrawBuffer(GL_FRONT_LEFT);
    	glBindTexture(GL_TEXTURE_2D, textures[frame % 2].left);
    	renderTexture();
    	glDrawBuffer(GL_FRONT_RIGHT);
    	glBindTexture(GL_TEXTURE_2D, textures[frame % 2].right);
    	renderTexture();
    }
    else {
		//if (frame % 2 == 0) {
			glBindTexture(GL_TEXTURE_2D, textures[frame % 2].left);
		//}
		//else {
		//	glBindTexture(GL_TEXTURE_2D, state->right->getId());
		//}
    	renderTexture();
    }
}

void VRVisLinkTextureNode::renderTexture() {
    // Set shader parameters
    glUseProgram(shaderProgram);

    // Draw quad
    glBindVertexArray(vao);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // reset program
    glUseProgram(0);
}

void VRVisLinkTextureNode::displayFinishedRendering(VRDataIndex *renderState) {
	frame++;
}


VRDisplayNode*
VRVisLinkTextureNode::create(VRMainInterface *vrMain, VRDataIndex *config, const std::string &nameSpace, void* param) {
	std::string nodeNameSpace = nameSpace;
	vislink::VisLinkAPI* api = static_cast<vislink::VisLinkAPI*>(param);

    std::string wtk = config->getValue("WindowToolkit", nameSpace);
    VRWindowToolkit *winToolkit = vrMain->getWindowToolkit(wtk);
    if (winToolkit == NULL) {
      std::cerr << "Cannot get the window toolkit named: " << wtk << std::endl;
    }

	if (config->exists("StereoFormat", nameSpace)) {
		std::string formatStr = config->getValue("StereoFormat", nameSpace);
		return new VRVisLinkTextureNode(nameSpace, formatStr == "QuadBuffered", api, winToolkit);
	}


	return new VRVisLinkTextureNode(nameSpace, false, api, winToolkit);
}


} /* namespace MinVR */


