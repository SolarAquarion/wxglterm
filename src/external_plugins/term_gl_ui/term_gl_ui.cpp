#include <pybind11/embed.h>

#include "plugin.h"
#include "plugin_manager.h"
#include "term_gl_ui.h"
#include "term_window.h"
#include "task.h"
#include "term_context.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "app_config_impl.h"

#include <iostream>

#include "freetype-gl.h"
#include "vertex-buffer.h"
#include "text-buffer.h"

#include <GLFW/glfw3.h>


class PluginBase : public virtual Plugin {
public:
    PluginBase(const char * name, const char * description, uint32_t version);
    virtual ~PluginBase() = default;

public:
    const char * GetName() override {
        return m_Name.c_str();
    }
    const char * GetDescription() override {
        return m_Description.c_str();
    }

    uint32_t GetVersion() override {
        return m_Version;
    }

    virtual void InitPlugin(ContextPtr context,
                            AppConfigPtr plugin_config) override {
        m_Context = context;
        m_PluginConfig = plugin_config;
    }
    ContextPtr GetPluginContext() const override {
        return m_Context;
    }
    AppConfigPtr GetPluginConfig() const override {
        return m_PluginConfig;
    }
private:
    std::string m_Name;
    std::string m_Description;
    uint32_t m_Version;

protected:
    ContextPtr m_Context;
    AppConfigPtr m_PluginConfig;
};

PluginBase::PluginBase(const char * name, const char * description, uint32_t version) :
    m_Name(name)
    , m_Description(description)
    , m_Version(version)
    , m_Context{}
    , m_PluginConfig{}
{
}

// ---------------------------------------------------------------- reshape ---
void reshape( GLFWwindow* window, int width, int height )
{
    (void)window;
    glViewport(0, 0, width, height);
}


// --------------------------------------------------------------- keyboard ---
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    (void)window;
    (void)key;
    (void)scancode;
    (void)action;
    (void)mods;

    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose( window, GL_TRUE );
    }
}

// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    (void)window;
}

// --------------------------------------------------------- error-callback ---
void error_callback( int error, const char* description )
{
    (void)error;
    (void)description;
    fputs( description, stderr );
}

class DefaultTermWindow :
        public virtual PluginBase
        , public virtual TermWindow {
public:
    DefaultTermWindow() :
        PluginBase("term_gl_window", "opengl terminal window plugin", 1)
        , m_MainDlg(nullptr) {
    }

    virtual ~DefaultTermWindow() {
        if (m_MainDlg)
            glfwDestroyWindow(m_MainDlg);
    }

public:
    void Refresh() override {
    }

    void Show() override {
        if (!m_MainDlg)
            m_MainDlg = glfwCreateWindow( 800, 640, "wxglTerm", NULL, NULL );

        glfwShowWindow(m_MainDlg);
        glfwMakeContextCurrent(m_MainDlg);

        glfwSwapInterval( 1 );

        glfwSetFramebufferSizeCallback(m_MainDlg, reshape );
        glfwSetWindowRefreshCallback(m_MainDlg, display );
        glfwSetKeyCallback(m_MainDlg, keyboard );
    }

    void Close() override {
        if (!m_MainDlg) return;

        glfwSetWindowShouldClose( m_MainDlg, GL_TRUE );
    }

    void SetWindowTitle(const std::string & title) override {
        (void)title;
    }

    uint32_t GetColorByIndex(uint32_t index) override {
        (void)index;
        return 0;
    }

    std::string GetSelectionData() override {
        std::string sel_data {};

        return sel_data;
    }

    void SetSelectionData(const std::string & sel_data) override {
        (void)sel_data;
    }

private:
    GLFWwindow * m_MainDlg;
};

class DefaultTermUI : public virtual PluginBase, public virtual TermUI {
public:
    DefaultTermUI() :
        PluginBase("term_gl_ui", "opengl terminal ui plugin", 1)
    {
    }

    virtual ~DefaultTermUI() = default;

    TermWindowPtr CreateWindow() {
        auto window = TermWindowPtr { new DefaultTermWindow() };
        window->InitPlugin(GetPluginContext(),
                           GetPluginConfig());

        return window;
    }

    int32_t StartMainUILoop() {
        glfwSetErrorCallback( error_callback );

        glfwInit();

        glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
        glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

        pybind11::gil_scoped_release release;

        while (true)
        {
            glfwPollEvents( );
        }

        return 0;
    }

    bool ScheduleTask(TaskPtr task, int miliseconds, bool repeated) {
        (void)task;
        (void)miliseconds;
        (void)repeated;

        return false;
    }
};

TermUIPtr CreateOpenGLTermUI() {
    return TermUIPtr{ new DefaultTermUI()};
}
