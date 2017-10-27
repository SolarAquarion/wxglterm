#include <pybind11/embed.h>

#include <iostream>
#include <vector>
#include <list>

#include <unistd.h>
#include <string.h>

#include <sys/select.h>
#include <sys/wait.h>

#include "plugin_base.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"

#include "PortableThread.h"

#include "term_data_handler_impl.h"
#include "read_write_queue.h"
#include "load_module.h"

using TermDataQueue = moodycamel::BlockingReaderWriterQueue<char, 4096>;

namespace py = pybind11;

#pragma GCC visibility push(hidden)
class TermDataHandlerImpl
        : public virtual PluginBase
        , public virtual TermDataHandler
        , public virtual PortableThread::IPortableRunnable
{
public:
    TermDataHandlerImpl() :
        PluginBase("term_data_handler", "terminal data handler", 1)
        , m_DataHandlerThread(this)
        , m_TermDataQueue {4096}
        , m_Stopped{true}
    {
    }

    virtual ~TermDataHandlerImpl() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new TermDataHandlerImpl};
    }

    void InitPlugin(ContextPtr context,
                    AppConfigPtr plugin_config) override {
        PluginBase::InitPlugin(context, plugin_config);
        LoadPyDataHandler();
    }

    unsigned long Run(void * /*pArgument*/) override;
    void OnData(const char * data, size_t data_len) override;
    void Start() override;
    void Stop() override;

private:
    void LoadPyDataHandler();
    void ProcessSingleChar(const char * ch);
    void ProcessAllChars(char ch);

    PortableThread::CPortableThread m_DataHandlerThread;
    TermDataQueue m_TermDataQueue;
    bool m_Stopped;
    py::object m_DataHandler;
};

TermDataHandlerPtr CreateTermDataHandler()
{
    return TermDataHandlerPtr { new TermDataHandlerImpl };
}

void TermDataHandlerImpl::ProcessSingleChar(const char * ch) {
    if (ch)
        m_DataHandler.attr("on_term_data")(*ch);
    else
        m_DataHandler.attr("on_term_data")();

    py::object cap_name = m_DataHandler.attr("cap_name");
    py::object params = m_DataHandler.attr("params");
    py::object output_char = m_DataHandler.attr("output_char");

    if (!cap_name.is_none())
        std::cout << "cap name:" << cap_name.cast<std::string>() << std::endl;
    if (!params.is_none()) {
        py::list l = params;
        for(auto i : l)
            std::cout << "param:" << i.cast<int>() << std::endl;
    }
    if (!output_char.is_none())
        std::cout << "output char:" << output_char.cast<std::string>() << std::endl;
}

void TermDataHandlerImpl::ProcessAllChars(char ch) {
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    do {
        ProcessSingleChar(&ch);
    } while (!m_Stopped && m_TermDataQueue.try_dequeue(ch));

    if (!m_Stopped)
        ProcessSingleChar(NULL);

    PyGILState_Release(gstate);
}

unsigned long TermDataHandlerImpl::Run(void * /*pArgument*/) {

    while(!m_Stopped) {
        char c = '\0';

        if (!m_TermDataQueue.wait_dequeue_timed(c, std::chrono::microseconds(1000000))) {
            std::cout << "queue wait timed out ..." << std::endl;

            if (m_Stopped)
                break;

            continue;
        }

        if (m_Stopped)
            break;

        //process char
        ProcessAllChars(c);
    };

    return 0;
}

void TermDataHandlerImpl::OnData(const char * data, size_t data_len) {
    for (size_t i = 0; i < data_len; i++) {
        m_TermDataQueue.enqueue(data[i]);
    }
}

void TermDataHandlerImpl::LoadPyDataHandler() {
    try
    {
        const char *module_content =
#include "term_data_handler_impl.inc"
                ;

        std::string termcap_dir = GetPluginContext()->GetAppConfig()->GetEntry("/term/termcap_dir", "data");
        std::string term_name = GetPluginContext()->GetAppConfig()->GetEntry("/term/term_name", "xterm-256color");

        m_DataHandler =
                LoadPyModuleFromString(module_content,
                                       "term_data_handler_impl",
                                       "term_data_handler_impl.py").attr("create_data_handler")(termcap_dir, term_name);
    }
    catch(std::exception & e)
    {
        std::cerr << "load python data handler failed!"
                  << std::endl
                  << e.what()
                  << std::endl;
    }
    catch(...)
    {
        std::cerr << "load python data handler failed!"
                  << std::endl;
        PyErr_Print();
    }
}

void TermDataHandlerImpl::Start() {
    if (!m_Stopped)
        return;

    m_Stopped = false;

    m_DataHandlerThread.Start();
}

void TermDataHandlerImpl::Stop() {
    if (m_Stopped)
        return;

    m_Stopped = true;

    OnData("A", 1);

    m_DataHandlerThread.Join();
}
#pragma GCC visibility pop
