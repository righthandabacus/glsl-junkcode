/*
 * GLSL for general purpose computing
 * Adrian S-W Tam
 * Mon, 02 Dec 2013 10:50:13 -0500
 */

#include "glsl_utils.h"

// Platform dependent formats
GLenum texTarget = GL_TEXTURE_RECTANGLE_ARB;	// GL_TEXTURE_2D
GLint intFmt = GL_FLOAT_R32_NV; // GL_RGBA32F_ARB;
GLint texFmt = GL_LUMINANCE;    // GL_RGBA;
unsigned floatPerTexel = 1;     // 4;

// Variables for convenience
const int ATTACHMENTPOINT[] = {
	GL_COLOR_ATTACHMENT0_EXT,  GL_COLOR_ATTACHMENT1_EXT,  GL_COLOR_ATTACHMENT2_EXT,  GL_COLOR_ATTACHMENT3_EXT,
	GL_COLOR_ATTACHMENT4_EXT,  GL_COLOR_ATTACHMENT5_EXT,  GL_COLOR_ATTACHMENT6_EXT,  GL_COLOR_ATTACHMENT7_EXT,
	GL_COLOR_ATTACHMENT8_EXT,  GL_COLOR_ATTACHMENT9_EXT,  GL_COLOR_ATTACHMENT10_EXT, GL_COLOR_ATTACHMENT11_EXT,
	GL_COLOR_ATTACHMENT12_EXT, GL_COLOR_ATTACHMENT13_EXT, GL_COLOR_ATTACHMENT14_EXT, GL_COLOR_ATTACHMENT15_EXT
};


/** Reset the format to be used */
void setGlFormats(GLenum _texTarget, GLint _intFmt, GLint _texFmt, unsigned _floatPerTexel)
{
	texTarget = _texTarget;
	intFmt = _intFmt;
	texFmt = _texFmt;
	floatPerTexel = _floatPerTexel;
}

/** Read content from a file
 *  @param filename The file to read
 *  @return pointer to memory created by malloc holding the file content
 */
char* contentFromFile(const char* filename)
{
	FILE* fp;
	char* content = NULL;
	long length;

	if(0  ==( fp = fopen(filename, "rb")                  )) goto EXIT;
	if(-1 ==  fseek(fp, 0, SEEK_END)                       ) goto EXIT;
	if(-1 ==( length = ftell(fp)                          )) goto EXIT;
	if(-1 ==  fseek(fp, 0, SEEK_SET)                       ) goto EXIT;
	if(0  ==( content = (char*) malloc((size_t)length+1)  )) goto EXIT;
	fread(content, 1, (size_t)length, fp);
	if(ferror(fp)) {
		free(content);
		content = NULL;
	} else {
		content[length] = '\0';
	};
EXIT:
	fclose(fp);
	return content;
}


/** Display compilation errors to stderr from shader compiler or program linker
 *  @param object the shader object or program object
 */
void printLogToStderr(GLuint object)
{
	GLint length = 0;
	char* log = NULL;

	if (glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
	} else if (glIsProgram(object))
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
	else {
		fprintf(stderr, "printlog: Not a shader or a program\n");
		return;
	};

	if (length && 0 != (log = (char*)malloc(length))) {
		if (glIsShader(object)) {
			glGetShaderInfoLog(object, length, NULL, log);
		} else if (glIsProgram(object)) {
			glGetProgramInfoLog(object, length, NULL, log);
		};
		fprintf(stderr, "%s", log);
		free(log);
	};
}


/** Compile shader from file with error handling
 *  @param filename the shader source code file
 *  @param type the shader type
 *  @return shader handle
 */
GLuint createShader(const char* filename, GLenum type)
{
	const GLchar* source = contentFromFile(filename);
	if (!source) {
		fprintf(stderr, "Error opening %s: ", filename); perror("");
		return 0;
	};

	GLuint shader = glCreateShader(type);
	/* Generic way for both OpenGL ES 2.0 and OpenGL 2.1 */
	const GLchar* sources[2] = {
#ifdef GL_ES_VERSION_2_0
		"#version 100\n"
		"#define GLES2\n",
#else
		"#version 120\n",
#endif
		source
	};
	glShaderSource(shader, 2, sources, NULL);
	free((void*)source);

	glCompileShader(shader);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		fprintf(stderr, "%s:", filename);
		printLogToStderr(shader);
		glDeleteShader(shader);
		return 0;
	};
	return shader;
}


/** Load and compile vertex shader and fragment shader into a program
 *  @param vsFilename the vertex shader source code file
 *  @param fsFilename the fragment shader source code file
 *  @return program handle
 */
GLuint createProgram(char *vsFilename, char *fsFilename)
{
	/* prepare shaders */
	GLuint vs = 0;
	GLuint fs = 0;
	if (vsFilename) {
		vs = createShader(vsFilename, GL_VERTEX_SHADER_ARB); // same as GL_VERTEX_SHADER
		if (!vs) goto EXIT;
	}
	if (fsFilename) {
		fs = createShader(fsFilename, GL_FRAGMENT_SHADER_ARB); // same as GL_FRAGMENT_SHADER
		if (!fs) goto EXIT;
	};

	/* attach shader to program and link */
	GLuint program = glCreateProgram();
	if (vs) glAttachShader(program, vs);
	if (fs) glAttachShader(program, fs);
	glLinkProgram(program);
	if (vs) glDeleteShader(vs);
	if (fs) glDeleteShader(fs);
	GLint link_ok;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		printLogToStderr(program);
		goto EXIT;
	};
	checkGLStatus();
	return program;
