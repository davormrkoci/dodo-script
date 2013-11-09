#include "DSRScriptFactory.h"
#include "DSRScriptManager.h"

namespace dsr
{
	ScriptFactory::ScriptFactory()
	{
		ScriptManagerPtr()->Add(this);
	}

	ScriptFactory::~ScriptFactory()
	{
		ScriptManagerPtr()->Remove(this);
	}
}