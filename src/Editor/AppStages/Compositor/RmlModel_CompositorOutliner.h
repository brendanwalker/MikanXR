#pragma once

#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

struct RmlModel_CompositorObject
{
	Rml::String name;
	int depth;
	SelectionComponentWeakPtr selectionComponent;
};

class RmlModel_CompositorOutliner : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		AnchorObjectSystemPtr anchorSystemPtr,
		EditorObjectSystemPtr editorSystemPtr,
		StencilObjectSystemPtr stencilSystemPtr);
	virtual void dispose() override;

private:
	void addNewAnchor(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewQuad(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewBox(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewModel(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectObjectEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);

	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void stencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void onObjectInitialized(MikanObjectSystemPtr objectSystemPtr, MikanObjectPtr objectPtr);
	void onObjectDisposed(MikanObjectSystemPtr objectSystemPtr, MikanObjectConstPtr objectPtr);

	void rebuildComponentList();
	void updateSelection();
	void addTransformComponent(TransformComponentPtr transformComponentPtr, int depth);

	AnchorObjectSystemPtr m_anchorSystemPtr;
	EditorObjectSystemPtr m_editorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;
	Rml::Vector<RmlModel_CompositorObject> m_componentOutliner;
	int m_selectionIndex= -1;

	static bool s_bHasRegisteredTypes;
};
