#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <map>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <iostream>

const int tilewidth = 384;
const int tileheight = 192;

int width;
int height;

int px = 0;
int py = 0;

int speed = 8;
int tmpx, tmpy;

const float tfps = 120.0f;
const float tfd = 1.0f / tfps;
const char* lf = "levels/test/floor.txt";
const char* lw = "levels/test/wall.txt";

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

unsigned int loadTexture(const char* texture) {
	int imgwidth, imgheight, channels;
	// std::vector<unsigned char> txt(32 * 32 * 4);
	unsigned char* text = stbi_load(texture, &imgwidth, &imgheight, &channels, 4);
	/*if (tileid == -1) {
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
	}*/

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
	stbi_image_free(text);
	return textID;
}

void drawImg(int x, int y, int w, int h, unsigned int textureID, int tileID, float ssh) {
	if (tileID == -1) {
		tileID = 12;
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);

	int tx = tileID % 8;
	int ty = tileID / 8;

	float u1 = (32*tx) / 256.0f;
	float v1 = ((32*ty) / ssh);

	float u2 = ((32*tx) + 32.0f) / 256.0f; // you see, every single sprite sheet is 256 pixels wide and I would most likely cease to exist if any needed to be a different width.
	float v2 = ((32*ty + 32.0f) / ssh);

	//std::cout << tx << " " << ty << " ";

	glBegin(GL_QUADS);
	//glTexCoord2f(0, 0);
	glTexCoord2f(u1, v1);
	glVertex2f(x, y);
	//glTexCoord2f(1, 0);
	glTexCoord2f(u2, v1);
	glVertex2f(x + w, y);
	//glTexCoord2f(1, 1);
	glTexCoord2f(u2, v2);
	glVertex2f(x + w, y + h);
	//glTexCoord2f(0, 1);
	glTexCoord2f(u1, v2);
	glVertex2f(x, y + h);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

// we live in a cruel world

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
	unsigned int state = 0;

	std::vector<std::vector<int>> floormap = readmap(lf);
	std::vector<std::vector<int>> wallmap = readmap(lw);
	
	unsigned int textID = loadTexture("assets/tiles.png");
	unsigned int playerID = loadTexture("assets/raine.png");

	while (!glfwWindowShouldClose(screen)) {
		float fst = glfwGetTime();

		glfwGetWindowSize(screen, &width, &height);
		glViewport(0, 0, width, height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width, height, 0, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if (glfwGetKey(screen, mvmnt[0]) == GLFW_PRESS) {
			py += speed;
			state = 1;
		}
		if (glfwGetKey(screen, mvmnt[1]) == GLFW_PRESS) {
			py -= speed;
			state = 0;
		}
		if (glfwGetKey(screen, mvmnt[2]) == GLFW_PRESS) {
			px -= speed;
			state = 3;
		}
		if (glfwGetKey(screen, mvmnt[3]) == GLFW_PRESS) {
			px += speed;
			state = 2;
		}

		glClear(GL_COLOR_BUFFER_BIT);
		int ssr = 2; /*the number of rows in the tile spritesheet. change if needed.*/
		for (int i=0; i<floormap.size(); i++) {
			for (int j=0; j<floormap[i].size(); j++) {
				getscrxy(j, i, &tmpx, &tmpy);
				drawImg(tmpx-px+(width/2-96), tmpy+py+(height/2-96), 384, 384, textID, floormap[i][j], (float)(32*ssr));
			}
		}
		for (int i=0; i<wallmap.size(); i++) {
			for (int j=0; j<wallmap[i].size(); j++) {
				getscrxy(j, i, &tmpx, &tmpy);
				drawImg(tmpx-px+(width/2-96), tmpy+py-48+(height/2-96), 384, 384, textID, wallmap[i][j], (float)(32*ssr));
			}
		}

		int pssr = 1;
		drawImg(width/2-96, height/2-96, 192, 192, playerID, state, (float)(32*pssr));
		
		glfwSwapBuffers(screen);
		glfwPollEvents();

		float fet = glfwGetTime();
		float fdu = fet - fst;
		if (fdu < tfd) {
			std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>((tfd - fdu) * 1000)));
		}
	}
	glfwTerminate();
}

