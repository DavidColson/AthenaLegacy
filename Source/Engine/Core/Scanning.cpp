#include "Scanning.h"

#include "Log.h"

#include <EASTL/vector.h>

// Scanning utilities
///////////////////////

// @Improvement, use string_view for more performance and less memory thrashing

char Scan::Advance(ScanningState& scan)
{
	scan.current++;
	if (scan.file[scan.current - 1] == '\n')
	{
		scan.line++;
		scan.currentLineStart = scan.current;	
	}
	return scan.file[scan.current - 1];
}

char Scan::Peek(ScanningState& scan)
{
	return scan.file[scan.current];
}

char Scan::PeekNext(ScanningState& scan)
{
	return scan.file[scan.current + 1];
}

bool Scan::IsWhitespace(char c)
{
	if(c == ' ' || c == '\r' || c == '\t' || c == '\n')
		return true;
	return false;
}

void Scan::AdvanceOverWhitespace(ScanningState& scan)
{
	char c = scan.file[scan.current];
	while (IsWhitespace(c))
	{
		Advance(scan);
		c = Peek(scan);
		if (c == '\n')
		{
			scan.line++;
			scan.currentLineStart = scan.current + 1;
		}
	}
}

void Scan::AdvanceOverWhitespaceNoNewline(ScanningState& scan)
{
	char c = scan.file[scan.current];
	while (IsWhitespace(c))
	{
        if (c == '\n')
            break;
		Advance(scan);
		c = Peek(scan);
	}
}

bool Scan::IsAtEnd(ScanningState& scan)
{
	return scan.current >= scan.file.size();
}

bool Scan::IsPartOfNumber(char c)
{
	return (c >= '0' && c <= '9') || c == '-';
}

// Error reporting
//////////////////

eastl::string Scan::ExtractLineWithError(ScanningState& scan, int errorAt)
{
	// We give back the last two lines before the error, in case of cascading errors
	eastl::vector<eastl::string> lines;

	// Find the end of the line in which the error occured
	int lineEnd = errorAt;
	while (lineEnd < scan.file.size())
	{
		if (scan.file[lineEnd] == '\n')
		{
			break;
		}
		lineEnd++;
	}

	// Count backward finding the last 2 lines
	int lineStart = errorAt;
    if (scan.file[lineStart] == '\n')
        lineStart--;

	while (lineStart >= 0 && lines.size() < 2)
	{
		if (lineStart == 0 || scan.file[lineStart] == '\n')
		{
			// We use +1 and -1 to trim the newline characters, unless it's the start of the file
            if (lineStart == 0) lineStart -= 1;
			lines.push_back(scan.file.substr(lineStart + 1, (lineEnd - lineStart)-1));
			lineEnd = lineStart + 1;
		}
		lineStart--;
	}
	
	eastl::string error;
	for (int i = (int)lines.size() - 1; i >= 0; i--)
	{
		error.append_sprintf("%5i|%s", scan.line - i, lines[i].c_str());
	}
	error += "\n";
	return error;
}

void Scan::HandleError(ScanningState& scan, const char* message, int location)
{
	if (!scan.encounteredError)
	{
		eastl::string errorDiagram = ExtractLineWithError(scan, location);

		int columnToPointTo = (location - scan.currentLineStart) + 6; // 6 to account for the padded line number text
		for(int i = 0; i < columnToPointTo; i++)
			errorDiagram += ' ';
		errorDiagram.append_sprintf("^----- %s", message);

		Log::Crit("Encountered Parsing Error on line %i: \n%s\n", 
			scan.line, 
			errorDiagram.c_str()
			);
		scan.encounteredError = true;
	}
}