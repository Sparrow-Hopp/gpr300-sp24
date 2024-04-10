#include <stdio.h>
#include <math.h>
#include <iostream>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>
#include <sh/framebuffer.h>
#include <sh/shadowbuffer.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
unsigned int screenWidth = 1080;
unsigned int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Transform planeTransform;
ew::Transform monkeyTransforms[8];
ew::CameraController cameraController;
ew::Camera camera, lightCamera;

sh::ShadowBuffer shadowbuffer;
sh::FrameBuffer framebuffer;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

struct Blur {
	float intensity;
}blur;

struct Light {
	glm::vec3 direction = glm::vec3(1.0f);
	ImVec4 colour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}lightSpecs;

struct Shadow {
	float camDistance = 10.0f;
	float camSize = 10.0f;
	float minBias = 0.005f;
	float maxBias = 0.015f;
}shadowSpecs;

struct Node
{
	glm::mat4 localTransform;
	glm::mat4 globalTransform;
	unsigned int parentIndex;
};

struct Hierarchy
{
	Node* nodes;
	unsigned int nodeCount;
}parentHierarchy;

void SolveFK(Hierarchy hierarchy)
{
	for (int i = 0; i < static_cast<int>(hierarchy.nodeCount); i++)
	{
		if (hierarchy.nodes[i].parentIndex == -1)
			hierarchy.nodes[i].globalTransform = hierarchy.nodes[i].localTransform;
		else
			hierarchy.nodes[i].globalTransform = hierarchy.nodes[hierarchy.nodes[i].parentIndex].globalTransform * hierarchy.nodes[i].localTransform;
	}
}


