#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL/SOIL.h> 
#include <SFML/OpenGL.hpp>    
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <list>
#include <random>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h> 

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h> 
#include<ctime>

using namespace std;

GLuint FongLight;

//как раз сама текстура нашего поля
GLuint texture_snow;

//VBO и VAO для текстутры нашего поля(травы в снегу)
GLuint VBO_plane;
GLuint VAO_plane;

void checkCompileErrors(GLuint shader, string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			cout << "compilation error: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			cout << "linking error: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << endl;
		}
	}
}

//стандартные функции чека ошибки и лога шейдеров
void checkOpenGLerror()
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::cout << "OpenGL really error, bro " << error << std::endl;
	}
}

void ShaderLog(unsigned int shad)
{
	int information_log_length = 0;
	glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &information_log_length);
	GLint vertex_compiled;
	glGetShaderiv(shad, GL_COMPILE_STATUS, &vertex_compiled);

	if (information_log_length > 1)
	{
		int charsWritten = 0;
		std::vector<char> infoLog(information_log_length);
		glGetShaderInfoLog(shad, information_log_length, &charsWritten, infoLog.data());
		std::cout << "Information: " << infoLog.data() << std::endl;
	}

	if (vertex_compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(shad, 1024, &log_length, message);
		std::cout << "Information: " << message << std::endl;
	}

}


// Добавлено наложение текстуры
//https://ru.wikipedia.org/wiki/%D0%97%D0%B0%D1%82%D0%B5%D0%BD%D0%B5%D0%BD%D0%B8%D0%B5_%D0%BF%D0%BE_%D0%A4%D0%BE%D0%BD%D0%B3%D1%83
//вершинный шейдер для Фонга
const char* FongavertShad = R"(
   #version 330 core
    layout (location = 0) in vec3 coord_pos;//position coordinate in 3d
    layout (location = 1) in vec3 normal_in;//normals input data for vertices
    layout (location = 2) in vec2 tex_coord_in;//texture coord input
    //output data for frag shader
    out vec2 coord_tex;
	out vec3 normal;
	out vec3 frag_pos;
    //uniform attrib
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main() 
    { 
		//correct vertices
        gl_Position = projection * view * model * vec4(coord_pos, 1.0);
        //texture in frag shader
        coord_tex = tex_coord_in;
		//calculate postion in world
		frag_pos = vec3(model * vec4(coord_pos, 1.0));
		//normals
		normal =  mat3(transpose(inverse(model))) * normal_in;
        
    }
    )";


//https://ru.wikipedia.org/wiki/%D0%97%D0%B0%D1%82%D0%B5%D0%BD%D0%B5%D0%BD%D0%B8%D0%B5_%D0%BF%D0%BE_%D0%A4%D0%BE%D0%BD%D0%B3%D1%83
//фрагментный шейдер для Фонга
//псевдокод с википедии с учетом добавления текстуры, в конце сумма как раз нужных 3 параметров
//фоновой (ambient), рассеянной (diffuse) и глянцевых бликов (specular).
//Фоновая компонента — грубое приближение лучей света, рассеянных соседними объектами и затем достигших заданной точки; остальные две компоненты имитируют рассеивание и отражение прямого излучения
const char* FongafragmentShad = R"(
    #version 330 core

    //structure for light and other hars and options
	struct Light {
		vec3 position;
		vec3 direction;
  
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;

	//point
		float constant;
		float linear;
		float quadratic;

	};


	//uniform attrib
	uniform Light light;  

    in vec2 coord_tex;
    in vec3 frag_pos;
    in vec3 normal;

	out vec4 frag_color;

    uniform sampler2D texture_diffuse1;
	uniform vec3 viewPos;
	uniform vec3 airshipPos;
	uniform int shininess;
	uniform int type_light;
	uniform int spot_turn;

    void main()  
    {
		vec3 norm = normalize(normal);
		vec3 lightDir;

		lightDir = normalize(-light.direction);  //direction
		
		vec3 ambient = light.ambient * texture(texture_diffuse1, coord_tex).rgb; 

		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = light.diffuse * (diff * texture(texture_diffuse1, coord_tex).rgb); 

		vec3 viewDir = normalize(viewPos - frag_pos);
		vec3 reflectDir = reflect(-lightDir, norm);  

		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1);
		vec3 specular = light.specular * (spec * texture(texture_diffuse1, coord_tex).rgb); 

		vec3 result = (ambient + diffuse + specular);//summ 3 components

		frag_color = vec4(result, 1.0);//color
    } 
)";

