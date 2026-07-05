/*
 * main.cpp
 * 
 * 05-07-2026 by madpl
 */
#include <core/App.hpp>


int main(void)
{
	App app;
	
	if(not app.initialize())
		return -1;
	
	app.run();
	
	return 0;
}
