/*
 * Copyright Regents of the University of Minnesota, 2016.  This software is released under the following license: http://opensource.org/licenses/
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#include <VRVisLinkTextureNode.h>
#include <VisLink/net/Server.h>

using namespace sandbox; 

namespace MinVR {


VRVisLinkTextureNode::VRVisLinkTextureNode(const std::string &name) : VRDisplayNode(name), frame(0)  {
	vislink::VisLinkAPIImpl* impl = new vislink::VisLinkAPIImpl();
    mainImage.addComponent(new Image("../cmake/framework/external/VisLinkLib/src/app/textures/test.png"));
    mainImage.update();
    
}

VRVisLinkTextureNode::~VRVisLinkTextureNode() {
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

        //left = createOpenGLTexture(left);
        //right = createOpenGLTexture(right);
        Image* image = mainImage.getComponent<Image>(); 
        glCreateTextures(GL_TEXTURE_2D, 1, &textures[0].left);
        glBindTexture(GL_TEXTURE_2D, textures[0].left);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image->getWidth(), image->getHeight(), 0, format, type, image->getData());
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

    if (false) {
    	glDrawBuffer(GL_FRONT_LEFT);
    	glBindTexture(GL_TEXTURE_2D, textures[0].left);
    	renderTexture();
    	glDrawBuffer(GL_FRONT_RIGHT);
    	glBindTexture(GL_TEXTURE_2D, textures[0].right);
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
VRVisLinkTextureNode::create(VRMainInterface *vrMain, VRDataIndex *config, const std::string &nameSpace) {
	std::string nodeNameSpace = nameSpace;
	VRDisplayNode *node = new VRVisLinkTextureNode(nameSpace);

	// nothing more to do, no children for a console node.

	return node;
}


} /* namespace MinVR */


