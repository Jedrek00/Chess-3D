/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/
//HEEELELEOEOEOEEOEOEOE
#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "iostream"
#include "shaderprogram.h"
#include "myCube.h"
#include "model.h"

using namespace std;



GLuint tex;
GLuint tex2;
vector<glm::vec4> verts;
vector<glm::vec4> norms;
vector<glm::vec2> texCoords;
vector<unsigned int> indices;

float amount;
float speed_x = 0;//[radiany/s]
float speed_y = 0;//[radiany/s]
glm::vec3 move_y = glm::vec3(0.0f, 0.0f, 0.0f);

// // // // // // // // // // // //
glm::vec3 chessboard[8][8];

vector<Model*> white;
vector<Model*> black;
Model* board;

ShaderProgram* shader;

void createChessboard()
{
	glm::vec3 A1 = glm::vec3(16.8f, 0.0f, 16.75f);

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			chessboard[i][j] = A1;
			chessboard[i][j].x -= 4.8f * i;
			chessboard[i][j].z -= 4.8f * j;
		}
	}
	/*for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			cout << chessboard[i][j].x << " " << chessboard[i][j].z << endl;
		}
	}*/
}

void createModels()
{
	for (int i = 0; i < 8; i++)
	{
		white.push_back(new Model("my_models/whitepieces/pawn/Pawn.obj", i, 1));
		black.push_back(new Model("my_models/darkpieces/pawn/Pawn.obj", i, 6));
	}
	white.push_back(new Model("my_models/whitepieces/bishop/Bishop.obj", 2, 0));
	white.push_back(new Model("my_models/whitepieces/bishop/Bishop.obj", 5, 0));
	white.push_back(new Model("my_models/whitepieces/knight/Knight.obj", 1, 0));
	white.push_back(new Model("my_models/whitepieces/knight/Knight.obj", 6, 0));
	white.push_back(new Model("my_models/whitepieces/rook/Rook.obj", 0, 0));
	white.push_back(new Model("my_models/whitepieces/rook/Rook.obj", 7, 0));
	white.push_back(new Model("my_models/whitepieces/queen/Queen.obj", 3, 0));
	white.push_back(new Model("my_models/whitepieces/king/King.obj", 4, 0));

	black.push_back(new Model("my_models/darkpieces/bishop/Bishop.obj", 2, 7));
	black.push_back(new Model("my_models/darkpieces/bishop/Bishop.obj", 5, 7));
	black.push_back(new Model("my_models/darkpieces/knight/Knight.obj", 1, 7));
	black.push_back(new Model("my_models/darkpieces/knight/Knight.obj", 6, 7));
	black.push_back(new Model("my_models/darkpieces/rook/Rook.obj", 0, 7));
	black.push_back(new Model("my_models/darkpieces/rook/Rook.obj", 7, 7));
	black.push_back(new Model("my_models/darkpieces/queen/Queen.obj", 3, 7));
	black.push_back(new Model("my_models/darkpieces/king/King.obj", 4, 7));
}

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glActiveTexture(GL_TEXTURE0);
	return tex;
}

