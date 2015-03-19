#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

#include <GL\glew.h>

class Shader
{
public:
	GLuint program;

	Shader(const GLchar* vertSourcePath, const GLchar* fragSourcePath)
	{
		string vertCode;
		string fragCode;
		try
		{
			ifstream vShaderFile(vertSourcePath);
			ifstream fShaderFile(fragSourcePath);
			stringstream vShaderStream, fShaderStream;

			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			vShaderFile.close();
			fShaderFile.close();

			vertCode = vShaderStream.str();
			fragCode = fShaderStream.str();
		}
		catch (exception e)
		{
			cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
		}
		const GLchar* vShaderCode = vertCode.c_str();
		const GLchar* fShaderCode = fragCode.c_str();

		GLuint vert, frag;
		GLint success;
		GLchar infoLog[512];

		vert = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert, 1, &vShaderCode, NULL);
		glCompileShader(vert);

		glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vert, 512, NULL, infoLog);
			cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
		}

		frag = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag, 1, &fShaderCode, NULL);
		glCompileShader(frag);

		glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(frag, 512, NULL, infoLog);
			cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
		}

		this->program = glCreateProgram();
		glAttachShader(this->program, vert);
		glAttachShader(this->program, frag);
		glLinkProgram(this->program);

		glGetProgramiv(this->program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->program, 512, NULL, infoLog);
			cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
		}

		glDeleteShader(vert);
		glDeleteShader(frag);
	}
	void use() { glUseProgram(this->program); }
};

#endif