#include "Engine.h"
#include "Shader.h"
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct VertexArrayArgument {
	GLenum type;
	GLint size;
	GLboolean normalized;
	GLsizei stride;
	GLvoid* pointer;
};

struct Mesh {
	GLuint VAO, VBO, EBO;
	GLuint vertexCount;
	GLuint startPos;
	GLuint strife;

	Mesh(float* vertices, GLuint verticesCount, GLint* indices, GLuint indicesCount, GLuint startPos, VertexArrayArgument* arguments, GLuint argumentCount) {
		this->startPos = startPos;
		this->vertexCount = verticesCount;
		this->strife = 3; // 3 floats por vértice (x, y, z)

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, verticesCount * sizeof(float), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(int), indices, GL_STATIC_DRAW);

		for (GLuint i = 0; i < argumentCount; i++)
		{
			glVertexAttribPointer(i, arguments[i].size, arguments[i].type, arguments[i].normalized, arguments[i].stride, (void*)(arguments[i].pointer));
			glEnableVertexAttribArray(i);
		}


		glBindVertexArray(0);
	}
};

float triangleVertices[] = {
	-0.75f, -0.75f, 0.0f, // Bottom left vertex
	0.75f, -0.75f, 0.0f,  // Bottom right vertex
	0.0f,  0.75f, 0.0f   // Top vertex
};

GLuint triangleIndices [] = {
	0, 1, 2 // Triangle indices
};

float screenQuadVertices[] = {
	-1.0f,-1.0f, 0.0f,		0.0f,	0.0f, // Bottom left vertex
	1.0f, -1.0f, 0.0f,		1.0f,	0.0f, // Bottom right vertex
	1.0f,  1.0f, 0.0f,		1.0f,	 1.0f, // Top right vertex
	-1.0f, 1.0f, 0.0f,		0.0f,	 1.0f // Top left vertex
};
unsigned int screenQuadIndices[] = {
	0, 1, 2, // First triangle
	0, 2, 3  // Second triangle
};

class BasicApp : public App
{
private:

	// TODO: encapsular numa classe Mesh
	unsigned int VBO, VAO, EBO;


	Shader* shader;
	Shader* fxaaShader;
	Shader* screenShader;

	float green_level = 0.0f;

	float time_passed = 0.0f;

	GLuint aa = 0;

	// simples framebuffer
	GLuint FBO, colorTexture, RBO;

	GLuint msFBO, msColorbuffer, msRBO;
	GLuint samples = 16;

	GLuint fxaaFBO, fxaaTextureColorBuffer, fxaaRBO; // FBO para FXAA


	GLuint intermediateFBO, intermediateTextureColorBuffer, intermediateRBO; // FBO para armazenar o resultado intermediário

	GLuint screenVAO, screenEBO, screenVBO;


	glm::mat4 transform;

	glm::mat4 view;
	glm::mat4 proj;

	float cameraRadius = 5.0f;
	float cameraAngleX;
	float cameraAngleY;

public:

	void Init() override {
		//(float*, GLuint, int*, GLuint, GLuint, VertexArrayArgument* , GLuint)
        //triangle = Mesh(new float, 9, triangleIndices, 3, 0, new VertexArrayArgument{ GL_FLOAT, 3, GL_FALSE, 3 * sizeof(float), (void*)(0) }, 1);

		proj = glm::perspective(glm::radians(60.0f), (float)Window::width / (float)Window::height, 0.1f, 100.0f);


		shader = new Shader("shaders/basic.vs", "shaders/basic.ps");
		screenShader = new Shader("shaders/screen.vs", "shaders/screen.ps");

		fxaaShader = new Shader("shaders/fxaa.vs", "shaders/fxaa.ps");

		// Mesh buffers
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		// Vertex Array Object
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices), triangleIndices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

