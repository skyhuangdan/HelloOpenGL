#include "CUseShaderFile.h"
#include "CShaderFromFile.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"



void CUseShaderFile::framebuffer_size_callback(GLFWwindow* wnd, int width, int height)
{
	glViewport(0, 0, width, height);//调整视窗区域
}

void CUseShaderFile::processInput(GLFWwindow* wnd)
{
	if (glfwGetKey(wnd, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(wnd, true);
	else if (glfwGetKey(wnd, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		m_cur_mix -= 0.1f;
		m_shader->setFloat("va_mix", m_cur_mix);
		std::cout << m_cur_mix << std::endl;

	}
	else if (glfwGetKey(wnd, GLFW_KEY_UP) == GLFW_PRESS)
	{
		m_cur_mix += 0.1f;
		m_shader->setFloat("va_mix", m_cur_mix);
		std::cout << m_cur_mix << std::endl;

	}
}

void CUseShaderFile::exit()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);


	glfwTerminate();
}

CUseShaderFile::CUseShaderFile() 
	: m_wnd(NULL)
	, m_cur_mix(0.2)
{
	init();

}

CUseShaderFile::~CUseShaderFile()
{

}

unsigned int CUseShaderFile::loadImage(const char* path, bool bTransparency)
{
	if (!path)
	{
		std::cout << "CUseShaderFile::loadImage: image path ERR!" << std::endl;
		return 0;
	}
	int width, height, nrChannels;//最后为颜色通道的个数
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cout << "CUseShaderFile::loadImage: FAILED to load texture:" << path << std::endl;
		return 0;
	}
	unsigned int texture;
	//参数一 为生成纹理的数量
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//为当前绑定的纹理对象设置环绕,过滤方式
	/*
	环绕方式	描述
	GL_REPEAT	对纹理的默认行为。重复纹理图像。
	GL_MIRRORED_REPEAT	和GL_REPEAT一样，但每次重复图片是镜像放置的。
	GL_CLAMP_TO_EDGE	纹理坐标会被约束在0到1之间，超出的部分会重复纹理坐标的边缘，产生一种边缘被拉伸的效果。
	GL_CLAMP_TO_BORDER	超出的坐标为用户指定的边缘颜色。
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	/*
	过滤方式	描述
	GL_NEAREST_MIPMAP_NEAREST	使用最邻近的多级渐远纹理来匹配像素大小，并使用邻近插值进行纹理采样
	GL_LINEAR_MIPMAP_NEAREST	使用最邻近的多级渐远纹理级别，并使用线性插值进行采样
	GL_NEAREST_MIPMAP_LINEAR	在两个最匹配像素大小的多级渐远纹理之间进行线性插值，使用邻近插值进行采样
	GL_LINEAR_MIPMAP_LINEAR		在两个邻近的多级渐远纹理之间使用线性插值，并使用线性插值进行采样
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//把前面载入的图片数据生成一个纹理
	glTexImage2D(
		GL_TEXTURE_2D, //纹理目标
		0, //多级渐远纹理的级别，0为基础级别
		bTransparency ? GL_RGBA : GL_RGB, //期望的纹理存储格式
		width, //纹理高宽
		height, 
		0, //只能为0 OpenGL辣鸡代码
		bTransparency ? GL_RGBA : GL_RGB, //原图的格式和数据类型
		GL_UNSIGNED_BYTE, 
		data);//参数数据
	glGenerateMipmap(GL_TEXTURE_2D);

	//释放纹理
	stbi_image_free(data);
	return texture;
}

int CUseShaderFile::init()
{
	std::cout << "Hello OpenGL!" << std::endl;
	//初始化
	glfwInit();
	//OpenGL主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//OpenGL副版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//OpenGL我们使用核心模式
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//创建窗口
	m_wnd = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello OpenGL", NULL, NULL);
	if (!m_wnd)
	{
		std::cout << "创建失败" << std::endl;
		glfwTerminate();
		return -1;
	}
	//通知GLFW将我们窗口的上下文设置为当前线程的主上下文
	glfwMakeContextCurrent(m_wnd);
	//初始化glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "初始化glad失败" << std::endl;
		return -1;
	}
	//注册窗口回调
	glfwSetFramebufferSizeCallback(m_wnd, framebuffer_size_callback);

	//生成唯一id
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	std::cout << "wnd init finish." << std::endl;
	return 0;
}
int CUseShaderFile::texture()
{
	//纹理应用点
	float tex_vertices[] = {
		//     ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
			 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // 右上
			 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // 右下
			-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // 左下
			-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // 左上		
	};

	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};


	glBufferData(GL_ARRAY_BUFFER, sizeof(tex_vertices), tex_vertices, GL_STATIC_DRAW);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//由于stb_image库的y起始在底,而一般照片的y起始在顶,所以我们需要修正
	stbi_set_flip_vertically_on_load(true);

	m_texture = loadImage("../path/wall.jpg");
	m_texture2 = loadImage("../path/face.png", true);





	return 0;

}
int CUseShaderFile::triangle()
{
	float vertices[] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.0f, 0.5f, 0.0f
	};


	//参数 1,顶点数据需要拷贝到的地方(VBO) 2,数组大小 3,数组地址 4,告诉显卡采用什么方式管理数据 GL_STATIC_DRAW为无须改变数据
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//参数 1, 对应的顶点属性编号(location=0) 2,顶点属性的大小 
	//3,数据类型 4,指明是否需要规范化(暂时不知道是什么东西) 
	//5,属性跨度 即数组中每三个为一组
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	return 0;
}

void CUseShaderFile::loop()
{
	m_shader = new CShaderFromFile("../path/Shader.vs", "../path/Shader.fs");


	//循环渲染
	while (!glfwWindowShouldClose(m_wnd))
	{
		processInput(m_wnd);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);//

		m_shader->use();
		double timeValue = glfwGetTime();
		double Value = (sin(timeValue) / 2.0f);
		m_shader->setFloat("xoffset", (float)Value);

		//glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

		glfwSwapBuffers(m_wnd);
		glfwPollEvents();
	}
	exit();
}

void CUseShaderFile::loop_texture()
{
	m_shader = new CShaderFromFile("../path/mat4_shader.vs", "../path/tex_shader.fs");
	
	m_shader->use();
	m_shader->setInt("ourTexture1", 0);
	m_shader->setInt("ourTexture2", 1);

	m_shader->setFloat("va_mix", m_cur_mix);

	//循环渲染
	while (!glfwWindowShouldClose(m_wnd))
	{
		processInput(m_wnd);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);//

		//glBindTexture(GL_TEXTURE_2D, m_texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_texture2);


		//UseMat4(m_shader);
		UseMat4_move(m_shader);
		m_shader->use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(m_wnd);
		glfwPollEvents();
	}

	exit();

}


void CUseShaderFile::UseMat4(const CShaderFromFile* shader)
{
	if (!shader)
		return;
	
	glm::mat4 trans;

	//让贴图旋转90° (glm::radians为将角度转换为弧度)
	trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
	//让贴图变成原来的一半大
	trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
	
	unsigned int transformLoc = glGetUniformLocation(shader->GetID(), "transform");
	//参数:
	//1.就是uniform的位置值
	//2.告诉OpenGL我们将发送多少个矩阵,这里为1个
	//3.为是否希望对我们的矩阵进行置换(是否需要交换矩阵行和列),OpenGL内部矩阵布局叫 列主序.glm也相同.所以保持
	//4.真正的矩阵参数.
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));



}

void CUseShaderFile::UseMat4_move(const CShaderFromFile * shader)
{
	if (!shader)
		return;

	glm::mat4 trans;
	//同理这里是单位阵然后加入位移(0.5, -0.5, 0)
	trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
	//让图形随着时间进程而旋转
	//(这里gettime函数返回的是当前运行的秒数,这里是怎么给换算成旋转所需的弧度的?并且在表现中是循环的
	trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

	unsigned int transformLoc = glGetUniformLocation(shader->GetID(), "transform");
	//参数:
	//1.就是uniform的位置值
	//2.告诉OpenGL我们将发送多少个矩阵,这里为1个
	//3.为是否希望对我们的矩阵进行置换(是否需要交换矩阵行和列),OpenGL内部矩阵布局叫 列主序.glm也相同.所以保持
	//4.真正的矩阵参数.
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));


}
