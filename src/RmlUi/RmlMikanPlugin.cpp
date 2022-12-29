#include "ElementFormControlNumericInput.h"
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
		instancer = MakeUnique<ElementInstancerGeneric<ElementFormControlNumericInput> >();
		
		Factory::RegisterElementInstancer("numeric", instancer.get());

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
	UniquePtr<ElementInstancerGeneric<ElementFormControlNumericInput>> instancer;
};


void Initialise()
{
	RegisterPlugin(new RmlMikanPlugin);
}

} // namespace Lottie
} // namespace Rml
