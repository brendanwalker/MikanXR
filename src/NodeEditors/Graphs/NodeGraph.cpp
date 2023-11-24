#include "NodeGraph.h"

NodeGraph::NodeGraph()
{

}

NodeGraph::~NodeGraph()
{

}

int NodeGraph::allocateId()
{
	int newId = m_nextId;
	m_nextId++;
	return newId;
}