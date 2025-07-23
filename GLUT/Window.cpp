#include "Window.h"

GLFWwindow* Window::currentWindow = nullptr;
unsigned int Window::width = 0;
unsigned int Window::height = 0;

Window::Window()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

Window::~Window()
{
}

bool Window::Create(const char* p_title, int p_width, int p_height)
{
	width = p_width;
	height = p_height;
	title = p_title;
	currentWindow = glfwCreateWindow(p_width, p_height, p_title, nullptr, nullptr);
	return currentWindow != nullptr;
}

void Window::Destroy()
{
	glfwDestroyWindow(currentWindow);
	currentWindow = nullptr;
}

void Window::SetTitle(const char* p_title)
{
	glfwSetWindowTitle(glfwGetCurrentContext(), p_title);
	title = p_title;
}

void Window::SetSize(int p_width, int p_height)
{
	glfwSetWindowSize(glfwGetCurrentContext(), p_width, p_height);
	glViewport(0, 0, p_width, p_height);
	width = p_width;
	height = p_height;
}

inline void Window::SetPosition(int x, int y)
{
	glfwSetWindowPos(glfwGetCurrentContext(), x, y);
}

inline void Window::Show()
{
	glfwShowWindow(glfwGetCurrentContext());
}

inline void Window::Hide()
{
	glfwHideWindow(glfwGetCurrentContext());
}

