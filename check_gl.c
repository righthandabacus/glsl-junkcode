/* Test script of OpenGL for General Purpose Computing
 *
 * Code adopted from the tutorial:
 *   http://www.mathematik.tu-dortmund.de/~goeddeke/gpgpu/tutorial.html
 *
 * This code simply copy data to and from the GPU to make sure it works
 */

#include <assert.h>
#include "glsl_utils.h"

int main(int argc, char** argv)
{
    int width, height;
    float* data;
    float* result;
    GLuint fbo, tex, win, ok;

    width = height = 5;
    data = (float*)malloc(sizeof(float)*width*height*floatPerTexel);
    result = (float*)malloc(sizeof(float)*width*height*floatPerTexel);
    for (unsigned i=0; i<width*height*floatPerTexel; i++)
        data[i] = i+1.0;

    setGlFormats(GL_TEXTURE_RECTANGLE_ARB, GL_FLOAT_R32_NV, GL_LUMINANCE, 1);
    win = initGlut(&argc, argv);
    ok = setupFBO(width, height, &data, 1, &fbo, &tex);
    assert(ok == fbo);

    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadPixels(0, 0, width, height, texFmt, GL_FLOAT, result);

    // print out results
    printf("Data into GPU:\n");
    for (unsigned i=0; i<width*height*floatPerTexel; i++)
        printf("%f\n",data[i]);
    printf("Data from GPU:\n");
    for (unsigned i=0; i<width*height*floatPerTexel; i++)
        printf("%f\n",result[i]);

    free(data); free(result);
    cleanupFBO(&fbo, &tex, 1);
    glutDestroyWindow(win);
    return 0;
}
