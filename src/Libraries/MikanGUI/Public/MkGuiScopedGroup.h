#pragma once

class MkGuiScopedGroup
{
public:
	MkGuiScopedGroup();
	~MkGuiScopedGroup();

	MkGuiScopedGroup(const MkGuiScopedGroup&) = delete;
	MkGuiScopedGroup& operator=(const MkGuiScopedGroup&) = delete;
};
