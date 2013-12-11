/* Test script of OpenGL Shader Language for General Purpose Computing
 *
 * Code adopted from the tutorial:
 *   http://www.mathematik.tu-dortmund.de/~goeddeke/gpgpu/tutorial.html
 *
 * This code computes y = alpha*y + x over vectors x and y for a number of loops
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "glsl_utils.h"

int main(int argc, char **argv) {
    /* command line parameters */
    int mode;                       // 0=test, 1=benchmark
    int N, iterations;              // problem size, iterations
    int showResults, compareResults;
    /* application variables */
    float alpha, *dataX, *dataY;    // data
    double start, end;              // for timing
    GLuint fb, prog;                // FBO handle and program handle
    GLuint tex[3];                  // textures handle
    GLint yParam, xParam, aParam;   // connection to params in GLSL
    int writePos=0;                 // ping-pong variable
    int texSize;

    /* parse command line ***********/
    if (argc < 5) {
        printf("Command line parameters:\n");
        printf("Param 1: 0 = check if given format is supported\n");
        printf("         1 = do some benchmarking \n");
        printf("Param 2: 0 = no comparison of results\n");
        printf("         1 = compare and only print out max errors\n");
        printf("         2 = compare and print out full result vectors (use with care for large N)\n");
        printf("Param 3: problem size N       \n");
        printf("Param 4: number of iterations \n");
        exit(0);
    } else {
        mode = atoi(argv[1]);
        int i = atoi(argv[2]);
        if (i >= 0 && i <= 3) {
            showResults = i & 0x02;
            compareResults = i & 0x01;
        } else {
            printf("unknown parameter, exit\n");
            exit(1);
        };
        N = atoi (argv[3]);
        iterations = atoi (argv[4]);
        printf("N=%d, numIter=%d, show=%d, compare=%d\n", N, iterations, showResults, compareResults);
    }

    /* setup parameters *************/
    texSize = (int)sqrt((double)N); // calc dimensions
    N = texSize * texSize;
    // create data vectors and fill with arbitrary values
    dataX = (float*)malloc(N*sizeof(float));
    dataY = (float*)malloc(N*sizeof(float));
    float* data[] = {dataY, dataY, dataX};
    srand(0);
    for (int i=0; i<N; i++) {
        dataX[i] = rand() / (double)(RAND_MAX);
        dataY[i] = rand() / (double)(RAND_MAX);
    }
    alpha = 1.0/9.0;

    /* initialize system ************/
    GLuint hwnd = initGlut(&argc, argv);
    GLuint fbo = setupFBO(texSize, texSize, data, 3, &fb, tex);
    prog = createProgram(NULL, "linear_mapping.f.glsl");
    // Get hook to GLSL variables
    yParam = glGetUniformLocation(prog, "textureY");
    xParam = glGetUniformLocation(prog, "textureX");
    aParam = glGetUniformLocation(prog, "alpha");
    glUseProgram(prog);
    glUniform1f(aParam, alpha);         // use variable alpha as uniform float alpha
    glActiveTexture(GL_TEXTURE1);       // select texture1
    glBindTexture(texTarget, tex[2]);   // apply data X into texture 1
    glUniform1i(xParam, 1);             // use texture 1 as uniform sampler x
    glFinish();                         // flush GPU for more accurate timing

    start = clock();
    /* perform calculation **********/
    for (int i=0; i<iterations; i++) {
        glDrawBuffer(ATTACHMENTPOINT[writePos]);    // set render destination
        glActiveTexture(GL_TEXTURE0);               // select texture 0
        glBindTexture(texTarget, tex[1-writePos]);  // apply data Y into texture 0
        glUniform1i(yParam,0);                      // use texture 0 as uniform sampler y
        render(texSize, texSize);                   // run GLSL program
        writePos = 1-writePos;                      // swap the role of two textures for next iteration
    }
    glFinish();

    end = clock();
    /* calculate FLOPS **************/
    double total = (end-start)/CLOCKS_PER_SEC;
    double mflops = (2.0*N*iterations) / (total * 1e6);
    printf("GPU MFLOP/s:\t\t\t%d\n",(int)mflops);
    // verify data
    if (!frameBufferStatus() && !checkGLStatus()) {
        float* result = (float*)malloc(sizeof(float)*N);    // malloc and copy result from GPU
        readFBO(ATTACHMENTPOINT[1-writePos], texSize, texSize, result);
        if (compareResults)  {
            // verify with CPU
            start=clock();
            for (int i=0; i<N; i++)
                for (int n=0; n<iterations; n++)
                    dataY[i] = dataX[i] + alpha*dataY[i];
            end = clock();
            total = (end-start)/CLOCKS_PER_SEC;
            mflops = (2.0*N*iterations) / (total * 1e6);
            printf("CPU MFLOP/s:\t\t\t%d\n",(int)mflops);
            // and compare results
            double maxError = -1000.0;
            double avgError = 0.0;
            for (int i=0; i<N; i++) {
                double diff = fabs(result[i]-dataY[i]);
                if (diff > maxError)
                    maxError = diff;
                avgError += diff;
            }
            avgError /= (double)N;
            printf("Max Error: \t\t\t%e\n",maxError);
            printf("Avg Error: \t\t\t%e\n",avgError);
            if (showResults) {
                printf("GPU RESULTS\tCPU RESULTS:\n");
                for (int i=0; i<N; i++) {
                    printf("%f\t%f\t%f\n", result[i], dataY[i], result[i]-dataY[i]);
                };
            }
        } else if (showResults) {
            // print out results
            printf("GPU RESULTS:\n");
            for (int i=0; i<N; i++)
                printf("%f\n",result[i]);
        }
        free(result);
    };
    // and clean up
    glDeleteProgram(prog);
    cleanupFBO(&fb, tex, 3);
    glutDestroyWindow (hwnd);
    free(dataX);
    free(dataY);
    // exit
    return 0;
}
