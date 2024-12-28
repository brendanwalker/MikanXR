#include "RmlDataBinding.h"
#include "RmlModel.h"

bool RmlDataBinding::init(Rml::DataModelConstructor constructor)
{
	m_modelHandle= constructor.GetModelHandle();

	return (bool)m_modelHandle;
}
