
#include <Diff.h>

#include <Log.h>

namespace FileSys
{


Diff::Diff()
{
}

Diff::~Diff()
{
}

void Diff::clear()
{
    m_changes.clear();
}

const eastl::vector<Change> & Diff::changes() const
{
    return m_changes;
}

void Diff::add(const Change & change)
{
    m_changes.push_back(change);
}

void Diff::add(Change && change)
{
    m_changes.push_back(change);
}

void Diff::add(Change::Operation operation, const eastl::string & path)
{
    m_changes.emplace_back(operation, path);
}

void Diff::add(Change::Operation operation, eastl::string && path)
{
    m_changes.emplace_back(operation, path);
}

eastl::string Diff::toString()
{
    eastl::string result;
    for (size_t i = 0; i < m_changes.size(); i++)
    {
        result.append(m_changes[i].toString());
    }
    return result;
}

void Diff::logPrint()
{
    for (size_t i = 0; i < m_changes.size(); i++)
    {
        Log::Info("%s", m_changes[i].toString());
    }
}


} // namespace FileSys
