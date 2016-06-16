#pragma once
#include "Types\Math.h"

// standardized shader attribute binding for types of vertex arrays
enum ShaderAttributeBinding {
    VERTICES = 0,
    UV = 1,
    NORMALS = 2,
};

struct RenderObject {
    unsigned int vao;
    unsigned int textureBufferId;
    unsigned int vertices;
    unsigned int normals;
    unsigned int uvs;

    unsigned int numElements;
};

namespace Uniform {
    struct ShaderUniformLocations {
        int mvp{ -1 };
        int mv{ -1 };
        int v{ -1 };
        int m{ -1 };
        int diffuseTexture{ -1 };
        int normalMapTexture{ -1 };

        int lightSourceUniformBlockIndex{ -1 };
        int materialUniformBlockIndex{ -1 };
    };

    static const char * mvp = "mvp";
    static const char * mv = "mv";
    static const char * v = "v";
    static const char * m = "m";

    // Material components
    static const char * materialBlockIndex = "Material";
    static const char * lightSourceBlockIndex = "LightSource";

    // Default bindings between shader programs for uniform blocks.  see populateShaderUniformLocations
    const int defaultMaterialUniformBlockBinding = 0;
    const int defaultLightSourceUniformBlockBinding = 1;

    // Texture uniforms
    static const char * diffuseTexture = "diffuseTexture";
    static const char * normalMapTexture = "normalMap";
}


struct RenderGroup {
    unsigned int vao;
    unsigned int uvBuf;
    unsigned int geometryBuf;

    void * geometryData;
    void * uvData;

    unsigned int textureId;
    // texture slot??
};



void initAndSetVao(unsigned int *vaoBufferId);
void loadBuffer(unsigned int *bufferId, int sizeBytes, void  *data, ShaderAttributeBinding shaderBinding, int perVertexComponents, int stride);
void loadTexture(unsigned int *bufferId, int width, int height, void * data, int internalFormat, int pixelFormat);
void loadFileToTexture(const char * fname, unsigned int * bufferId);
void createShader(Uniform::ShaderUniformLocations &shaderUniforms, unsigned int *shaderProgramId, const char * vertexSource, const char * fragmentSource);
void drawVertexArray(unsigned int vaoBufferId, int numElements, int indexOfset = 0);
void setTexture(unsigned int textureBufferId, int textureBufferSlot, int textureBufferUniformLocation);
void setShader(unsigned int shaderProgramId);
void setMatUniform(unsigned int uniformLocation, const Mat4 &mat);