		// Framebuffer setup
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glGenTextures(1, &colorTexture);
		glBindTexture(GL_TEXTURE_2D, colorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Window::width, Window::height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Window::width, Window::height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		//Multisample framebuffer setup
		glGenFramebuffers(1, &msFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
		glGenRenderbuffers(1, &msRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, msRBO);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, Window::width, Window::height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glGenRenderbuffers(1, &msColorbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, msColorbuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGB, Window::width, Window::height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msRBO);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msColorbuffer);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::MS_FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// Framebuffer for intermediate rendering
		glGenFramebuffers(1, &intermediateFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
		glGenTextures(1, &intermediateTextureColorBuffer);
		glBindTexture(GL_TEXTURE_2D, intermediateTextureColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Window::width, Window::height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediateTextureColorBuffer, 0);
		glGenRenderbuffers(1, &intermediateRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, intermediateRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Window::width, Window::height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, intermediateRBO);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::INTERMEDIATE_FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		// Screen quad setup
		glGenVertexArrays(1, &screenVAO);
		glBindVertexArray(screenVAO);
		glGenBuffers(1, &screenVBO);
		glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertices), screenQuadVertices, GL_STATIC_DRAW);
		glGenBuffers(1, &screenEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenQuadIndices), screenQuadIndices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);

	}

	void Update(const float gameTime) override {
		time_passed += gameTime;
		transform = glm::mat4(1.0f);
		//transform = glm::rotate(transform, glm::radians(gameTime), glm::vec3(0.0f, 0.0f, 1.0f));
		//transform = glm::scale(transform, glm::vec3((sin(gameTime) + 1.25f) / 2.0f, (cos(1.5f * gameTime) + 1.25f) / 2.0f, 1.0f));

        float xPos = cameraRadius * sin(cameraAngleX) * cos(cameraAngleY); // Posição X da câmera
        float yPos = cameraRadius * sin(cameraAngleY); // Posição Y da câmera
        float zPos = cameraRadius * cos(cameraAngleX) * cos(cameraAngleY); // Posição Z da câmera

		view = glm::lookAt(glm::vec3(xPos, yPos, zPos),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void Render() override {

		glEnable(GL_DEPTH_TEST);

		shader->Use();
		shader->SetFloat("green_level", green_level);
		glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(transform));
		glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

		switch (aa) {
			case 0:
				// No antialiasing
				glBindFramebuffer(GL_FRAMEBUFFER, FBO);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
				break;

			case 1:
				// FXAA
				glBindFramebuffer(GL_FRAMEBUFFER, FBO);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


				glBindFramebuffer(GL_FRAMEBUFFER, fxaaFBO);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				fxaaShader->Use();
				fxaaShader->SetFloat("offsetWidth", 1.0f / (Window::width / 2.0f));
				fxaaShader->SetFloat("offsetHeight", 1.0f / (Window::height / 2.0f));
				glBindTexture(GL_TEXTURE_2D, colorTexture);
				glBindVertexArray(screenVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, fxaaFBO);
				break;

			case 2:
				// MSAA
				glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				glEnable(GL_MULTISAMPLE);
				glBindVertexArray(VAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glDisable(GL_MULTISAMPLE);
				glBindBuffer(GL_READ_FRAMEBUFFER, msFBO);
				break;

		}

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
		glBlitFramebuffer(0, 0, Window::width, Window::height, 0, 0, Window::width, Window::height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		screenShader->Use();
		glBindTexture(GL_TEXTURE_2D, intermediateTextureColorBuffer);
		glBindVertexArray(screenVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		glfwSwapBuffers(Window::currentWindow);
		glfwPollEvents();

	}

	void Finalize() override {
		delete shader;
		delete fxaaShader;
		delete screenShader;	

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);

		glDeleteVertexArrays(1, &screenVAO);
		glDeleteBuffers(1, &screenVBO);
		glDeleteBuffers(1, &screenEBO);


		glDeleteRenderbuffers(1, &colorTexture);
		glDeleteRenderbuffers(1, &RBO);
		glDeleteFramebuffers(1, &FBO);

		glDeleteRenderbuffers(1, &msColorbuffer);
		glDeleteRenderbuffers(1, &msRBO);
		glDeleteFramebuffers(1, &msFBO);

		glDeleteRenderbuffers(1, &fxaaTextureColorBuffer);
		glDeleteRenderbuffers(1, &fxaaRBO);
		glDeleteFramebuffers(1, &fxaaFBO);

		glDeleteTextures(1, &intermediateTextureColorBuffer);
		glDeleteTextures(1, &intermediateRBO);
		glDeleteFramebuffers(1, &intermediateFBO);


	}

	void OnKeyPress(int key, int action) override {
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			std::cout << "pressionado W \n";
            cameraRadius = glm::clamp(cameraRadius - 0.1f, 2.5f, 7.5f); // Move camera closer
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			cameraRadius = glm::clamp(cameraRadius + 0.1f, 2.5f, 7.5f); // Move camera away
		}
	}

    void OnMouseMove(double x, double y) override {
    // Update green level based on mouse position
    green_level = static_cast<float>(y) / Window::height;
    if (green_level > 1.0f) green_level = 1.0f;
    if (green_level < 0.0f) green_level = 0.0f;

    cameraAngleX = (x / Window::width) * 2 * glm::pi<float>(); // Ângulo em torno do eixo Y
    cameraAngleY = (y / Window::height) * glm::pi<float>() - (glm::pi<float>() / 2); // Ângulo em torno do eixo X
}

	void OnMouseClick(int button, int action, double x, double y) override
	{
		// o pior jeito do mundo de fazer isso...
        static bool leftButtonPressed = false;

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !leftButtonPressed) {
            leftButtonPressed = true;
            // Ação a ser executada quando o botão esquerdo do mouse é pressionado pela primeira vez
			aa = (aa + 1) % 2; // Alterna o estado de antialiasing
        } else if (action == GLFW_RELEASE) {
            leftButtonPressed = false; // Reseta quando o botão é solto
        }
	}

};

int main()
{
	Engine engine;
	BasicApp app;

	engine.Start(&app);

	return 0;
}