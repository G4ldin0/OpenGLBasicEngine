#pragma once
#include <GLFW/glfw3.h>

class Window
{
	public:
		static GLFWwindow * currentWindow;

		static unsigned int width;
		static unsigned int height;
		const char* title;

        float colors[3];
		void setBackgroundColor(float r, float g, float b) {
			colors[0] = r;
			colors[1] = g;
			colors[2] = b;
		}

		Window();
		~Window();

		bool Create(const char* title, int width, int height);
		void Destroy();
		void SetTitle(const char* title);
		void SetSize(int width, int height);
		void SetPosition(int x, int y);
		void Show();
		void Hide();

};

