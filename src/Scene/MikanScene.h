#pragma once

#include <memory>
#include <vector>

class MikanObject;
typedef std::shared_ptr<MikanObject> MikanObjectPtr;

class MikanScene final
{
public:
	MikanScene();
	~MikanScene();

	void init();
	void dispose();

	void addMikanObject(MikanObjectPtr objectPtr);
	void removeMikanObject(MikanObjectPtr objectPtr);

	void update();
	void render();

private:
	class GlScene* m_glScene= nullptr;
	std::vector<MikanObjectPtr> m_objects;
};