//инициализация шейдеров
void InitShader()
{
	GLuint vShaderFong = glCreateShader(GL_VERTEX_SHADER);
	// Передаем исходный код
	glShaderSource(vShaderFong, 1, &FongavertShad, NULL);
	// Компилируем шейдер
	glCompileShader(vShaderFong);
	std::cout << "vertex shader f\n";
	// Функция печати лога шейдера
	ShaderLog(vShaderFong);

	//-----------------------

	// Создаем фрагментный шейдер
	GLuint fShaderFong = glCreateShader(GL_FRAGMENT_SHADER);
	// Передаем исходный код
	glShaderSource(fShaderFong, 1, &FongafragmentShad, NULL);
	// Компилируем шейдер
	glCompileShader(fShaderFong);
	std::cout << "fragment shader f\n";
	// Функция печати лога шейдера
	ShaderLog(fShaderFong);
	// Проверяем статус сборки
	int link_ok;
	FongLight = glCreateProgram();
	glAttachShader(FongLight, vShaderFong);
	glAttachShader(FongLight, fShaderFong);

	// Линкуем шейдерную программу
	glLinkProgram(FongLight);

	// Проверяем статус сборки
	glGetProgramiv(FongLight, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		std::cout << "error attach shaders \n";
		return;
	}

	checkOpenGLerror();
}


void Init()
{
	// Шейдеры
	InitShader();

	//включаем тест глубины
	glEnable(GL_DEPTH_TEST);
}

// Освобождение буфера
void ReleaseVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Освобождение шейдеров
void ReleaseShader()
{
	// Передавая ноль, мы отключаем шейдерную программу
	glUseProgram(0);
	// Удаляем шейдерные программы
	glDeleteProgram(FongLight);
}

void Release()
{
	// Шейдеры
	ReleaseShader();
	// Вершинный буфер
	ReleaseVBO();
}

//инициализация нашего поля и текстуры для него
void PlatoInitialization()
{
	float Plato[] = {
		 30.0f, -1.5f,  30.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		-30.0f, -1.5f,  30.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-30.0f, -1.5f, -30.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,

		 30.0f, -1.5f,  30.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		-30.0f, -1.5f, -30.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,
		 30.0f, -1.5f, -30.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f
	};

	//объявляем массив атрибутов и буфер

	glGenVertexArrays(1, &VAO_plane);
	glGenBuffers(1, &VBO_plane);
	glBindVertexArray(VAO_plane);
	// передаем вершины в буфер
	glBindBuffer(GL_ARRAY_BUFFER, VBO_plane);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Plato), Plato, GL_STATIC_DRAW);

	// Подключаем массив аттрибутов с указанием на каких местах кто находится
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	glBindVertexArray(0);

	// создаем текстуру
	glGenTextures(1, &texture_snow);

	// связываем с типом текступы
	glBindTexture(GL_TEXTURE_2D, texture_snow);

	// настроки отображения текстуры при выходе за пределы диапазона текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// настройки отображения текстуры в зависимости от удаления или приближения обьекта
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// грузим картинку
	int width, height;
	unsigned char* image = SOIL_load_image("zima.png", &width, &height, 0, SOIL_LOAD_RGB);

	//создаем текстуру
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	//отключаем привязку к текстуре
	glBindTexture(GL_TEXTURE_2D, 0);

}