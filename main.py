import pygame
from pygame.locals import *

from OpenGL.GL import *
from OpenGL.GLU import *
from PIL import Image
from numpy import array

def loadTexture(texture, tileid):
	try:
		if tileid == "-1":
			text = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
		else:
			text = Image.open(texture).convert("RGBA")
			text = text.crop((32*(tileid%8), 32*(tileid//8), 32*(tileid%8 + 1), 32*(tileid//8 + 1)))
	except IOError as ex:
		print("Failed to open texture file: ", texture)
		return

	textData = array(list(text.get_flattened_data()))
	textID = glGenTextures(1)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
	glBindTexture(GL_TEXTURE_2D, textID)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, text.size[0], text.size[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, textData)
	text.close()
	return textID

def drawImg(x, y, w, h, textureID):
	glEnable(GL_TEXTURE_2D)
	glBindTexture(GL_TEXTURE_2D, textureID)

	glBegin(GL_QUADS)
	glTexCoord2f(0, 0); glVertex2f(x, y)
	glTexCoord2f(1, 0); glVertex2f(x + w, y)
	glTexCoord2f(1, 1); glVertex2f(x + w, y + h)
	glTexCoord2f(0, 1); glVertex2f(x, y + h)
	glEnd()

	glDisable(GL_TEXTURE_2D)

tilewidth, tileheight = 384, 192
getscrxy = lambda x, y: (tilewidth/2 * (x-y), tileheight/2 * (x+y))
from maptest import floormap, wallmap

"""
this is what's in maptest.py
floormap = [
	[-1, -1, -1, -1, -1, -1],
	[-1, 0, 0, 0, 0, 0, 0],
	[-1, 0, 0, 0, 0, 0, 0],
	[-1, 0, 0, 0, 0, 0, 0],
	[-1, 0, 0, 0, 0, 0, 0],
	[-1, 0, 0, 0, 0, 0, 0],
	[-1, 0, 0, 0, 0, 0, 0],
]
wallmap = [
	[-1, -1, -1, -1, -1, -1, -1, -1, -1],
	[-1, -1, 3, 3, 3, 3, 3, 3, -1],
	[-1, 2, -1, -1, -1, -1, -1, 2, -1],
	[-1, 2, -1, -1, -1, -1, -1, 2, -1],
	[-1, 2, -1, -1, -1, -1, -1, 2, -1],
	[-1, 2, -1, -1, -1, -1, -1, 2, -1],
	[-1, 2, -1, -1, -1, -1, -1, 2, -1],
	[-1, 2, 3, 3, 3, 3, 3, 4, -1],
	[-1, -1, -1, -1, -1, -1, -1, -1],
]
"""

if __name__ == "__main__":
	pygame.init()
	screen = pygame.display.set_mode((600, 800), RESIZABLE|DOUBLEBUF|OPENGL)
	glClearColor(0.2, 0.2, 0.2, 1.0)	
	glEnable(GL_BLEND)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
	glDisable(GL_DEPTH_TEST)
	glDisable(GL_SCISSOR_TEST)
	glDisable(GL_CULL_FACE)
	px, py = 0, 0
	mvmnt = [pygame.K_e, pygame.K_d, pygame.K_s, pygame.K_f]

	textID = {}
	
	running = True
	width, height = screen.get_size()
	while running:
		glViewport(0, 0, width, height)

		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		glOrtho(0, width, height, 0, -1, 1)

		glMatrixMode(GL_MODELVIEW)
		glLoadIdentity()
		
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				running = False
			elif event.type == pygame.VIDEORESIZE:
				width, height = event.w, event.h
		
		speed = 8
		keys = pygame.key.get_pressed()
		if keys[mvmnt[0]]:
			py += speed
		if keys[mvmnt[1]]:
			py -= speed
		if keys[mvmnt[2]]:
			px -= speed
		if keys[mvmnt[3]]:
			px += speed

		glClear(GL_COLOR_BUFFER_BIT)
	
		for i in range(len(floormap)):
			for j in range(len(floormap[i])):
				if floormap[i][j] not in textID:
					textID[floormap[i][j]] = loadTexture("assets/tiles.png", floormap[i][j])
				x, y = getscrxy(j, i)
				drawImg(x-px+(width/2),y+py, 384, 384, textID[floormap[i][j]])
		for i in range(len(wallmap)):
			for j in range(len(wallmap[i])):
				if wallmap[i][j] not in textID:
					textID[wallmap[i][j]] = loadTexture("assets/tiles.png", wallmap[i][j])
				x, y = getscrxy(j, i)
				drawImg(x-px+(width/2),y+py-48, 384, 384, textID[wallmap[i][j]])
		
		pygame.display.flip()
		pygame.time.wait(10)

	pygame.quit()
