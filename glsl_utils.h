#ifndef _GLSL_UTILS_H
#define _GLSL_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Loading OpenGL headers, for OS X and Linux */
#ifdef __APPLE__
	#include <GLUT/glut.h>
	#include <OpenGL/gl.h>
#else
	#include <GL/glut.h>
	#include <GL/gl.h>
#endif


#ifdef __APPLE__
    // so the code in glsl_utils.c works for my Mountain Lion as in my Linux box
    #define GL_FLOAT_R32_NV   GL_R32F
    #define GL_LUMINANCE      GL_RED
#endif


// Platform dependent formats
extern GLenum texTarget;
extern GLint intFmt;
extern GLint texFmt;
extern unsigned floatPerTexel;

// Variables for convenience
extern const int ATTACHMENTPOINT[];

// Functions
void setGlFormats(GLenum _texTarget, GLint _intFmt, GLint _texFmt, unsigned _floatPerTexel);
char* contentFromFile(const char * filename);
void printLogToStderr(GLuint object);
GLuint createShader(const char* filename, GLenum type);
GLuint createProgram(char *vsFilename, char *fsFilename);
int frameBufferStatus();
int checkGLStatus();
GLuint initGlut(int* argcp, char** argv);
GLuint setupFBO(GLsizei width, GLsizei height, float**data, const unsigned count, GLuint*fbo, GLuint*tex);
void readFBO(GLenum attachpoint, GLsizei width, GLsizei height, float*data);
int setupTexture(GLsizei width, GLsizei height, GLuint tex);
void cleanupFBO(GLuint* fbo, GLuint* tex, const unsigned count);
void render(GLsizei width, GLsizei height);

#ifdef __cplusplus
}
#endif

#endif /* _GLSL_UTILS_H */
