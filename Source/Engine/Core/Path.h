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

    eastl::string RootName() const;

    eastl::string RootDirectory() const;

    eastl::string RootPath() const;

    eastl::string RelativePath() const;

    eastl::string ParentPath() const;

    eastl::string Filename() const;

    eastl::string Stem() const;

    eastl::string Extension() const;

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

    Path& operator/=(const Path& pathToAppend);
    
    Path& operator+=(const Path& pathToConcat);

    struct PathIterator
	{
		PathIterator(eastl::vector<eastl::string>::iterator _pathPartsIter) : pathPartsIter(_pathPartsIter) {}

		Path operator*() const;
		bool operator==(const PathIterator& other) const;
		bool operator!=(const PathIterator& other) const;

		PathIterator& operator++();

		eastl::vector<eastl::string>::iterator pathPartsIter;
	};

	const PathIterator begin();

	const PathIterator end();

protected:
    void Analyze();

    bool hasValidRootname = false;
    eastl::vector<eastl::string> pathParts;
    eastl::string   stringPath;
    char preferredSeparator = '/';
};