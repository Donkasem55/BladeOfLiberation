#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

#include <iostream>

const int tilewidth = 384;
const int tileheight = 192;

int width;
int height;

int px = 0;
int py = 0;

int speed = 1;
int tmpx, tmpy;

void getscrxy(int x, int y, int* xout, int* yout) {
	*xout = tilewidth/2 * (x-y);
	*yout = tileheight/2 * (x+y);
}

std::vector<std::string> split(std::string input, char delimiter) {
	std::stringstream ss(input);
	std::string token;
	std::vector<std::string> output;
	while (std::getline(ss, token, delimiter)) {
		output.push_back(token);
	}
	return output;
}

std::vector<std::vector<int>> readmap(const char* filename) {
	std::ifstream readfile(filename);
	std::string line;
	std::vector<std::vector<int>> output;
	while (std::getline(readfile, line)) {
		std::vector<int> tmp;
		for (const auto& val : split(line, ' ')) {
			tmp.push_back(std::stoi(val));
			std::cout << val << " ";
		}
		output.push_back(tmp);
		std::cout << std::endl;
	}
	return output;
}

unsigned int loadTexture(const char* texture, int tileid) {
	std::cout << tileid << std::endl;
	int imgwidth, imgheight, channels;
	unsigned char* textTMP = nullptr;
	unsigned char* text = nullptr;
	std::vector<unsigned char> txt(32 * 32 * 4);
	if (tileid == -1) {
		//textTMP = stbi_load("empty.png", &imgwidth, &imgheight, &channels, 4);
		//text = textTMP;
		tileid = 12; // tile -1 is the legacy id for empty tile
	}
	{
		textTMP = stbi_load(texture, &imgwidth, &imgheight, &channels, 4);
		if (!textTMP) {
			std::cout << stbi_failure_reason() << std::endl;
		}
		int tx = tileid % 8;
		int ty = tileid / 8;

		for (int y = 0; y < 32; y++) {
			memcpy(
				txt.data() + y * 32 * 4,
				textTMP + ((ty * 32 + y) * imgwidth + tx * 32) * 4,
				32 * 4
			);
		}
		text = txt.data();
		
		imgwidth = 32;
		imgheight = 32;
	}

	unsigned int textID;
	glGenTextures(1, &textID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, textID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	//std::cout << text[0];   for some reason it just freezes if you uncomment this line
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgwidth, imgheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, text);
	stbi_image_free(textTMP);
	return textID;
}

void drawImg(int x, int y, int w, int h, unsigned int textureID) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(x, y);
	glTexCoord2f(1, 0);
	glVertex2f(x + w, y);
	glTexCoord2f(1, 1);
	glVertex2f(x + w, y + h);
	glTexCoord2f(0, 1);
	glVertex2f(x, y + h);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

int main(int argc, char* argv[]) {
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* screen = glfwCreateWindow(800, 600, "Blade of Liberation", NULL, NULL);
	glfwMakeContextCurrent(screen);
	gladLoadGL();

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_CULL_FACE);

	int mvmnt[4] = {GLFW_KEY_E, GLFW_KEY_D, GLFW_KEY_S, GLFW_KEY_F};

	std::vector<std::vector<int>> floormap = readmap("floor.txt");
	std::vector<std::vector<int>> wallmap = readmap("wall.txt");
	
	std::map<int, unsigned int> textID;

	while (!glfwWindowShouldClose(screen)) {
		glfwGetWindowSize(screen, &width, &height);
		glViewport(0, 0, width, height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width, height, 0, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if (glfwGetKey(screen, mvmnt[0]) == GLFW_PRESS) {
			py += speed;
		}
		if (glfwGetKey(screen, mvmnt[1]) == GLFW_PRESS) {
			py -= speed;
		}
		if (glfwGetKey(screen, mvmnt[2]) == GLFW_PRESS) {
			px -= speed;
		}
		if (glfwGetKey(screen, mvmnt[3]) == GLFW_PRESS) {
			px += speed;
		}

		glClear(GL_COLOR_BUFFER_BIT);
		for (int i=0; i<floormap.size(); i++) {
			for (int j=0; j<floormap[i].size(); j++) {
				if (textID.find(floormap[i][j]) == textID.end()) {
					textID[floormap[i][j]] = loadTexture("assets/tiles.png", floormap[i][j]);
				}
				getscrxy(j, i, &tmpx, &tmpy);
				drawImg(tmpx-px+(width/2), tmpy+py, 384, 384, textID[floormap[i][j]]);

			}
		}
		for (int i=0; i<wallmap.size(); i++) {
			for (int j=0; j<wallmap[i].size(); j++) {
				if (textID.find(wallmap[i][j]) == textID.end()) {
					textID[wallmap[i][j]] = loadTexture("assets/tiles.png", wallmap[i][j]);
				}
				getscrxy(j, i, &tmpx, &tmpy);
				drawImg(tmpx-px+(width/2), tmpy+py-48, 384, 384, textID[wallmap[i][j]]);
			}
		}
		
		glfwSwapBuffers(screen);
		glfwPollEvents();
	}
	glfwTerminate();
}

