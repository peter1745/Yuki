#pragma once

#include "Core/Logging.hpp"
#include "Memory/Unique.hpp"

namespace Yuki {

	template<typename TAppClass>
	struct EntryPoint
	{
		int operator()()
		{
			Yuki::LogInit();
			Unique<TAppClass> app = Unique<TAppClass>::Create();
			app->Initialize();
			app->Run();
			int exitCode = app->GetExitCode();
			app.Release();
			return exitCode;
		}
	};

}

#define YUKI_DECLARE_APPLICATION(AppClass)\
int main(int argc, char* argv[])\
{\
	return Yuki::EntryPoint<AppClass>()();\
}\
