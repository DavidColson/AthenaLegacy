#pragma once

#include <EASTL/string.h>

namespace Scan
{
    // Scanning utilities
    ///////////////////////

    struct ScanningState
    {
        eastl::string file;
        int current{ 0 };
        int currentLineStart{ 0 };
        int line{ 1 };
        bool encounteredError{ false };
    };

    char Advance(ScanningState& scan);

    bool Match(ScanningState& scan, char expected);

    char Peek(ScanningState& scan);

    char PeekNext(ScanningState& scan);

    bool IsWhitespace(char c);

    void AdvanceOverWhitespace(ScanningState& scan);

    void AdvanceOverWhitespaceNoNewline(ScanningState& scan);

    bool IsAtEnd(ScanningState& scan);

    bool IsPartOfNumber(char c);

    bool IsDigit(char c);

    bool IsHexDigit(char c);

    bool IsAlpha(char c);
    
    bool IsAlphaNumeric(char c);

    eastl::string ParseToString(ScanningState& scan, char bound);

    // Error reporting
    //////////////////

    eastl::string ExtractLineWithError(ScanningState& scan, int errorAt);

    void HandleError(ScanningState& scan, const char* message, int location);

}