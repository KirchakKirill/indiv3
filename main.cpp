#include "Headers.h"
#include "Model.h"


//переменные для инициализации позиции света и других параметров
glm::vec3 dirlighting(22.0f, -0.5f, -0.8f);


//переменные для инициализации камеры
glm::vec3 positioncam = glm::vec3(0.1f, 38.2f, 55.7f);
glm::vec3 upcam = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 frcam = glm::vec3(0.0f, -0.5f, -0.8f);

//задание перспективы
glm::mat4 ourpersp = glm::perspective(glm::radians(45.0f), 900.0f / 900.0f, 0.1f, 100.0f);

//позиция дирижабля
glm::vec3 positionairStat(0.0f, 10.0f, 0.0f);



void Draw(sf::Clock clock, vector<Model> modelka, GLint shader, int count)
{
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 model = glm::mat4(1.0f);

	glUseProgram(shader); // Устанавливаем шейдерную программу текущей

	view = glm::lookAt(positioncam, positioncam + frcam, upcam);

	//позиция камеры и перспективгая проекция
	ourpersp = glm::perspective(glm::radians(45.0f), 900.0f / 900.0f, 0.1f, 100.0f);
	
	glUniform3f(glGetUniformLocation(shader, "airshipPos"), positionairStat.x, positionairStat.y, positionairStat.z);
	
	glUniform3f(glGetUniformLocation(shader, "viewPos"), positioncam.x, positioncam.y, positioncam.z);

	glUniform3f(glGetUniformLocation(shader, "light.ambient"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(shader, "light.diffuse"), 0.9f, 0.9f, 0.9);
	glUniform3f(glGetUniformLocation(shader, "light.specular"), 1.0f, 1.0f, 1.0f);
	
	glUniform1i(glGetUniformLocation(shader, "shininess"), 16);

	//направление для света
	glUniform3f(glGetUniformLocation(shader, "light.direction"), dirlighting.x, dirlighting.y, dirlighting.z);
	
	glUniform1f(glGetUniformLocation(shader, "light.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shader, "light.linear"), 0.045f);
	glUniform1f(glGetUniformLocation(shader, "light.quadratic"), 0.0075f);

	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(ourpersp));
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_snow);
	glUniform1i(glGetUniformLocation(shader, "texture_diffuse1"), 0);
	glBindVertexArray(VAO_plane);
	

	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4(1.0f);

	//отрисовка модели дирижабля с наложением alien текстуры
	model = glm::translate(model, glm::vec3(positionairStat.x, positionairStat.y, positionairStat.z));
	//повороты самой модельки
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.5f, 0.0f));
	//задание скейла, определенного рамзера чтобы пропорции смотрелись адекватно
	model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	//затем дальнейшая отрисовка модельки
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

	sf::Texture texture_tree;

	texture_tree.loadFromFile("RGB_085e60619ef44d9d9787473d1ae094fa_4k_Tree_BaseColor.png");
	GLuint texture_tree_id = texture_tree.getNativeHandle();

	texture_tree.bind(&texture_tree, sf::Texture::Normalized); // Привязываем новую текстуру к контексту OpenGL
	glBindTexture(GL_TEXTURE_2D, texture_tree_id); // Привязываем новую текстуру к модели



	modelka[0].Draw(shader, count);
	

	//повтор команд для следующей модельки(модели елки), текстура для нее в 4к наложена из png файла
	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(5.0f, 15.0f, .0f));

	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	model = glm::scale(model, glm::vec3(3.015f, 3.015f, 3.015f));
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//отрисовка модели
	modelka[1].Draw(shader, count);

	
	

	glUseProgram(0); // Отключаем шейдерную программу
	checkOpenGLerror();
}





int main()
{
	std::setlocale(LC_ALL, "Russian");

	sf::Window window(sf::VideoMode(700, 700), "Indiv3Min", sf::Style::Default, sf::ContextSettings(24));
	window.setVerticalSyncEnabled(true);
	window.setActive(true);

	glewInit();
	glGetError(); // сброс флага GL_INVALID_ENUM

	sf::Clock clock;
	vector<Model> modelkins;

	Init();
	PlatoInitialization();

	Model models("flyship/model.obj");
	modelkins.push_back(models);

	Model modelf("flyship/objZeppelin.obj");
	modelkins.push_back(modelf);

	//цикл отрисоки для окна
	while (window.isOpen())
	{
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLint shader = FongLight;
		//используем Фонга
		shader = FongLight;
		
		Draw(clock, modelkins, shader, 1);

		window.display();
	}

	Release();
	return 0;
}