EXIT:
	checkGLStatus();
	if (vs) glDeleteShader(vs);
	if (fs) glDeleteShader(fs);
	if (program) glDeleteProgram(program);
	checkGLStatus();
	return 0;
}


/** Check frame buffer status after FBO initialization
 *  @return 0 if framebuffer complete
 *  @return 1 otherwise with error message print to stderr
 */
int frameBufferStatus()
{
	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
		return 0;
	};
	switch(status) {
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            printf("Unsupported framebuffer format\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, incomplete attachment\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, missing attachment\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            printf("Framebuffer incomplete, attached images must have same dimensions\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            printf("Framebuffer incomplete, attached images must have same format\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            printf("Framebuffer incomplete, missing draw buffer\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            printf("Framebuffer incomplete, missing read buffer\n");
			break;
		default:
			printf("Framebuffer error\n");
	};
	return 1;
}

/** Check OpenGL status
 *  @return 0 if no error
 *  @return 1 otherwise with error message print to stderr
 */
int checkGLStatus()
{
    GLenum err;
    const GLubyte *str;
    
    if ((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr, "%s", gluErrorString(err));
		return 1;
    }
	return 0;
}

/** a macro for initialization
 *  @return handle to the window created
 */
GLuint initGlut(int* argcp, char** argv)
{
	GLuint window;
    glutInit(argcp, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(64, 20);
    glutInitWindowPosition(50,50);
    window = glutCreateWindow("gpgpu computing");
    glEnable(texTarget);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glFlush();
	return window;
}

/** Initialize offscreen framebuffer and set data into it
 *  Offscreen framebuffer offers access to rendering data at full precsion
 *  without clamping.
 *  @param width the texture (i.e. array) width
 *  @param height the texture (i.e. array) height
 *  @param data array of data to fill into the texture buffer(s)
 *  @param count number of texture data in the array, i.e. len(data)
 *  @param fbo pointer to hold the handle to the FBO upon complete
 *  @param tex array to hold the texture id(s) upon complete, array size should equals to count
 */
GLuint setupFBO(GLsizei width, GLsizei height, float**data, const unsigned count, GLuint*fbo, GLuint*tex)
{
	// set up viewport for 1:1 pixel=texel mapping
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, width, height);

	// create FBO and bind
	glGenFramebuffersEXT(1, fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *fbo);

	// create new texture(s)
	glGenTextures(count, tex);
	for (unsigned i=0; i<count; ++i) {
		// bind, turn off filtering, set wrap mode for texture
		glBindTexture(texTarget, tex[i]);
		glTexParameteri(texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // FBO safe
		glTexParameteri(texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		// allocate graphics memory
		glTexImage2D(texTarget, /* texture target, e.g. GL_TEXTURE_2D */
					 0,         /* 0=no mipmap levels for this texture */
					 intFmt,    /* internal format */
					 width,     /* texture widht */
					 height,    /* texture height */
					 0,         /* 0=no borders for texture */
					 texFmt,    /* texture format: number of channels */
					 GL_FLOAT,  /* tell CPU that the data to pass into the texture is float */
					 NULL);     /* NULL pointer: no data to set into texture right now */
		if (checkGLStatus()) goto EXIT;
		// transfer data to texture
		glTexSubImage2D(texTarget, 0, 0, 0, width, height, texFmt, GL_FLOAT, data[i]);
	};
	
	// set texenv to replace instead of modulate
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	if (checkGLStatus()) goto EXIT;
	// attach texture(s) to FBO
	for (unsigned i=0; i<count; ++i) {
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, ATTACHMENTPOINT[i], texTarget, tex[i], 0);
		if (frameBufferStatus()) goto EXIT;
	};
	return *fbo;
EXIT:
	fprintf(stderr, "Failed making textures");
	return 0;
}

/** Clean up the framebuffer
 *  @param fbo pointer to the handle of the framebuffer object
 *  @param tex array of pointer to the handle of the texture object
 *  @param count number of texture objects in array tex
 */
void cleanupFBO(GLuint* fbo, GLuint* tex, const unsigned count)
{
	glDeleteFramebuffersEXT(1, fbo);
	glDeleteTextures(count, tex);
}


/** Make quad and render
 */
void render(GLsizei width, GLsizei height)
{
	glPolygonMode(GL_FRONT,GL_FILL);// make quad filled, should be the default, repeat for safe
	glBegin(GL_QUADS);				// render the quad with unnormalized texture coordinates
        glTexCoord2f(0.0, 0.0); 
        glVertex2f(0.0, 0.0);
        glTexCoord2f(width, 0.0); 
        glVertex2f(width, 0.0);
        glTexCoord2f(width, height); 
        glVertex2f(width, height);
        glTexCoord2f(0.0, height); 
        glVertex2f(0.0, height);
	glEnd();
}

/* vim:set noet sw=4 ts=4 bs=indent,eol,start syntax=c: */
