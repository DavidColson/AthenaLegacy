#include "Path.h"

#include <EASTL/vector.h>
#include <EASTL/algorithm.h>

Path::Path(eastl::string pathAsString)
{
    stringPath = pathAsString;
    Analyze();
}

Path::Path(const char* pathAsString)
{
    stringPath = pathAsString;
    Analyze();
}

Path::Path(const Path& path)
{
    stringPath = path.stringPath;
    pathParts = path.pathParts;
    hasValidRootname = path.hasValidRootname;
}

Path::Path(Path&& path)
{
    stringPath = eastl::move(path.stringPath);
    pathParts = eastl::move(path.pathParts);
    hasValidRootname = eastl::move(path.hasValidRootname);
}

Path& Path::operator=(const Path& path)
{
    stringPath = path.stringPath;
    pathParts = path.pathParts;
    hasValidRootname = path.hasValidRootname;
    return *this;
}

Path& Path::operator=(Path&& path)
{
    stringPath = eastl::move(path.stringPath);
    pathParts = eastl::move(path.pathParts);
    hasValidRootname = eastl::move(path.hasValidRootname);
    return *this;
}

eastl::string Path::RootName() const
{
    if (hasValidRootname)
        return pathParts[0];;
    return "";
}

eastl::string Path::RootDirectory() const
{
    if (RootName() == "")
    {
        if ((pathParts[0] == "/" || pathParts[0] == "\\"))
            return pathParts[0];
        else
            return "";
    }
    return pathParts[1];
}

eastl::string Path::RootPath() const
{
    return RootName() + RootDirectory();
}

eastl::string Path::RelativePath() const
{
    eastl::string relativePath = stringPath;
    eastl::string rootPath = RootPath();
    // remove rootPath
    size_t pos = 0;
    if ((pos = relativePath.find(RootPath())) != eastl::string::npos)
    {
        relativePath.erase(0, pos + rootPath.length());
    }
    return relativePath;
}

eastl::string Path::ParentPath() const
{
    eastl::string parentPath;
    for (size_t i = 0; i < eastl::max((int)pathParts.size()-2, 0); i++)
    {
        parentPath += pathParts[i];
    }
    return parentPath;
}

eastl::string Path::Filename() const
{
    size_t numParts = pathParts.size();
    return (numParts > 0) ? pathParts[numParts-1] : "";
}

eastl::string Path::Stem() const
{
    eastl::string filename = Filename();
    size_t pos = filename.find_first_of('.', 1);

    if (filename == "." || filename == ".." || pos == eastl::string::npos)
    {
        return filename;
    }
    else
    {
        return filename.substr(0, pos);
    }

}

eastl::string Path::Extension() const
{
    eastl::string filename = Filename();
    size_t pos = filename.find_first_of('.', 1);

    if (filename == "." || filename == ".." || pos == eastl::string::npos)
    {
        return "";
    }
    else
    {
        return filename.substr(pos);
    }

}

bool Path::IsEmpty() const
{
    return stringPath.empty();
}

bool Path::HasRootPath() const
{
    return !RootPath().empty();
}

bool Path::HasRootName() const
{
    return hasValidRootname;
}

bool Path::HasRootDirectory() const
{
    return !RootDirectory().empty();
}

bool Path::HasRelativePath() const
{
    return !RelativePath().empty();
}

bool Path::HasParentPath() const
{
    return !ParentPath().empty();
}

bool Path::HasFilename() const
{
    return !Filename().empty();
}

bool Path::HasStem() const
{
    return !Stem().empty();
}

bool Path::HasExtension() const
{
    return !Extension().empty();
}

bool Path::IsAbsolute() const
{
    return hasValidRootname;
}

bool Path::IsRelative() const
{
    return !hasValidRootname;
}

eastl::string Path::AsString() const
{
    return stringPath;
}

Path& Path::operator/=(const Path& pathToAppend)
{
    if (pathToAppend.IsAbsolute())
    {
        stringPath = pathToAppend.stringPath;
        Analyze();
        return *this;
    }

    if (stringPath[stringPath.size() - 1] == '/' || stringPath[stringPath.size() - 1] == '\\')
        stringPath += pathToAppend.stringPath;
    else
        stringPath += preferredSeparator + pathToAppend.stringPath;
    Analyze();
    return *this;
}
    
Path& Path::operator+=(const Path& pathToConcat)
{
    stringPath += pathToConcat.stringPath;
    Analyze();
    return *this;
}

size_t FindNextSeparatorLoc(const eastl::string& path, size_t startPos)
{
    size_t nextForward = path.find('/', startPos);
    size_t nextBackward = path.find('\\', startPos);

    return nextForward < nextBackward ? nextForward : nextBackward;
} 

void Path::Analyze()
{
    pathParts.clear();
    // Split path into parts
    size_t pos1 = 0;
    size_t pos2 = 0;
    eastl::string token;

    // Parse rootname
    if (stringPath[1] == ':')
    {
        pathParts.push_back(stringPath.substr(0, 2));
        pos1 = 2; pos2 = 2; hasValidRootname = true;
    }
    else if (stringPath.substr(0, 2) == "\\\\" || stringPath.substr(0, 2) == "//")
    {
        size_t rootEnd = FindNextSeparatorLoc(stringPath, 2);
        pathParts.push_back(stringPath.substr(0, rootEnd));
        pos1 = rootEnd; pos2 = rootEnd; hasValidRootname = true;
    }
    else if (stringPath.substr(0, 3) == "\\\\." || stringPath.substr(0, 3) == "\\\\?" || stringPath.substr(0, 3) == "//?" || stringPath.substr(0, 3) == "//.")
    {
        pathParts.push_back(stringPath.substr(0, 3));
        pos1 = 3; pos2 = 3; hasValidRootname = true;
    }

    while((pos2 = FindNextSeparatorLoc(stringPath, pos1)) != eastl::string::npos)
    {
        token = stringPath.substr(pos1, pos2-pos1);
        if (!token.empty())
            pathParts.push_back(token);
        pathParts.push_back(stringPath.substr(pos2, 1)); // separator
        pos1 = pos2+1;
    }
    if (pos1 != eastl::string::npos)
        pathParts.push_back(stringPath.substr(pos1, stringPath.length() - pos1));
}

const Path::PathIterator Path::begin() 
{
    return PathIterator(pathParts.begin());
}

const Path::PathIterator Path::end()
{
    return PathIterator(pathParts.end());
}

Path Path::PathIterator::operator*() const 
{ 
    return Path(*pathPartsIter);
}

bool Path::PathIterator::operator==(const PathIterator& other) const 
{
    return pathPartsIter == other.pathPartsIter;
}
bool Path::PathIterator::operator!=(const PathIterator& other) const 
{
    return pathPartsIter != other.pathPartsIter;
}

Path::PathIterator& Path::PathIterator::operator++()
{
    if (Path(*pathPartsIter).hasValidRootname)
    {
        ++pathPartsIter;
        return *this;
    }
    
    ++pathPartsIter;
    if (*pathPartsIter == "\\" || *pathPartsIter == "/")
        ++pathPartsIter;
    return *this;
}

