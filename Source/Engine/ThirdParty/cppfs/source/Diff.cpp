
#include <cppfs/Diff.h>

#include <iostream>


namespace cppfs
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

void Diff::print(std::ostream & stream)
{
    for (size_t i = 0; i < m_changes.size(); i++)
    {
        stream << std::string(m_changes[i].toString().c_str()) << std::endl;
    }
}

void Diff::print()
{
    print(std::cout);
}


} // namespace cppfs
