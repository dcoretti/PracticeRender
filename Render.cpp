#include "Render.h"
#include <GL/glew.h>
#include "Common.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


void initAndSetVao(unsigned int *vao) {
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);
}

void loadBuffer(unsigned int *bufferId, int sizeBytes, void  *data, ShaderAttributeBinding shaderBinding, int perVertexComponents, int stride) {
    glGenBuffers(1, bufferId);
    glBindBuffer(GL_ARRAY_BUFFER, *bufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, GL_STATIC_DRAW);

    // Bind this array buffer to the shader layout and define the data format.
    glVertexAttribPointer(shaderBinding,
        perVertexComponents,
        GL_FLOAT,
        GL_FALSE, // not normalized
        stride, (void*)0);
    glEnableVertexAttribArray(shaderBinding);
}

void loadTexture(unsigned int *bufferId, int width, int height, void * data, int internalFormat, int pixelFormat)
{
    glGenTextures(1, bufferId);
    glBindTexture(GL_TEXTURE_2D, *bufferId);
    // TODO use internal texture format and convert to gl format or have some sort of static mapping
    glTexImage2D(GL_TEXTURE_2D,
        0,
        internalFormat, //GL_RGB
        width,
        height,
        0,
        pixelFormat,    // GL_RGB
        GL_UNSIGNED_BYTE,
        data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void drawVertexArray(unsigned int vaoBufferId, int numElements, int indexOffset) {
    glBindVertexArray(vaoBufferId);
    glDrawArrays(GL_TRIANGLES, indexOffset, numElements);
    glBindVertexArray(0);
}


void setTexture(unsigned int textureBufferId, int textureBufferSlot, int textureBufferUniformLocation) {
    glActiveTexture(GL_TEXTURE0 + textureBufferSlot);
    glBindTexture(GL_TEXTURE_2D, textureBufferId);
    //glUniform1i(textureBufferUniformLocation, textureBufferSlot);
}

void checkShaderCompileStatus(unsigned int shaderId) {
    int status;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

        char *strInfoLog = new char[infoLogLength + 1];
        glGetShaderInfoLog(shaderId, infoLogLength, NULL, strInfoLog);
        DBG_ASSERT(false, "Compile failure for shader %d : %s", shaderId, strInfoLog);
        delete[] strInfoLog;
    }
}

void checkShaderLinkStatus(unsigned int shaderProgram) {
    int status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(shaderProgram, infoLogLength, NULL, strInfoLog);
        DBG_ASSERT(false, "Linker failure %s", strInfoLog);
        delete[] strInfoLog;
    }
}

//// TODO Make this a separate command that can include arbitrary uniforms and map to a specified value location
void populateShaderUniformLocations(Uniform::ShaderUniformLocations &uniformLocations, unsigned int *shaderProgramId) {
    // single uniforms
    uniformLocations.mvp = glGetUniformLocation(*shaderProgramId, Uniform::mvp);
    //uniformLocations.mv = glGetUniformLocation(*shaderProgramId, Uniform::mv);
    //parseError("mv");
    //uniformLocations.v = glGetUniformLocation(*shaderProgramId, Uniform::v);
    //parseError("v");
    //uniformLocations.m = glGetUniformLocation(*shaderProgramId, Uniform::m);
    //parseError("m");

    //shaderProgram->uniformLocations.diffuseTexture = glGetUniformLocation(shaderProgram->shaderProgramId, GPU::Uniform::diffuseTexture);
    //shaderProgram->uniformLocations.normalMapTexture = glGetUniformLocation(shaderProgram->shaderProgramId, GPU::Uniform::normalMapTexture);

    // shader uniform blocks
    //
    //shaderProgram->uniformLocations.materialUniformBlockIndex = glGetUniformBlockIndex(shaderProgram->shaderProgramId,
    //    GPU::Uniform::materialBlockIndex);
    //glUniformBlockBinding(shaderProgram->shaderProgramId,
    //    shaderProgram->uniformLocations.materialUniformBlockIndex,
    //    GPU::Uniform::defaultMaterialUniformBlockBinding);

    //shaderProgram->uniformLocations.lightSourceUniformBlockIndex = glGetUniformBlockIndex(shaderProgram->shaderProgramId,
    //    GPU::Uniform::lightSourceBlockIndex);
    //glUniformBlockBinding(shaderProgram->shaderProgramId,
    //    shaderProgram->uniformLocations.lightSourceUniformBlockIndex,
    //    GPU::Uniform::defaultLightSourceUniformBlockBinding);
}


void createShader(Uniform::ShaderUniformLocations &shaderUniforms, unsigned int *shaderProgramId, const char * vertexSource, const char * fragmentSource) {

    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, 0);   // null for len assumes \0 terminated.
    glCompileShader(vertexShader);
    checkShaderCompileStatus(vertexShader);

    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, 0);   // null for len assumes \0 terminated.
    glCompileShader(fragmentShader);
    checkShaderCompileStatus(fragmentShader);

    *shaderProgramId = glCreateProgram();
    glAttachShader(*shaderProgramId, vertexShader);
    glAttachShader(*shaderProgramId, fragmentShader);
    glLinkProgram(*shaderProgramId);
    checkShaderLinkStatus(*shaderProgramId);

    glDetachShader(*shaderProgramId, vertexShader);
    glDetachShader(*shaderProgramId, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    populateShaderUniformLocations(shaderUniforms, shaderProgramId);
}


void setShader(unsigned int shaderProgramId) {
    glUseProgram(shaderProgramId);
}



void loadFileToTexture(const char * fname, unsigned int * bufferId) {
    int x, y, components;
    // Flip to opengl expected format with first pixel at bottom left rather than top left.
    stbi_set_flip_vertically_on_load(1);
    unsigned char * data = stbi_load(fname, &x, &y, &components, 0);
    DBG_ASSERT(data != nullptr, "Problem loading file %s", fname);
    int imageFormat = 3;
    if (components == 1) {
        imageFormat = GL_RED;
    } else if (components == 3) {
        imageFormat = GL_RGB;
    } else if (components == 4) {
        imageFormat = GL_RGBA;
    }
    loadTexture(bufferId, x, y, data, imageFormat, imageFormat);

    stbi_image_free(data);
}


void setMatUniform(unsigned int uniformLocation, const Mat4 &mat) {
    glUniformMatrix4fv(uniformLocation, 1, false, &mat.m[0][0]);
}