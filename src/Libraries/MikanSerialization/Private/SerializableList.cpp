#include "SerializableList.h"
#include "SerializableList.rfks.h"

namespace Serialization
{
	struct BoolListData
	{
		std::vector<bool> list;

		BoolListData() = default;
		BoolListData(const BoolListData& other) : list(other.list) {}
	};

	BoolList::BoolList() : m_pimpl(new BoolListData()){ }
	BoolList::BoolList(const BoolList& other) : m_pimpl(new BoolListData(*other.m_pimpl)) {}
	BoolList::BoolList(BoolList&& other) noexcept : m_pimpl(new BoolListData(*other.m_pimpl)) {}
	BoolList::~BoolList() { delete m_pimpl; }

	BoolList& BoolList::operator=(const BoolList& other) 
	{ 
		m_pimpl->list = other.m_pimpl->list; 
		return *this;
	}

	std::size_t BoolList::size() const noexcept { return m_pimpl->list.size(); }
	void BoolList::push_back(bool value) { m_pimpl->list.push_back(value); }
	bool BoolList::at(std::size_t index) const { return m_pimpl->list.at(index); }
	void BoolList::resize(std::size_t newSize) { m_pimpl->list.resize(newSize); }
	void BoolList::clear() { m_pimpl->list.clear(); }
	bool BoolList::operator[](std::size_t index) const { return m_pimpl->list[index]; }

	const std::vector<bool>& BoolList::getVector() const { return m_pimpl->list; }
	std::vector<bool>& BoolList::getVectorMutable() { return m_pimpl->list; }
}