#include <iostream>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>

class TestApplication : public Yuki::Application
{
public:
	TestApplication()
	    : Yuki::Application("Test Application")
	{
	}

private:
	void OnInitialize() override
	{
		Yuki::LogInfo("OnInitialize!");
	}

	void OnRunLoop() override
	{
		Yuki::LogInfo("OnRunLoop");
	}
};

YUKI_DECLARE_APPLICATION(TestApplication)
