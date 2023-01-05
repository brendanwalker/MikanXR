#include "ElementFormControlNumericInput.h"
#include "ElementNumericSelection.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/ElementInstancer.h>
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Plugin.h>

namespace Rml {
namespace Mikan {


class RmlMikanPlugin : public Plugin {
public:
	void OnInitialise() override
	{
		numeric_instancer = MakeUnique<ElementInstancerGeneric<ElementFormControlNumericInput> >();
		numeric_selection_instancer = MakeUnique<ElementInstancerGeneric<ElementNumericSelection> >();
		
		Factory::RegisterElementInstancer("numeric", numeric_instancer.get());
		Factory::RegisterElementInstancer("#numeric_selection", numeric_selection_instancer.get());

		Log::Message(Log::LT_INFO, "Mikan-RML plugin initialised.");
	}

	void OnShutdown() override
	{
		delete this;
	}

	int GetEventClasses() override
	{
		return Plugin::EVT_BASIC;
	}

private:
	UniquePtr<ElementInstancerGeneric<ElementFormControlNumericInput>> numeric_instancer;
	UniquePtr<ElementInstancerGeneric<ElementNumericSelection>> numeric_selection_instancer;
};


void Initialise()
{
	RegisterPlugin(new RmlMikanPlugin);
}

} // namespace Lottie
} // namespace Rml
