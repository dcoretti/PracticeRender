#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <string>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <Windows.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Font/Font.h"
#include "Render.h"
#include "Util.h"
#include "Types/Math.h"
#include <string>
#include <vector>
#include "Model/SmdLoader.h"
#include "Sound/Sound.h"
#include "Types/Time.h"
#include "Animation/AnimationSystem.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimationInstance.h"

using std::string;
using std::cout;
using std::cin;
using std::endl;




struct RenderContext {
    unsigned int vao;
    unsigned int modelTextureBufferId;
    unsigned int fontTextureBufferId;
    unsigned int shaderProgram;

    // not needed to keep around??
    unsigned int geometryBufferId;
    unsigned int uvBufferId;

    unsigned int fontVao;
    int fontVertices;

    Uniform::ShaderUniformLocations shaderUniforms;

    Mat4 mat;

    RenderObject modelRenderObj;
    Mat4 modelMat;
	AnimationClip *animationClip;
	AnimationInstance animationInstance;
	double elapsedFrameTime{ 0.0 };
};
void renderSetup(RenderContext &renderContext);
void renderDraw(RenderContext &renderContext, uint64_t start);


//float squareUv[] = {
//    0.0f, 0.0f,
//    1.0f, 0.0f,
//    1.0f, 1.0f,
//    0.0f, 0.0f,
//    0.0f, 1.0f,
//    1.0f, 1.0f
//};
//
//
//float square[]{
//    -0.95f, -0.95f, 0.0f,
//    0.95f, -0.95f, 0.0f,
//    0.95f, 0.95f, 0.0f,
//    -0.95f, -0.95f, 0.0f,
//    -0.95f, 0.95f, 0.0f,
//    0.95f, 0.95f, 0.0f
//};

float squareUv[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};


float square[]{
    -0.95f, -0.95f, 0.0f,
    0.95f, -0.95f, 0.0f,
    0.95f, 0.95f, 0.0f,
    -0.95f, -0.95f, 0.0f,
    0.95f, 0.95f, 0.0f,
    -0.95f, 0.95f, 0.0f
};


void setupConsole() {
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
}

// hacky
int curFrame = 0;
int numFrames = 0;
bool animEnabled = false;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (GLFW_PRESS == action) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
		case GLFW_KEY_F:
			curFrame = (curFrame + 1) % numFrames;
			printf("Animation frame: %d\n", curFrame);
			break;
		case GLFW_KEY_P:
			animEnabled = !animEnabled;
			break;
		};

    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    setupConsole();
    //FontTest::testFont();

    GLFWwindow* window;
    glewExperimental = GL_TRUE;
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    // Deprecated functionality will be removed from being marked in a previous version. 
    // in 3.1 it will remove anything marked as deprecated in 3.0 but not removed in 3.1
    // This is ONLY useful between 3.0 and 3.1 for 3.2+ this does nothing as it was a deprecation path
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // instead of setting profile, giving hints and letting system decide
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSwapInterval(1);	//vsync

    window = glfwCreateWindow(1024, 768, "RenderMode", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);


    // some debug string info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    cout << "Renderer " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    //glEnable(GL_DEPTH_TEST); // enable depth-testing
    //glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE);
    glfwSetKeyCallback(window, keyCallback);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        cout << "Error initializing glew: " << glewGetErrorString(err) << endl;
        return -1;
    }

    RenderContext renderContext;

    renderSetup(renderContext);
    HRESULT s = initSound();
	initTimers();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
		uint64_t start = getPerfCounter();
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        renderDraw(renderContext, start);
        glfwSwapBuffers(window);

		renderContext.elapsedFrameTime = getElapsedTimeSeconds(start, getPerfCounter());
    }

    glfwTerminate();
    return 0;
}
Text::Font f{};

void animationSetup(RenderContext &renderContext, SMDModel &model) {
	loadAnimationForExistingModel("anim.smd", &model);
	Skeleton *skeleton = generateSkeleton(model);
	AnimationClip *clip = extractAnimationClipFromSMD(model, skeleton,0);
	numFrames = clip->numFrames;
	renderContext.animationClip = clip;
	renderContext.animationInstance.fps = 20;
	renderContext.animationInstance.localTime = getPerfCounter();
	renderContext.animationInstance.loop = true;
	renderContext.animationInstance.enabled = false;
	renderContext.animationInstance.curClip = 0;
	renderContext.animationInstance.numClips = numFrames;
}

