/**
 * android_video.cpp — OpenGL ES 3.0 video output for FBNeo.
 * Renders the NeoGeo 320x224 framebuffer via a full-screen textured quad.
 */

#include "globals.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <cstring>
#include <cmath>

// ============================================================
// GLSL Shaders
// ============================================================
static const char* VERTEX_SHADER = R"(#version 300 es
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
uniform mat4 uMVP;

void main() {
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

static const char* FRAGMENT_SHADER = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D uTexture;

void main() {
    FragColor = texture(uTexture, vTexCoord);
}
)";

// ============================================================
// Video state
// ============================================================
namespace video {

struct State {
    // Native window
    ANativeWindow* window = nullptr;
    int width  = 0;
    int height = 0;

    // EGL
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLSurface eglSurface = EGL_NO_SURFACE;
    EGLContext eglContext = EGL_NO_CONTEXT;
    bool eglInitialized  = false;

    // OpenGL
    GLuint program     = 0;
    GLuint vao         = 0;
    GLuint vbo         = 0;
    GLuint texture     = 0;
    GLint  uTextureLoc = -1;

    // Framebuffer — NeoGeo native 320x224 RGB565
    // FBNeo writes here via pBurnDraw
    uint16_t framebuffer[NEOGEO_WIDTH * NEOGEO_HEIGHT];
    bool     framebufferDirty = false;

    // Filter mode: 0=nearest, 1=linear
    int filterMode = 0;

    // Integer scaling
    bool integerScale = true;

