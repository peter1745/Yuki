#pragma once

#include <Engine/Core/App.hpp>

#define YukiApp(AppType)\
void AppType##Main();\
int main(int argc, char* argv[])\
{\
	std::string filepath = argv[0];\
	AppType##Main();\
	AppRunner<AppType>(filepath).Run();\
	return 0;\
}\
void AppType##Main()
