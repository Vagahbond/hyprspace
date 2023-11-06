#define WLR_USE_UNSTABLE

#include <unistd.h>

#include <any>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/Window.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/render/Shaders.hpp>

#include "comet.hpp"
#include "globals.hpp"
#include "shaders.hpp"

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() { return HYPRLAND_API_VERSION; }

void onNewWindow(void *self, std::any data) {
  // data is guaranteed
  auto *const PWINDOW = std::any_cast<CWindow *>(data);

  HyprlandAPI::addWindowDecoration(PHANDLE, PWINDOW, new CTrail(PWINDOW));
}

GLuint CompileShader(const GLuint &type, std::string src) {
  auto shader = glCreateShader(type);

  auto shaderSource = src.c_str();

  glShaderSource(shader, 1, (const GLchar **)&shaderSource, nullptr);
  glCompileShader(shader);

  GLint ok;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

  if (ok == GL_FALSE)
    throw std::runtime_error("compileShader() failed!");

  return shader;
}

GLuint CreateProgram(const std::string &vert, const std::string &frag) {
  auto vertCompiled = CompileShader(GL_VERTEX_SHADER, vert);

  if (!vertCompiled)
    throw std::runtime_error("Compiling vshader failed.");

  auto fragCompiled = CompileShader(GL_FRAGMENT_SHADER, frag);

  if (!fragCompiled)
    throw std::runtime_error("Compiling fshader failed.");

  auto prog = glCreateProgram();
  glAttachShader(prog, vertCompiled);
  glAttachShader(prog, fragCompiled);
  glLinkProgram(prog);

  glDetachShader(prog, vertCompiled);
  glDetachShader(prog, fragCompiled);
  glDeleteShader(vertCompiled);
  glDeleteShader(fragCompiled);

  GLint ok;
  glGetProgramiv(prog, GL_LINK_STATUS, &ok);

  if (ok == GL_FALSE)
    throw std::runtime_error("createProgram() failed! GL_LINK_STATUS not OK!");

  return prog;
}

int onTick(void *data) {
  EMIT_HOOK_EVENT("trailTick", nullptr);

  const int TIMEOUT =
      g_pHyprRenderer->m_pMostHzMonitor
          ? 1000.0 / g_pHyprRenderer->m_pMostHzMonitor->refreshRate
          : 16;
  wl_event_source_timer_update(g_pGlobalState->tick, TIMEOUT);

  return 0;
}

void initGlobal() {
  RASSERT(eglMakeCurrent(wlr_egl_get_display(g_pCompositor->m_sWLREGL),
                         EGL_NO_SURFACE, EGL_NO_SURFACE,
                         wlr_egl_get_context(g_pCompositor->m_sWLREGL)),
          "Couldn't set current EGL!");

  GLuint prog = CreateProgram(QUADCOMET, FRAGCOMET);
  g_pGlobalState->cometShader.program = prog;
  g_pGlobalState->cometShader.proj = glGetUniformLocation(prog, "proj");
  g_pGlobalState->cometShader.time = glGetUniformLocation(prog, "time");
  g_pGlobalState->cometShader.tex = glGetUniformLocation(prog, "tex");
  g_pGlobalState->cometShader.color = glGetUniformLocation(prog, "color");
  g_pGlobalState->cometShader.output = glGetUniformLocation(prog, "resolution");
  g_pGlobalState->cometShader.texAttrib = glGetAttribLocation(prog, "colors");
  g_pGlobalState->cometShader.posAttrib = glGetAttribLocation(prog, "pos");

  g_pGlobalState->cometShader.gradient = glGetUniformLocation(prog, "window");

  RASSERT(eglMakeCurrent(wlr_egl_get_display(g_pCompositor->m_sWLREGL),
                         EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT),
          "Couldn't unset current EGL!");

  g_pGlobalState->tick =
      wl_event_loop_add_timer(g_pCompositor->m_sWLEventLoop, &onTick, nullptr);
  wl_event_source_timer_update(g_pGlobalState->tick, 1);
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
  PHANDLE = handle;

  const std::string HASH = __hyprland_api_get_hash();

  if (HASH != GIT_COMMIT_HASH) {
    HyprlandAPI::addNotification(
        PHANDLE,
        "[ht] Failure in initialization: Version mismatch (headers ver is not "
        "equal to running hyprland ver)",
        CColor{1.0, 0.2, 0.2, 1.0}, 5000);
    throw std::runtime_error("[ht] Version mismatch");
  }

  HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprtrails:bezier_step",
                              SConfigValue{.floatValue = 0.025});
  HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprtrails:points_per_step",
                              SConfigValue{.intValue = 2});
  HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprtrails:history_points",
                              SConfigValue{.intValue = 20});
  HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprtrails:history_step",
                              SConfigValue{.intValue = 2});
  HyprlandAPI::addConfigValue(
      PHANDLE, "plugin:hyprtrails:color",
      SConfigValue{.intValue = configStringToInt("rgb(cba6f7)")});

  HyprlandAPI::registerCallbackDynamic(
      PHANDLE, "openWindow",
      [&](void *self, SCallbackInfo &info, std::any data) {
        onNewWindow(self, data);
      });

  g_pGlobalState = std::make_unique<SGlobalState>();
  initGlobal();

  // add deco to existing windows
  for (auto &w : g_pCompositor->m_vWindows) {
    if (w->isHidden() || !w->m_bIsMapped)
      continue;

    HyprlandAPI::addWindowDecoration(PHANDLE, w.get(), new CTrail(w.get()));
  }

  HyprlandAPI::reloadConfig();

  HyprlandAPI::addNotification(PHANDLE, "[hyprspace] Initialized successfully!",
                               CColor{0.2, 1.0, 0.2, 1.0}, 5000);

  return {"hyprspace", "A plugin to add comet window decorations and stars",
          "Vaxry & Vagahbond", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
  wl_event_source_remove(g_pGlobalState->tick);
}
