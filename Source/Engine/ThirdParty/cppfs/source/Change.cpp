
#include <Change.h>


namespace FileSys
{


Change::Change()
: m_operation(Change::None)
, m_path("")
{
}

Change::Change(Operation operation, const eastl::string & path)
: m_operation(operation)
, m_path(path)
{
}

Change::Change(Operation operation, eastl::string && path)
: m_operation(operation)
, m_path(eastl::move(path))
{
}

Change::Change(const Change & change)
: m_operation(change.m_operation)
, m_path(change.m_path)
{
}

Change::Change(Change && change)
: m_operation(eastl::move(change.m_operation))
, m_path(eastl::move(change.m_path))
{
}

Change::~Change()
{
}

Change & Change::operator=(const Change & change)
{
    m_operation = change.m_operation;
    m_path      = change.m_path;

    return *this;
}

Change & Change::operator=(Change && change)
{
    m_operation = eastl::move(change.m_operation);
    m_path      = eastl::move(change.m_path);

    return *this;
}

eastl::string Change::toString() const
{
    switch (m_operation)
    {
        case Change::CopyFile:   return "CP "    + m_path;
        case Change::CopyDir:    return "CPDIR " + m_path;
        case Change::RemoveFile: return "RM "    + m_path;
        case Change::RemoveDir:  return "RMDIR " + m_path;
        default:                 return "NOOP";
    }
}

Change::Operation Change::operation() const
{
    return m_operation;
}

const eastl::string & Change::path() const
{
    return m_path;
}


} // namespace FileSys
