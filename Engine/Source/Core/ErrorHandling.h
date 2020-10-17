#pragma once

#define ASSERT(condition, text) ((condition) ? (void)0 : Assertion(text, __FILE__, __LINE__))

int ShowAssertDialog(const char* errorMsg, const char* file, int line);

void Assertion(const char* errorMsg, const char* file, int line);