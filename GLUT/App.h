#pragma once
class App
{
	public:
		App(){};
		~App(){};
		virtual void Init() = 0;
		virtual void Update(const float gameTime) = 0;
		virtual void Render() = 0;
		virtual void Finalize() = 0;
	
		virtual void OnKeyPress(int key, int action){};
		virtual void OnMouseMove(double x, double y){};
		virtual void OnMouseClick(int button, int action, double x, double y){};
};

