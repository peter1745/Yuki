#pragma once

#include <Engine/Core/App.hpp>

#define YukiApp(AppType)\
static void AppType##Main();\
int main(int argc, char* argv[])\
{\
	std::string filepath = argv[0];\
	AppType##Main();\
	::Yuki::AppRunner<AppType>(filepath).Run();\
	return 0;\
}\
static void AppType##Main()
