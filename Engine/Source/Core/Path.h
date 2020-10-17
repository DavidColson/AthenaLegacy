#pragma once

#include "EASTL/string.h"

class Path
{
public:
    Path();

    Path(eastl::string pathAsString);

    Path(const char* pathAsString);

    Path(const Path& path);

    Path(Path&& path);

    Path& operator=(const Path& path);

    Path& operator=(Path&& path);

    Path RootName() const;

    Path RootDirectory() const;

    Path RootPath() const;

    Path RelativePath() const;

    Path ParentPath() const;

    Path Filename() const;

    Path Stem() const;

    Path Extension() const;

    Path RemoveTrailingSlash() const;

    bool IsEmpty() const;

    bool HasRootPath() const;

    bool HasRootName() const;

    bool HasRootDirectory() const;

    bool HasRelativePath() const;

    bool HasParentPath() const;

    bool HasFilename() const;

    bool HasStem() const;

    bool HasExtension() const;

    bool IsAbsolute() const;

    bool IsRelative() const;

    eastl::string AsString() const;

    const char* AsRawString() const;

    Path& operator/=(const Path& pathToAppend);
    
    Path& operator+=(const Path& pathToConcat);

    struct PathIterator
	{
		PathIterator(eastl::vector<eastl::string>::const_iterator _pathPartsIter) : pathPartsIter(_pathPartsIter) {}

		Path operator*() const;
		bool operator==(const PathIterator& other) const;
		bool operator!=(const PathIterator& other) const;

		PathIterator& operator++();

		eastl::vector<eastl::string>::const_iterator pathPartsIter;
	};

	const PathIterator begin() const;

	const PathIterator end() const;

protected:
    void Analyze();

    bool hasValidRootname = false;
    eastl::vector<eastl::string> pathParts;
    eastl::string   stringPath;
    char preferredSeparator = '/';
};

Path operator/(const Path& lhs, const Path& rhs);

Path operator+(const Path& lhs, const Path& rhs);

bool operator==(const Path& lhs, const Path& rhs);

bool operator!=(const Path& lhs, const Path& rhs);