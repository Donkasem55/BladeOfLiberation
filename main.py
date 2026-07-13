import glfw

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

if __name__ == "__main__":
	glfw.init()
	glfw.window_hint(glfw.RESIZABLE, glfw.TRUE)
	
	screen = glfw.create_window(800, 600, "Blade of Liberation", None, None)
	if not screen:
		glfw.terminate()
		raise Exception("GLFW window cannot be created")
	glfw.make_context_current(screen)
	glClearColor(0.2, 0.2, 0.2, 1.0)	
	glEnable(GL_BLEND)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
	glDisable(GL_DEPTH_TEST)
	glDisable(GL_SCISSOR_TEST)
	glDisable(GL_CULL_FACE)
	px, py = 0, 0
	mvmnt = [glfw.KEY_E, glfw.KEY_D, glfw.KEY_S, glfw.KEY_F]

	textID = {}
	
	while not glfw.window_should_close(screen):
		width, height = glfw.get_window_size(screen)
		glViewport(0, 0, width, height)

		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		glOrtho(0, width, height, 0, -1, 1)

		glMatrixMode(GL_MODELVIEW)
		glLoadIdentity()
		
		speed = 8
		if glfw.get_key(screen, mvmnt[0]) == glfw.PRESS:
			py += speed
		if glfw.get_key(screen, mvmnt[1]) == glfw.PRESS:
			py -= speed
		if glfw.get_key(screen, mvmnt[2]) == glfw.PRESS:
			px -= speed
		if glfw.get_key(screen, mvmnt[3]) == glfw.PRESS:
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
		
		glfw.swap_buffers(screen)
		glfw.poll_events()

	glfw.terminate()