void handleAnimationAdvance(uint64_t now, AnimationInstance &instance) {
	if (instance.enabled && getElapsedTimeSeconds(instance.localTime, now) >= (1.0 / instance.fps)) {
		instance.localTime = now;
		instance.curClip = instance.curClip == instance.numClips - 1 ? 0 : instance.curClip + 1;
	}
}

void renderSetup(RenderContext &renderContext) {
    freopen("err.log", "w", stderr); // for debug output to fiel
    int texW = 512;
    int texH = 512;
    unsigned char * texture = new unsigned char[texW * texH]{ 0 };
    f.heightPixels = 72.0f;// 64.0f; // 96.0f fits really well into 512x512

    Text::loadFontToTexture(f, "C:/Windows/Fonts/Arial.ttf", texture, texW, texH);

    loadTexture(&renderContext.fontTextureBufferId, texW, texH, texture, GL_RED, GL_RED);
    //loadFileToTexture("out.png", &renderContext.textureBufferId);

    int size;
	char * vertexShaderSource = loadTextFile("Shader/Anim.vert", &size);
    //char * vertexShaderSource = loadTextFile("Shader/Texture.vert", &size);
    char * fragmentShaderSource = loadTextFile("Shader/Texture.frag", &size);
    createShader(renderContext.shaderUniforms, &renderContext.shaderProgram, vertexShaderSource, fragmentShaderSource);
    setShader(renderContext.shaderProgram);


    //initAndSetVao(&renderContext.vao);
    //loadBuffer(&renderContext.geometryBufferId, sizeof(square), &square, ShaderAttributeBinding::VERTICES, 3, 0);
    //loadBuffer(&renderContext.uvBufferId, sizeof(squareUv), &squareUv, ShaderAttributeBinding::UV, 2, 0);
	//char buf[64];
	//sprintf(buf, "%d fps", 1000.0 / (double)renderContext.elapsedFrameTime);
 //   renderContext.fontVertices = Text::fontTextToGeometry(f, buf, &renderContext.fontVao);

    renderContext.mat = Mat4::translate(Vec3(-0.95f, 0.80f, 0.0f)) * Mat4::scale(0.001f);

    loadFileToTexture("colormap.png", &renderContext.modelTextureBufferId);
	SMDModel * model = parseSMDFile("box1.smd");//"box1anim.smd");
    renderContext.modelRenderObj = loadSMDToVao(*model);
    renderContext.modelMat = Mat4::scale(0.5f);
 
	animationSetup(renderContext, *model);
}

uint64_t lastFpsPoll = 0;
void setDebugPrint(RenderContext &renderContext, uint64_t frameStart) {
	uint64_t now = getPerfCounter();
	if (lastFpsPoll == 0 || frameStart - lastFpsPoll > (0.2 * counterFrequency)) {
		char buf[64];
		sprintf(buf, "%.1f fps", 1.0 / renderContext.elapsedFrameTime);
		renderContext.fontVertices = Text::fontTextToGeometry(f, buf, &renderContext.fontVao);

		lastFpsPoll = now;
	}
}


void bindDefaultAnimation(RenderContext &renderContext) {
	Mat4 identityMatrix;
	setMatUniform(renderContext.shaderUniforms.skinningPalette, identityMatrix, 1);
	// set joint index to 0?  This should be done in the VAO generation for the non-animated thing. 
	//  Does this mean all geometry needs a buffer of 0 for the joint indexes?
}


float rot = 0.0f;
void renderDraw(RenderContext &renderContext, uint64_t frameStart) {
	// Update animation state
	renderContext.animationInstance.enabled = animEnabled;
	handleAnimationAdvance(frameStart, renderContext.animationInstance);


	// Draw model
    renderContext.modelMat = Mat4::scale(0.5f) * Mat4::roty(rot)*Mat4::rotz(rot);
    //rot += 0.01f;

	AnimationSample *curSample = &renderContext.animationClip->samples[renderContext.animationInstance.curClip];
	setMatUniform(renderContext.shaderUniforms.skinningPalette, *curSample->skinningPalette, renderContext.animationClip->skeleton->numJoints);
    setMatUniform(renderContext.shaderUniforms.mvp, renderContext.modelMat);
    setTexture(renderContext.modelTextureBufferId, 0, 0);
    drawVertexArray(renderContext.modelRenderObj.vao, renderContext.modelRenderObj.numElements);
    

	// Draw HUD
	setDebugPrint(renderContext, frameStart);
    setTexture(renderContext.fontTextureBufferId, 0, 0);
    setMatUniform(renderContext.shaderUniforms.mvp, renderContext.mat);
	bindDefaultAnimation(renderContext);
    drawVertexArray(renderContext.fontVao, renderContext.fontVertices);
}