
#include <cppfs/windows/LocalFileIterator.h>

#include <windows.h>

#include <cppfs/FilePath.h>
#include <cppfs/windows/LocalFileSystem.h>


namespace cppfs
{


LocalFileIterator::LocalFileIterator(eastl::shared_ptr<LocalFileSystem> fs, const eastl::string & path)
: m_fs(fs)
, m_path(path)
, m_index(-1)
, m_findHandle(nullptr)
, m_findData(nullptr)
{
	// Create find data
	m_findData = static_cast<void *>(new WIN32_FIND_DATA);

	// Read first directory entry
	readNextEntry();
}

LocalFileIterator::~LocalFileIterator()
{
	// Close search
	if (m_findHandle)
	{
		FindClose(m_findHandle);
	}

	// Destroy find data
	delete static_cast<WIN32_FIND_DATA *>(m_findData);
}

eastl::unique_ptr<AbstractFileIteratorBackend> LocalFileIterator::clone() const
{
    auto * twin = new LocalFileIterator(m_fs, m_path);

    while (twin->m_index < m_index)
    {
        twin->readNextEntry();
    }

    return eastl::unique_ptr<AbstractFileIteratorBackend>(twin);
}

AbstractFileSystem * LocalFileIterator::fs() const
{
    return static_cast<AbstractFileSystem *>(m_fs.get());
}

bool LocalFileIterator::valid() const
{
    return (m_findHandle != nullptr);
}

eastl::string LocalFileIterator::path() const
{
    return m_path;
}

int LocalFileIterator::index() const
{
    return m_index;
}

eastl::string LocalFileIterator::name() const
{
    // Check directory and entry handle
    if (!m_findHandle)
    {
        return "";
    }

    // Return filename of current item
	return eastl::string(static_cast<WIN32_FIND_DATA *>(m_findData)->cFileName);
}

void LocalFileIterator::next()
{
    readNextEntry();
}

void LocalFileIterator::readNextEntry()
{
	eastl::string filename;

	do
	{
		// Check find handle
		if (!m_findHandle)
		{
			// Open directory
			eastl::string query = FilePath(m_path).fullPath() + "/*";
			m_findHandle = FindFirstFileA(query.c_str(), static_cast<WIN32_FIND_DATA *>(m_findData));

			// Abort if directory could not be opened
			if (m_findHandle == INVALID_HANDLE_VALUE)
			{
				m_findHandle = nullptr;
				return;
			}
		}

		else {
			// Read next entry
			if (!FindNextFile(m_findHandle, static_cast<WIN32_FIND_DATA *>(m_findData)))
			{
				// No more files, close
				FindClose(m_findHandle);
				m_findHandle = nullptr;
				return;
			}
		}

		// Advance index
		m_index++;

		// Get filename
		filename = eastl::string(static_cast<WIN32_FIND_DATA *>(m_findData)->cFileName);
	} while (filename == ".." || filename == ".");
}


} // namespace cppfs
