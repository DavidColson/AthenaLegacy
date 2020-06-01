
#include <FunctionalFileEventHandler.h>


namespace FileSys
{


FunctionalFileEventHandler::FunctionalFileEventHandler()
{
}

FunctionalFileEventHandler::FunctionalFileEventHandler(EventFunc funcFileEvent)
: m_funcFileEvent(funcFileEvent)
{
}

FunctionalFileEventHandler::~FunctionalFileEventHandler()
{
}

void FunctionalFileEventHandler::onFileEvent(FileHandle & fh, FileEvent event)
{
    if (m_funcFileEvent)
    {
        m_funcFileEvent(fh, event);
    }
}


} // namespace FileSys
