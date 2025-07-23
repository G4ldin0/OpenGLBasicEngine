#include "Engine.h"
#include <iostream>

App* Engine::app = nullptr;
Window* Engine::window = nullptr;
double Engine::deltaTime = 0.0f;
bool Engine::paused = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	Engine::window->SetSize(width, height);
}

Engine::Engine()
{
	glfwInit();
}

Engine::~Engine()
{
	if (window)
	{
		window->Destroy();
		delete window;
	}
	glfwTerminate();
	std::cout << "Engine terminated successfully.\n";
}

int Engine::Start(App* p_app)
{
	app = p_app;

	window = new Window();

	if (!window->Create("GLFW Engine", 800, 600))
	{
		window->Destroy();
		return -1;
	}

	glfwMakeContextCurrent(Window::currentWindow);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		window->Destroy();
		std::cout << "Failed to initialize GLAD\n";
		return -1;
	}

	glfwSetFramebufferSizeCallback(Window::currentWindow, framebuffer_size_callback);
    GLint samples;
    glGetIntegerv(GL_MAX_SAMPLES, &samples);
    if (samples > 0)
    {
        glEnable(GL_MULTISAMPLE);
        std::cout << "Multisampling is supported with " << samples << " samples.\n";
    }
    else
    {
        std::cout << "Multisampling is not supported.\n";
    }

	return Loop();
}

int Engine::Loop()
{
	glfwSetKeyCallback(Window::currentWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}
		else if (key == GLFW_KEY_P && action == GLFW_PRESS)
		{
			paused = !paused;
			if (paused)
			{
				std::cout << "Game Paused\n";
			}
			else
			{
				std::cout << "Game Resumed\n";
			}
		}
		Engine::app->OnKeyPress(key, action);
	});

	glfwSetCursorPosCallback(Window::currentWindow, [](GLFWwindow* window, double xpos, double ypos) {
	Engine::app->OnMouseMove(xpos, ypos); 
	});

	glfwSetMouseButtonCallback(Window::currentWindow, [](GLFWwindow* window, int button, int action, int mods) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		Engine::app->OnMouseClick(button, action, xpos, ypos);
	});

	app->Init();

	while (!glfwWindowShouldClose(Window::currentWindow))
	{
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - deltaTime;
		deltaTime = currentFrame;

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		if (!paused)
		{
			app->Update(deltaTime);
			app->Render();
		}
	}

	app->Finalize();

	return 0;

}