int main() {
	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader postProcessingShader = ew::Shader("assets/screenQuad.vert", "assets/postProcess.frag");
	ew::Shader shadowShader = ew::Shader("assets/depthOnly.vert", "assets/depthOnly.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	ew::Mesh planeMesh = ew::Mesh(ew::createPlane(10, 10, 5));
	\
	planeTransform.position = glm::vec3(0.0f, -2.0f, 0.0f);

	//Handles to OpenGL object are unsigned integers
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	glEnable(GL_CULL_FACE);

	//create buffers
	shadowbuffer = sh::createShadowBuffer(2048, 2048);
	framebuffer = sh::createFramebuffer(screenWidth, screenHeight, (int)(GL_RGB16F));

	unsigned int dummyVAO;
	glCreateVertexArrays(1, &dummyVAO);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	blur.intensity = 1.0f;

	//set viewing camera values
	camera.position = glm::vec3(0.0f, 5.0f, 5.0f);
	camera.target = glm::vec3(0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	//set lighting camera values
	lightCamera.position = glm::vec3(10.0f, shadowSpecs.camDistance, 10.0f);
	lightCamera.target = glm::vec3(0.0f);
	lightCamera.aspectRatio = 1.0f;
	lightCamera.orthographic = true;
	lightCamera.orthoHeight = shadowSpecs.camSize;

	//set up hierarchy
	{
		//set number of nodes
		parentHierarchy.nodeCount = 8;
		parentHierarchy.nodes = new Node[parentHierarchy.nodeCount];

		//set the positions of each monkey
		monkeyTransforms[0].position = glm::vec3(0.0);
		monkeyTransforms[1].position = glm::vec3(-2.0, 0.0, 0.0);
		monkeyTransforms[2].position = glm::vec3(2.0, 0.0, 0.0);
		monkeyTransforms[3].position = glm::vec3(0.0, 2.0, 0.0);
		monkeyTransforms[4].position = glm::vec3(-2.0, 0.0, 0.0);
		monkeyTransforms[5].position = glm::vec3(2.0, 0.0, 0.0);
		monkeyTransforms[6].position = glm::vec3(0.0, 2.0, 0.0);
		monkeyTransforms[7].position = glm::vec3(0.0, 2.0, 0.0);

		//set the scales of each monkey that isn't 1
		for (int i = 1; i < static_cast<int>(parentHierarchy.nodeCount); i++)
		{
			monkeyTransforms[i].scale = glm::vec3(0.75);
		}

		//set the transforms of each node in the hierarchy
		for (int i = 0; i < static_cast<int>(parentHierarchy.nodeCount); i++)
		{
			parentHierarchy.nodes[i].localTransform = monkeyTransforms[i].modelMatrix();
		}

		//set the parents of each node
		parentHierarchy.nodes[0].parentIndex = -1;
		parentHierarchy.nodes[1].parentIndex = 0;
		parentHierarchy.nodes[2].parentIndex = 0;
		parentHierarchy.nodes[3].parentIndex = 0;
		parentHierarchy.nodes[4].parentIndex = 1;
		parentHierarchy.nodes[5].parentIndex = 2;
		parentHierarchy.nodes[6].parentIndex = 4;
		parentHierarchy.nodes[7].parentIndex = 5;
	}

	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//set the transforms of each node in the hierarchy
		for (int i = 0; i < static_cast<int>(parentHierarchy.nodeCount); i++)
		{
			parentHierarchy.nodes[i].localTransform = monkeyTransforms[i].modelMatrix();
		}
		//solve for global monkey transforms
		SolveFK(parentHierarchy);

		//RENDER TO SHADOW BUFFER
		{
			glBindFramebuffer(GL_FRAMEBUFFER, shadowbuffer.fbo);
			glViewport(0, 0, shadowbuffer.width, shadowbuffer.height);
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);

			shadowShader.use();

			shadowShader.setMat4("_ViewProjection", lightCamera.projectionMatrix() * lightCamera.viewMatrix());

			//draw all monkeys
			{
				for (int i = 0; i < static_cast<int>(parentHierarchy.nodeCount); i++)
				{
					shadowShader.setMat4("_Model", parentHierarchy.nodes[i].globalTransform);
					monkeyModel.draw();
				}
			}

			shadowShader.setMat4("_Model", planeTransform.modelMatrix());
			planeMesh.draw();
		}
		//RENDER TO FRAMEBUFFER WITH SHADOW MAP
		{
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
			glViewport(0, 0, framebuffer.width, framebuffer.height);

			glClearColor(0.0f, 0.6f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glCullFace(GL_BACK);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, brickTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, shadowbuffer.shadowMap);

			glActiveTexture(GL_TEXTURE0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		}
		//USE MONKEY SHADER AND DRAW
		{
			//Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
			shader.use();
			shader.setInt("_MainTex", 0);
			shader.setInt("_ShadowMap", 1);

			shader.setVec3("_EyePos", camera.position);
			shader.setVec3("_LightPos", glm::normalize(lightSpecs.direction));
			shader.setVec3("_LightColor", glm::vec3(lightSpecs.colour.x, lightSpecs.colour.y, lightSpecs.colour.z));

			shader.setFloat("_Material.Ka", material.Ka);
			shader.setFloat("_Material.Kd", material.Kd);
			shader.setFloat("_Material.Ks", material.Ks);
			shader.setFloat("_Material.Shininess", material.Shininess);
			shader.setFloat("_Shadow.minBias", shadowSpecs.minBias);
			shader.setFloat("_Shadow.maxBias", shadowSpecs.maxBias);

			shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
			shader.setMat4("_LightViewProj", lightCamera.projectionMatrix() * lightCamera.viewMatrix());

			//draw all monkeys
			{
				for (int i = 0; i < static_cast<int>(parentHierarchy.nodeCount); i++)
				{
					shader.setMat4("_Model", parentHierarchy.nodes[i].globalTransform);
					monkeyModel.draw();
				}
			}


			shader.setMat4("_Model", planeTransform.modelMatrix());
			planeMesh.draw();

			glBindTextureUnit(0, framebuffer.colorBuffer[0]);
		}
		//SWAP TO BACKGROUND AND DRAW TO FULLSCREEN QUAD USING POSTPROCESSING SHADER
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
			glClearColor(0.0f, 0.4f, 0.8f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			postProcessingShader.use();
			postProcessingShader.setFloat("_Blur.intensity", blur.intensity);

			glBindVertexArray(dummyVAO);
			glBindTexture(GL_TEXTURE_2D, framebuffer.colorBuffer[0]);
			glDrawArrays(GL_TRIANGLES, 0, 6); //6 for quad, 3 for triangle
		}

		//rotate monkeys
		{
			monkeyTransforms[0].rotation = glm::rotate(monkeyTransforms[0].rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
			monkeyTransforms[1].rotation = glm::rotate(monkeyTransforms[1].rotation, deltaTime, glm::vec3(1.0, 0.0, 0.0));
			monkeyTransforms[2].rotation = glm::rotate(monkeyTransforms[2].rotation, deltaTime, glm::vec3(-1.0, 0.0, -0.0));
			monkeyTransforms[3].rotation = glm::rotate(monkeyTransforms[3].rotation, deltaTime, glm::vec3(0.0, -1.0, 0.0));
			monkeyTransforms[6].rotation = glm::rotate(monkeyTransforms[6].rotation, deltaTime, glm::vec3(0.0, 0.0, 1.0));
			monkeyTransforms[7].rotation = glm::rotate(monkeyTransforms[7].rotation, deltaTime, glm::vec3(0.0, 0.0, -1.0));
		}

		lightCamera.position = lightSpecs.direction * shadowSpecs.camDistance;
		lightCamera.orthoHeight = shadowSpecs.camSize;

		cameraController.move(window, &camera, deltaTime);

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
	sh::deleteFramebuffer(framebuffer);
	sh::deleteShadowBuffer(shadowbuffer);
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 5.0f, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	if (ImGui::CollapsingHeader("Blur")) {
		ImGui::SliderFloat("Intensity", &blur.intensity, 1.0f, 10.0f);
	}
	if (ImGui::CollapsingHeader("Light")) {
		ImGui::ColorEdit3("Light Colour", (float*)&lightSpecs.colour);
		ImGui::SliderFloat3("Direction", (float*)&lightSpecs.direction, -1.0f, 1.0f);
	}
	if (ImGui::CollapsingHeader("Shadow")) {
		ImGui::DragFloat("Shadow Cam Distance", &shadowSpecs.camDistance, 0.05f, 5.0f, 50);
		ImGui::DragFloat("Shadow Cam Size", &shadowSpecs.camSize, 0.025f, 5.0f, 20.0f);
		ImGui::DragFloat("Shadow Min Bias", &shadowSpecs.minBias, 0.000025f, 0.001f, 0.05f);
		ImGui::DragFloat("Shadow Max Bias", &shadowSpecs.maxBias, 0.000025f, 0.015f, 0.1f);
	}
	//Add more camera settings here!

	ImGui::End();

	ImGui::Begin("Shadow Map");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("Shadow Map");
	//Stretch image to be window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)shadowbuffer.shadowMap, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

