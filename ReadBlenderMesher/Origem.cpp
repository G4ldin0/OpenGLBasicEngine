#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <Shader.h>
#include <fstream>
#include <vector>
#include <regex>

#define WINDOW_WIDTH	1200
#define WINDOW_HEIGHT	720

void resizeWindow(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	glfwSetWindowSize(glfwGetCurrentContext(), width, height);
}

struct Vertex {
	glm::vec3 position;

	static void defineVertexArrayAttributes() {
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
	}
};

struct Mesh {
	Vertex* vertexArrayCPU;
	GLuint vertexCount;
	GLuint* indexArrayCPU;
	GLuint indexCount;
	GLuint VertexArrayObject;
	GLuint VertexBuferObject;
	GLuint ElementBuferObject;


	Mesh(const char* filePath) 
	{
		std::ifstream fileStream(filePath);
		if (!fileStream.is_open()) {
			std::cerr << "Failed to open file: " << filePath << std::endl;
			return;
		}
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;

		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<std::string> verticesRepresentation;
	
		std::string line;
		while (!fileStream.eof()) {
			fileStream >> line;
			if (line == "#") { // comentários
                std::getline(fileStream, line);
				continue;
			}
			else if (line == "mtllib") { // materiais //ignora por enquanto
				std::getline(fileStream, line);
				continue;
			}
			else if (line == "v") {
				glm::vec3 position;
				fileStream >> position.x >> position.y >> position.z;
				positions.push_back(position);
			}
			else if (line == "vn") {
				glm::vec3 normal;
				fileStream >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
			}
			else if (line == "vt") {
				glm::vec2 textureCoord;
				fileStream >> textureCoord.x >> textureCoord.y;
				texCoords.push_back(textureCoord);
			}
			else if (line == "f") {

				std::regex faceRegex(R"((\d+)(?:/(\d+)?/(\d+))?)");
				std::smatch match;
				std::getline(fileStream, line);
				auto it = std::sregex_iterator(line.begin(), line.end(), faceRegex);
				auto end = std::sregex_iterator();

				std::vector<std::string> verticesRepresentationInLine;
				std::vector<int> indicesInLine;

				int count = 0;
				for (auto i = it; i != end; ++i) {
					count++;
					match = *i;
					std::string vertexRef = match.str(0);
					verticesRepresentationInLine.push_back(vertexRef);
					//std::cout << match.str(0) << " " << match.str(1) << " " << match.str(2) << " " << match.str(3) << '\n';
					auto findVertex = std::find(verticesRepresentation.begin(), verticesRepresentation.end(), vertexRef);
					if (findVertex == verticesRepresentation.end()) {
						verticesRepresentation.push_back(vertexRef);
						Vertex newVertex;
						newVertex.position = positions[std::stoi(match.str(1)) - 1];
						//newVertex.textureCoord = (match.str(2).empty()) ? glm::vec2(0.0f) : texCoords[std::stoi(match.str(2)) - 1];
						//newVertex.normal = (match.str(3).empty()) ? glm::vec3(0.0f) : normals[std::stoi(match.str(3)) - 1];
						vertices.push_back(newVertex);
						indicesInLine.push_back(static_cast<GLuint>(vertices.size() - 1));
					}
					else {
						indicesInLine.push_back(static_cast<GLuint>(std::distance(verticesRepresentation.begin(), findVertex)));
					}
				}

				if (verticesRepresentationInLine.size() == 3) {
					// Triângulo
					indices.push_back(indicesInLine[0]);
					indices.push_back(indicesInLine[1]);
					indices.push_back(indicesInLine[2]);
				}
				else if (verticesRepresentationInLine.size() == 4) {
					// Quadrilátero
					indices.push_back(indicesInLine[0]);
					indices.push_back(indicesInLine[1]);
					indices.push_back(indicesInLine[2]);
					indices.push_back(indicesInLine[0]);
					indices.push_back(indicesInLine[2]);
					indices.push_back(indicesInLine[3]);
				}
			}
			else {
				std::getline(fileStream, line); // ignora outras linhas
			}
		}
		fileStream.close();

		vertexArrayCPU = vertices.data();
		indexArrayCPU = indices.data();
		vertexCount = static_cast<GLuint>(vertices.size());
		indexCount = static_cast<GLuint>(indices.size());

		createBuffers();
	}

	Mesh(Vertex* vertices, GLuint vCount, GLuint* indices, GLuint iCount): 
		vertexArrayCPU(vertices), 
		vertexCount(vCount), 
		indexArrayCPU(indices), 
		indexCount(iCount) 
	{
		createBuffers();
	}

	~Mesh() {
		glDeleteVertexArrays(1, &VertexArrayObject);
		glDeleteBuffers(1, &VertexBuferObject);
		glDeleteBuffers(1, &ElementBuferObject);
	}

	void useMesh() {
		glBindVertexArray(VertexArrayObject);
	}

	void drawMesh() {
		// generalizar para submeshes no futuro
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	}

	private:
		void createBuffers() {
			glGenVertexArrays(1, &VertexArrayObject);
			glBindVertexArray(VertexArrayObject);
			glGenBuffers(1, &VertexBuferObject);
			glBindBuffer(GL_ARRAY_BUFFER, VertexBuferObject);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexCount, vertexArrayCPU, GL_STATIC_DRAW);
			glGenBuffers(1, &ElementBuferObject);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuferObject);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexCount, indexArrayCPU, GL_STATIC_DRAW);

			Vertex::defineVertexArrayAttributes();

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
};


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Render Blender .obj file", nullptr, nullptr);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, resizeWindow);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwDestroyWindow(window);
		std::cout << "Failed to initialize GLAD\n";
		return -1;
	}

	Vertex vertices[] = {
		{{-0.5f, 0.0f, 0.0f}}, 
		{{0.0f, 0.5f, 0.0f}}, 
		{{0.5f, 0.5f, 0.0f}}
	};
	GLuint indices[] = {1, 2, 3};

	Mesh* abrindoArquivo = new Mesh("C:/Users/Galdino/Desktop/exemplo_caixa.obj");

	Mesh* teste = new Mesh(
		vertices, sizeof(vertices) / sizeof(Vertex),
		indices, sizeof(indices) / sizeof(GLuint)
	);
	Shader* basic = new Shader("basic.vs", "basic.ps");
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, -5.0f), 
		glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	glm::mat4 proj = glm::perspective(glm::radians(75.0f), 1.6f, 0.1f, 100.0f);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		});

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//glBindVertexArray(teste->VertexArrayObject);
	abrindoArquivo->useMesh();

	basic->Use();
	glUniformMatrix4fv(glGetUniformLocation(basic->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(basic->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(basic->ID, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

	abrindoArquivo->drawMesh();

	glBindVertexArray(0);

	glfwSwapBuffers(window);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	delete teste;
	delete basic;

	glfwTerminate();

	return 0;
}