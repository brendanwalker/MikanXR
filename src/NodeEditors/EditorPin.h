#pragma once

#include "EditorNodeConstants.h"
#include "NodeEditorFwd.h"

#include <string>
#include <vector>

class EditorLink
{
public:
	EditorLink();

public:
	int id;
	EditorPinPtr pPin1;
	EditorPinPtr pPin2;
};

class EditorPin
{
public:
	EditorPin();

public:
	int id;
	EditorPinType type;
	int size;
	bool isOutput;
	std::string name;
	EditorNodePtr pNode;
	std::vector<EditorLinkPtr> connectedLinks;
};

class EditorFloatPin : public EditorPin
{
public:
	EditorFloatPin();

public:
	float value;
};

class EditorFloat2Pin : public EditorPin
{
public:
	EditorFloat2Pin();

public:
	float value[2]{};
};

class EditorFloat3Pin : public EditorPin
{
public:
	EditorFloat3Pin();

public:
	float value[3]{};
};

class EditorFloat4Pin : public EditorPin
{
public:
	EditorFloat4Pin();

public:
	float value[4]{};
};

class EditorIntPin : public EditorPin
{
public:
	EditorIntPin();

public:
	int value = 0;
};

class EditorInt2Pin : public EditorPin
{
public:
	EditorInt2Pin();

public:
	int value[2]{};
};

class EditorInt3Pin : public EditorPin
{
public:
	EditorInt3Pin();

public:
	int value[3]{};
};

class EditorInt4Pin : public EditorPin
{
public:
	EditorInt4Pin();

public:
	int value[4]{};
};

class EditorBlockPin : public EditorPin
{
public:
	EditorBlockPin();

public:
	EditorBlockPinType blockPinType;
	int index;
};