    bool initialized = false;
};

static State g;

// ============================================================
// Shader compilation
// ============================================================
static GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        LOGE("Shader compile error: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static bool createShaderProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, VERTEX_SHADER);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
    if (!vs || !fs) return false;

    g.program = glCreateProgram();
    glAttachShader(g.program, vs);
    glAttachShader(g.program, fs);
    glLinkProgram(g.program);

    GLint linked;
    glGetProgramiv(g.program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(g.program, 512, nullptr, log);
        LOGE("Program link error: %s", log);
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    g.uTextureLoc = glGetUniformLocation(g.program, "uTexture");
    return true;
}

static void setupGeometry() {
    // Full-screen quad (position + texcoord)
    // Positions: -1..1, Texcoords: 0..1
    float vertices[] = {
        // pos          // tex
        -1.0f,  1.0f,  0.0f, 0.0f,  // top-left
        -1.0f, -1.0f,  0.0f, 1.0f,  // bottom-left
         1.0f,  1.0f,  1.0f, 0.0f,  // top-right
         1.0f, -1.0f,  1.0f, 1.0f,  // bottom-right
    };

    glGenVertexArrays(1, &g.vao);
    glGenBuffers(1, &g.vbo);
    glBindVertexArray(g.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // aTexCoord
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

static void setupTexture() {
    glGenTextures(1, &g.texture);
    glBindTexture(GL_TEXTURE_2D, g.texture);

    // Default to nearest (pixel-perfect)
    GLenum filter = g.filterMode == 0 ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Allocate texture storage (RGB565 → RGBA8 for ES 3.0 compatibility)
    // We'll upload as RGB565
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, NEOGEO_WIDTH, NEOGEO_HEIGHT, 0,
                 GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
}

// ============================================================
// Public API
// ============================================================

bool init(ANativeWindow* window) {
    if (g.initialized) return true;
    if (!window) return false;

    g.window = window;

    // Initialize EGL
    g.eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g.eglDisplay == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed");
        return false;
    }

    EGLint major, minor;
    eglInitialize(g.eglDisplay, &major, &minor);

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    eglChooseConfig(g.eglDisplay, configAttribs, &config, 1, &numConfigs);

    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 0,
        EGL_NONE
    };
    g.eglContext = eglCreateContext(g.eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);

    g.eglSurface = eglCreateWindowSurface(g.eglDisplay, config, g.window, nullptr);
    eglMakeCurrent(g.eglDisplay, g.eglSurface, g.eglSurface, g.eglContext);

    // Create shaders + geometry + texture
    if (!createShaderProgram()) return false;
    setupGeometry();
    setupTexture();

    // Clear framebuffer
    memset(g.framebuffer, 0, sizeof(g.framebuffer));

    glViewport(0, 0, ANativeWindow_getWidth(g.window),
                     ANativeWindow_getHeight(g.window));
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    g.eglInitialized = true;
    g.initialized = true;

    LOGI("Video initialized: %dx%d, GL=%s", NEOGEO_WIDTH, NEOGEO_HEIGHT,
         glGetString(GL_VERSION));
    return true;
}

void shutdown() {
    if (!g.initialized) return;

    if (g.vao)    { glDeleteVertexArrays(1, &g.vao);    g.vao = 0; }
    if (g.vbo)    { glDeleteBuffers(1, &g.vbo);         g.vbo = 0; }
    if (g.texture){ glDeleteTextures(1, &g.texture);    g.texture = 0; }
    if (g.program){ glDeleteProgram(g.program);          g.program = 0; }

    if (g.eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(g.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g.eglSurface != EGL_NO_SURFACE) eglDestroySurface(g.eglDisplay, g.eglSurface);
        if (g.eglContext != EGL_NO_CONTEXT) eglDestroyContext(g.eglDisplay, g.eglContext);
        eglTerminate(g.eglDisplay);
        g.eglDisplay = EGL_NO_DISPLAY;
    }

    if (g.window) {
        ANativeWindow_release(g.window);
        g.window = nullptr;
    }

    g.initialized = false;
    g.eglInitialized = false;
    LOGI("Video shutdown");
}

void setWindow(ANativeWindow* window) {
    if (g.window) {
        ANativeWindow_release(g.window);
    }
    g.window = window;

    if (window && g.eglDisplay != EGL_NO_DISPLAY) {
        // Re-create surface
        if (g.eglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(g.eglDisplay, g.eglSurface);
        }
        EGLint attrs[] = { EGL_NONE };
        EGLConfig dummy;
        EGLint n;
        eglChooseConfig(g.eglDisplay, attrs, &dummy, 1, &n);
        g.eglSurface = eglCreateWindowSurface(g.eglDisplay, dummy, window, nullptr);
        eglMakeCurrent(g.eglDisplay, g.eglSurface, g.eglSurface, g.eglContext);

        int w = ANativeWindow_getWidth(window);
        int h = ANativeWindow_getHeight(window);
        glViewport(0, 0, w, h);
        g.width = w;
        g.height = h;
    }
}

void setSize(int w, int h) {
    g.width = w;
    g.height = h;
    if (g.eglInitialized) {
        glViewport(0, 0, w, h);
    }
}

void setFilterMode(int mode) {
    g.filterMode = mode;
    if (g.texture) {
        glBindTexture(GL_TEXTURE_2D, g.texture);
        GLenum filter = mode == 0 ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    }
}

void setIntegerScale(bool enable) {
    g.integerScale = enable;
}

/**
 * Called when FBNeo has rendered a new frame.
 * FBNeo writes RGB565 pixels to pBurnDraw.
 * We copy it to our framebuffer and upload to GPU.
 */
void uploadFramebuffer(const uint16_t* fbData, int srcW, int srcH) {
    if (!g.initialized || !fbData) return;

    // Copy NeoGeo framebuffer
    memcpy(g.framebuffer, fbData, srcW * srcH * sizeof(uint16_t));
    g.framebufferDirty = true;
}

/**
 * Render the frame — call after FBNeo::BurnDrvFrame()
 */
void render() {
    if (!g.initialized || g.eglDisplay == EGL_NO_DISPLAY) return;

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g.program);
    glBindVertexArray(g.vao);
    glActiveTexture(GL_TEXTURE0);

    // Upload framebuffer to texture
    if (g.framebufferDirty) {
        glBindTexture(GL_TEXTURE_2D, g.texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NEOGEO_WIDTH, NEOGEO_HEIGHT,
                        GL_RGB, GL_UNSIGNED_SHORT_5_6_5, g.framebuffer);
        g.framebufferDirty = false;
    } else {
        glBindTexture(GL_TEXTURE_2D, g.texture);
    }

    if (g.uTextureLoc >= 0) {
        glUniform1i(g.uTextureLoc, 0);
    }

    // Draw full-screen quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    eglSwapBuffers(g.eglDisplay, g.eglSurface);
}

bool isInitialized() {
    return g.initialized;
}

uint16_t* getFramebuffer() {
    return g.framebuffer;
}

} // namespace video
