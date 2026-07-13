#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

const int tilewidth = 384;
const int tileheight = 192;

int width;
int height;

int px = 0;
int py = 0;

int speed = 8;

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
	std::vector<std::vector<std::string>> output;
	while (std::getline(readfile, line)) {
		std::vector<int> tmp;
		for (const auto& val : split(line, ' ')) {
			tmp.push_back(std::stoi(val));
		}
		output.push_back(tmp);
	}
	return output;
}

unsigned int loadTexture(const char* texture, int tileid) {
	unsigned int imgwidth, imgheight, channels;
	if (tileid == -1) {
		unsigned char* textTMP = stbi_load("empty.png", &imgwidth, &imgheight, &channels, 4);
		unsigned char* text = textTMP;
	} else {
		unsigned char* textTMP = stbi_load(texture, &imgwidth, &imgheight, &channels, 4);
		unsigned char tile[32 * 32 * 4];
		int tx = tileid % 8;
		int ty = tileid / 8;

		for (int y = 0; y < 32; y++) {
			memcpy(
				tile + y * 32 * 4,
				data + ((ty * 32 + y) * width + tx * 32) * 4,
				32 * 4
			);
		}
		unsigned char text* = &tile;
		imgwidth = 32;
		imgheight = 32;
	}

	unsigned int textID = glGenTextures(1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, textID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgwidth, imgheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, text);
	stbi_image_free(textTMP);
	return textID;
}



int main(int argc, char* argv[]) {
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	*GLFWwindow screen = glfwCreateWindow(800, 600, "Blade of Liberation", NULL, NULL);
	glfwMakeContextCurrent(screen);
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_CULL_FACE);

	int mvmnt[4] = [GLFW_KEY_E, GLFW_KEY_D, GLFW_KEY_S, GLFW_KEY_F];

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

		
	}
}

