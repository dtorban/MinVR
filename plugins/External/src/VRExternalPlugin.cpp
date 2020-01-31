/*
 * Copyright Regents of the University of Minnesota, 2015.  This software is released under the following license: http://opensource.org/licenses/GPL-2.0.
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#include <iostream>

#include <main/VRMain.h>
using namespace MinVR;

extern "C" {
	PLUGIN_API typedef void(*MINVR_CALLBACK)(void* app, const void* state);
}

class ExternalApp : public VREventHandler, VRRenderHandler {
public:
	ExternalApp(int argc, char** argv, MINVR_CALLBACK eventCallback, MINVR_CALLBACK contextCallback, MINVR_CALLBACK sceneCallback)
	  : eventCallback(eventCallback), contextCallback(contextCallback), sceneCallback(sceneCallback) {
		vrMain = new VRMain();
		vrMain->addEventHandler(this);
		vrMain->addRenderHandler(this);
		vrMain->initialize(argc, argv);
	}

	~ExternalApp() {
		vrMain->shutdown();
		//frame();
		delete vrMain;
	}

	/// onVREvent is called when a new intput event happens.
	void onVREvent(const VRDataIndex &event) {
		eventCallback(this, &event);
	}

	/// onVRRenderContext is the override which allows users to setup context specific
	/// variables like VBO's, VAO's, textures, framebuffers, and shader programs.
	void onVRRenderContext(const VRDataIndex &stateData) {
		contextCallback(this, &stateData);
	}

	/// onVRRenderScene will run draw calls on each viewport inside a context.
	void onVRRenderScene(const VRDataIndex &stateData) {
		sceneCallback(this, &stateData);
	}

	void frame() {
		vrMain->synchronizeAndProcessEvents();
		vrMain->updateAllModels();
		vrMain->renderOnAllDisplays();
	}

	void startFrame() {
		vrMain->synchronizeAndProcessEvents();
		vrMain->updateAllModels();
		vrMain->startRenderOnAllDisplays();
	}

	void endFrame() {
		vrMain->syncronizeAndDisplayOnAllDisplays();
	}

	void* getObject(const std::string& name) {
		return vrMain->getObject(name);
	}

	private:
		VRMain *vrMain;
		MINVR_CALLBACK eventCallback, contextCallback, sceneCallback;
};


extern "C" {
	PLUGIN_API void* createApp(char* config, MINVR_CALLBACK eventCallback, MINVR_CALLBACK contextCallback, MINVR_CALLBACK sceneCallback) {
		char* argv[3];
		int argc = 3;
		argv[0] = "";
		argv[1] = "-c";
		argv[2] = config;
		ExternalApp* app = new ExternalApp(argc, argv, eventCallback, contextCallback, sceneCallback);
		return app;
	}

	PLUGIN_API void destroyApp(void* app) {
		ExternalApp* externalApp = static_cast<ExternalApp*>(app);
		delete externalApp;
	}
	
	PLUGIN_API void frame(void* app) {
		ExternalApp* externalApp = static_cast<ExternalApp*>(app);
		externalApp->frame();
	}

	PLUGIN_API void startFrame(void* app) {
		ExternalApp* externalApp = static_cast<ExternalApp*>(app);
		externalApp->startFrame();
	}

	PLUGIN_API void endFrame(void* app) {
		ExternalApp* externalApp = static_cast<ExternalApp*>(app);
		externalApp->endFrame();
	}

	PLUGIN_API void* getObject(void* app, char* name) {
		ExternalApp* externalApp = static_cast<ExternalApp*>(app);
		externalApp->getObject(name);
	}

	PLUGIN_API void dataIndexToString(char* str, const void* dataIndex) {
		const VRDataIndex& data = *static_cast<const VRDataIndex*>(dataIndex);
		strcpy(str, data.serialize().c_str());
	}

	PLUGIN_API int dataIndexGetName(const void* dataIndex, char* str) {
		const VRDataIndex& data = *static_cast<const VRDataIndex*>(dataIndex);
		std::string name = data.getName();
		strcpy(str, name.c_str());
		return name.size();
	}

	PLUGIN_API int dataIndexGetString(const void* dataIndex, char* str, char* key) {
		const VRDataIndex& data = *static_cast<const VRDataIndex*>(dataIndex);
		std::string value = data.getValue(key);
		strcpy(str, value.c_str());
		return value.size();
	}

	PLUGIN_API int dataIndexGetIntValue(const void* dataIndex, char* key) {
		const VRDataIndex& index = *static_cast<const VRDataIndex*>(dataIndex);
		return (int)index.getValue(key);
	}

	PLUGIN_API void* dataIndexGetObject(const void* dataIndex, char* key, void* app) {
		const VRDataIndex& index = *static_cast<const VRDataIndex*>(dataIndex);
		ExternalApp* externalApp = static_cast<ExternalApp*>(app);
		std::string objKey = index.getValue(key);
		return externalApp->getObject(objKey);
	}

	PLUGIN_API float dataIndexGetFloatValue(const void* dataIndex, char* key) {
		const VRDataIndex& index = *static_cast<const VRDataIndex*>(dataIndex);
		return (float)index.getValue(key);
	}

	PLUGIN_API void dataIndexGetFloatArray(const void* dataIndex, char* key, float* arr, int size) {
		const VRDataIndex& data = *static_cast<const VRDataIndex*>(dataIndex);
		std::string name = data.getName();
		VRFloatArray projMat = data.getValue(key);// "ProjectionMatrix");
		memcpy(arr, &projMat[0], size*sizeof(float));
	}

	PLUGIN_API void minvrCallback(void* app, const void* state) {
		std::cout << app << " " << state << std::endl;
	}

	PLUGIN_API int testDLL() {
		createApp("ivlabcave-unity.minvr", minvrCallback, minvrCallback, minvrCallback);
		return 78;
	}

}

int main(int argc, char **argv) {
	char* argv2[3];
	int argc2 = 3;
	argv2[0] = "";
	argv2[1] = "-c";
	argv2[2] = "ivlabcave-unity.minvr";
	ExternalApp* app = new ExternalApp(argc2, argv2, minvrCallback, minvrCallback, minvrCallback);
	while (true) {
		frame(app);
	}
	return 0;
}