void loadModel(std::string plik)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	cout << importer.GetErrorString() << endl;

	aiMesh* mesh = scene->mMeshes[0];

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D vertex = mesh->mVertices[i];
		verts.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));
		
		aiVector3D normal = mesh->mNormals[i];
		norms.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));

		aiVector3D texCoord = mesh->mTextureCoords[0][i];
		texCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
	}

	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace& face = mesh->mFaces[i];

		for (int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
		//cout << endl;
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	for (int i = 0; i < 19; i++)
	{
		cout << i << " " << material->GetTextureCount((aiTextureType)i) << endl;
	}

	
	for (int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++)
	{
		aiString str;
		material->GetTexture(aiTextureType_DIFFUSE, i, &str);
		cout << str.C_Str() << endl;
	}
}

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void key_callback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mod
) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) {
			speed_y = -PI;
		}
		if (key == GLFW_KEY_RIGHT) {
			speed_y = PI;
		}
		if (key == GLFW_KEY_UP) {
			speed_x = -PI;
		}
		if (key == GLFW_KEY_DOWN) {
			speed_x = PI;
		}
		if (key == GLFW_KEY_W) {
			move_y = move_y + glm::vec3(3.2f, 0.0f, 0.0f);
		}
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
			speed_y = 0;
		}
		if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) {
			speed_x = 0;
		}
		if (key == GLFW_KEY_W) {
			move_y = glm::vec3(0.0f, 0.0f, 0.0f);
		}
	}
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    initShaders();
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0, 0, 0, 1); //Ustaw kolor czyszczenia bufora kolorów
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetKeyCallback(window, key_callback);
	//tex = readTexture("textures\\bd.png");
	//tex2 = readTexture("textures\\cd.png");
	shader = new ShaderProgram("v_constant.glsl", NULL, "f_constant.glsl");
	//rook = new Model("my_models/whitepieces/rook/Rook.obj", 7, 7);
	//pawn2 = new Model("my_models/whitepieces/queen/Queen.obj");
	//pawn3 = new Model("my_models/darkpieces/king/King.obj");
	//pawn4 = new Model("my_models/darkpieces/bishop/Bishop.obj");
	board = new Model("my_models/board/Board.obj");
	createModels();
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    freeShaders();
	glDeleteTextures(1, &tex);
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float angle_x,float angle_y, glm::vec3 distance_y) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości	
	glm::vec3 arr[2] = { glm::vec3(5.0f, 30.0f, -5.0f), glm::vec3(0.0f, -15.0f, -5.0f) };
	glm::vec3 colors[2] = { glm::vec3(0.0f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f) };


	glm::mat4 M = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 40.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz widoku
	glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 1.0f, 50.0f); //Wylicz macierz rzutowania

	shader->use();
	glUniform3fv(shader->u("light_pos"), 2, &arr[0][0]);
	glUniform3f(shader->u("camera_pos"), 0.0f, 25.0f, -5.0f);
	glUniform3fv(shader->u("light_color"), 2, &colors[0][0]);
	glUniform1f(shader->u("Time"), glm::sin(amount / 3) * 0.5 + 0.5);
	glUniformMatrix4fv(shader->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(shader->u("V"), 1, false, glm::value_ptr(V));

	//M = glm::translate(M, glm::vec3(2.4f, 0.0f, 2.35f)); 
	//M = glm::translate(M, glm::vec3(16.8f, 0.0f, 16.75f));//USTAWIENIE NA POLE A1

	for (int i = 0; i < 16; i++)
	{
		M = glm::mat4(1.0f);
		M = glm::rotate(M, angle_y, glm::vec3(0.0f, 1.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi Y
		M = glm::rotate(M, angle_x, glm::vec3(1.0f, 0.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi X
		M = glm::translate(M, distance_y);
		M = glm::translate(M, chessboard[white[i]->row][white[i]->column]);//POLE H8

		glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(M));

		white[i]->Draw(*shader);
	}

	for (int i = 0; i < 16; i++)
	{
		M = glm::mat4(1.0f);
		M = glm::rotate(M, angle_y, glm::vec3(0.0f, 1.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi Y
		M = glm::rotate(M, angle_x, glm::vec3(1.0f, 0.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi X
		M = glm::translate(M, distance_y);
		M = glm::translate(M, chessboard[black[i]->row][black[i]->column]);//POLE H8

		glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(M));

		black[i]->Draw(*shader);
	}

	// Rysowanie planszy

	M = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
	M = glm::rotate(M, angle_y, glm::vec3(0.0f, 1.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi Y
	M = glm::rotate(M, angle_x, glm::vec3(1.0f, 0.0f, 0.0f)); //Pomnóż macierz modelu razy macierz obrotu o kąt angle wokół osi X

	M = glm::scale(M, glm::vec3(80.0f, 80.0f, 80.0f));

	glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(M));

	board->Draw(*shader);

	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
	glDisableVertexAttribArray(shader->a("vertex"));
	glDisableVertexAttribArray(shader->a("normal"));
	glDisableVertexAttribArray(shader->a("texCoord"));
}

int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące
	createChessboard();
	//Główna pętla
	float angle_x = 0; //zadeklaruj zmienną przechowującą aktualny kąt obrotu
	float angle_y = 0; //zadeklaruj zmienną przechowującą aktualny kąt obrotu
	glm::vec3 distance_y = glm::vec3(0.0f, 0.0f, 0.0f);
	glfwSetTime(0); //Wyzeruj licznik czasu
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		amount += glfwGetTime();
		angle_x += speed_x * glfwGetTime(); //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		angle_y += speed_y * glfwGetTime(); //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		distance_y += move_y * float(glfwGetTime());
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window,angle_x,angle_y, distance_y); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
