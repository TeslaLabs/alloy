/*
Copyright (c) 2006, Michael Kazhdan and Matthew Bolitho
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer. Redistributions in binary form must reproduce
the above copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the distribution. 

Neither the name of the Johns Hopkins University nor the names of its contributors
may be used to endorse or promote products derived from this software without specific
prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/

#ifndef CMD_LINE_PARSER_INCLUDED
#define CMD_LINE_PARSER_INCLUDED
#include <stdarg.h>
#include <string.h>


#ifdef WIN32
int strcasecmp(char* c1,char* c2);
#endif

class ArgumentReadable{
public:
	bool set;
	char* name;
	ArgumentReadable(const char* name);
	virtual ~ArgumentReadable(void);
	virtual int read(char** argv,int argc);
	virtual void writeValue(char* str);
};

class ArgumentInt : public ArgumentReadable {
public:
	int value;
	ArgumentInt(const char* name);
	ArgumentInt(const char* name,const int& v);
	int read(char** argv,int argc);
	void writeValue(char* str);
};
template<int Dim>
class ArgumentIntArray : public ArgumentReadable {
public:
	int values[Dim];
	ArgumentIntArray(const char* name);
	ArgumentIntArray(const char* name,const int v[Dim]);
	int read(char** argv,int argc);
	void writeValue(char* str);
};

class ArgumentFloat : public ArgumentReadable {
public:
	float value;
	ArgumentFloat(const char* name);
	ArgumentFloat(const char* name,const float& f);
	int read(char** argv,int argc);
	void writeValue(char* str);
};
template<int Dim>
class ArgumentFloatArray : public ArgumentReadable {
public:
	float values[Dim];
	ArgumentFloatArray(const char* name);
	ArgumentFloatArray(const char* name,const float f[Dim]);
	int read(char** argv,int argc);
	void writeValue(char* str);
};
class ArgumentString : public ArgumentReadable {
public:
	char* value;
	ArgumentString(const char* name);
	~ArgumentString();
	int read(char** argv,int argc);
	void writeValue(char* str);
};
class ArgumentStrings : public ArgumentReadable {
	int Dim;
public:
	char** values;
	ArgumentStrings(const char* name,int Dim);
	~ArgumentStrings(void);
	int read(char** argv,int argc);
	void writeValue(char* str);
};
template<int Dim>
class ArgumentStringArray : public ArgumentReadable {
public:
	char* values[Dim];
	ArgumentStringArray(const char* name);
	~ArgumentStringArray();
	int read(char** argv,int argc);
	void writeValue(char* str);
};

// This reads the arguments in argc, matches them against "names" and sets
// the values of "r" appropriately. Parameters start with "--"
void ArgumentParse(int argc, char **argv,int num,ArgumentReadable** r,int dumpError=1);

char* GetFileExtension(char* fileName);
char* GetLocalFileName(char* fileName);
char** ReadWords(const char* fileName,int& cnt);

#include "ArgumentParser.hpp"
#endif // CMD_LINE_PARSER_INCLUDED
