#include "Path.h"

#include <EASTL/vector.h>
#include <EASTL/algorithm.h>

Path::Path()
{
    stringPath = "";
}

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

Path Path::RootName() const
{
    if (hasValidRootname)
        return pathParts[0];;
    return "";
}

Path Path::RootDirectory() const
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

Path Path::RootPath() const
{
    return RootName() + RootDirectory();
}

Path Path::RelativePath() const
{
    eastl::string relativePath = stringPath;
    eastl::string rootPath = RootPath().AsString();
    // remove rootPath
    size_t pos = 0;
    if ((pos = relativePath.find(rootPath)) != eastl::string::npos)
    {
        relativePath.erase(0, pos + rootPath.length());
    }
    return relativePath;
}

Path Path::ParentPath() const
{
    eastl::string parentPath;
    for (size_t i = 0; i < eastl::max((int)pathParts.size()-2, 0); i++)
    {
        parentPath += pathParts[i];
    }
    return parentPath;
}

Path Path::Filename() const
{
    size_t numParts = pathParts.size();
    return (numParts > 0) ? pathParts[numParts-1] : "";
}

Path Path::Stem() const
{
    eastl::string filename = Filename().AsString();
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

Path Path::Extension() const
{
    eastl::string filename = Filename().AsString();
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

Path Path::RemoveTrailingSlash() const
{
    char c = stringPath[stringPath.length() - 1];
    if (c == '/' || c == '\\')
        return stringPath.substr(0, stringPath.length() - 1);
    return stringPath;
}

bool Path::IsEmpty() const
{
    return stringPath.empty();
}

bool Path::HasRootPath() const
{
    return !RootPath().IsEmpty();
}

bool Path::HasRootName() const
{
    return hasValidRootname;
}

bool Path::HasRootDirectory() const
{
    return !RootDirectory().IsEmpty();
}

bool Path::HasRelativePath() const
{
    return !RelativePath().IsEmpty();
}

bool Path::HasParentPath() const
{
    return !ParentPath().IsEmpty();
}

bool Path::HasFilename() const
{
    return !Filename().IsEmpty();
}

bool Path::HasStem() const
{
    return !Stem().IsEmpty();
}

bool Path::HasExtension() const
{
    return !Extension().IsEmpty();
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

const char* Path::AsRawString() const
{
    return stringPath.c_str();
}

Path& Path::operator/=(const Path& pathToAppend)
{
    if (IsEmpty())
    {
        stringPath = pathToAppend.stringPath;
        Analyze();
        return *this;   
    }

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
    if (IsEmpty())
        return;

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
    
    // Final token
    if (pos1 != eastl::string::npos)
    {
        token = stringPath.substr(pos1, stringPath.length() - pos1);
        if (!token.empty())
            pathParts.push_back(stringPath.substr(pos1, stringPath.length() - pos1));
    }
}

const Path::PathIterator Path::begin() const
{
    return PathIterator(pathParts.begin());
}

const Path::PathIterator Path::end() const
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

Path operator/(const Path& lhs, const Path& rhs)
{
    Path newPath = lhs;
    newPath /= rhs;
    return newPath;
}

Path operator+(const Path& lhs, const Path& rhs)
{
    Path newPath = lhs;
    newPath += rhs;
    return newPath;
}

bool operator==(const Path& lhs, const Path& rhs)
{
    return lhs.AsString() == rhs.AsString();
}

bool operator!=(const Path& lhs, const Path& rhs)
{
    return lhs.AsString() != rhs.AsString();
}
