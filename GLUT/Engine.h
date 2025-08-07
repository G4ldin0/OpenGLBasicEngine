#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "App.h"
#include "Window.h"

class Engine
{
	private:
		int Loop();

	public:
		static App* app;
		static Window* window;
		static double deltaTime;
		static bool paused;

		Engine();
		~Engine();

		int Start(App* app, const int width, const int height);
};

