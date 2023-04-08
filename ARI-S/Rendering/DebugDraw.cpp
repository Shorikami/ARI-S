#include <arpch.h>

#define DEBUG_DRAW_IMPLEMENTATION
#include "DebugDraw.h"

namespace ARIS
{
	void DebugWrapper::Initialize()
	{
		dd::initialize(&m_Interface);
	}

	void DebugWrapper::Destroy()
	{
		dd::shutdown();
	}

	void DebugWrapper::Render()
	{
		dd::flush(0);
	}

	void DebugWrapper::Update(EditorCamera& c)
	{
		m_Interface.m_MatrixMVP = c.GetViewProjection();
	}

	const char* DebugInterface::m_LineVertexShader =
		"\n"
		"#version 150\n"
		"\n"
		"in vec3 in_Position;\n"
		"in vec4 in_ColorPointSize;\n"
		"\n"
		"out vec4 v_Color;\n"
		"uniform mat4 u_MvpMatrix;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position  = u_MvpMatrix * vec4(in_Position, 1.0);\n"
		"    gl_PointSize = in_ColorPointSize.w;\n"
		"    v_Color      = vec4(in_ColorPointSize.xyz, 1.0);\n"
		"}\n";

	const char* DebugInterface::m_LineFragmentShader =
		"\n"
		"#version 150\n"
		"\n"
		"in  vec4 v_Color;\n"
		"out vec4 out_FragColor;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    out_FragColor = v_Color;\n"
		"}\n";

	void DebugInterface::drawPointList(const dd::DrawVertex* points, int count, bool depthEnabled)
	{
		glBindVertexArray(m_LineVAO);
		glUseProgram(m_LineProgram);

		glUniformMatrix4fv(m_LineProgramMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_MatrixMVP));

		if (depthEnabled)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), points);

		glDrawArrays(GL_POINTS, 0, count);

		glUseProgram(0);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void DebugInterface::drawLineList(const dd::DrawVertex* lines, int count, bool depthEnabled)
	{
		glBindVertexArray(m_LineVAO);
		glUseProgram(m_LineProgram);

		glUniformMatrix4fv(m_LineProgramMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_MatrixMVP));

		if (depthEnabled)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), lines);

		glDrawArrays(GL_LINES, 0, count);

		glUseProgram(0);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	DebugInterface::DebugInterface()
		: m_MatrixMVP(glm::mat4(1))
		, m_LineProgram(0)
		, m_LineProgramMatrixLoc(-1)
		, m_LineVAO(0)
		, m_LineVBO(0)
	{
		std::printf("Debug initializing ...\n");

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_PROGRAM_POINT_SIZE);

		SetupShaders();
		SetupVertexBuffers();

		std::printf("Debug ready!\n\n");
	}

	DebugInterface::~DebugInterface()
	{
		glDeleteProgram(m_LineProgram);

		glDeleteVertexArrays(1, &m_LineVAO);
		glDeleteBuffers(1, &m_LineVBO);

	}

	void DebugInterface::SetupShaders()
	{
		GLuint linePointVS = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(linePointVS, 1, &m_LineVertexShader, nullptr);
		CompileShader(linePointVS);

		GLint linePointFS = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(linePointFS, 1, &m_LineFragmentShader, nullptr);
		CompileShader(linePointFS);

		m_LineProgram = glCreateProgram();
		glAttachShader(m_LineProgram, linePointVS);
		glAttachShader(m_LineProgram, linePointFS);

		glBindAttribLocation(m_LineProgram, 0, "in_Position");
		glBindAttribLocation(m_LineProgram, 1, "in_ColorPointSize");
		LinkProgram(m_LineProgram);

		m_LineProgramMatrixLoc = glGetUniformLocation(m_LineProgram, "u_MvpMatrix");
		if (m_LineProgramMatrixLoc < 0)
		{
			std::cout << "Debug Draw Shader loaded incorrectly!" << std::endl;
		}
	}

	void DebugInterface::SetupVertexBuffers()
	{
		glGenVertexArrays(1, &m_LineVAO);
		glGenBuffers(1, &m_LineVBO);

		glBindVertexArray(m_LineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);

		// RenderInterface will never be called with a batch larger than
		// DEBUG_DRAW_VERTEX_BUFFER_SIZE vertexes, so we can allocate the same amount here.
		glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), nullptr, GL_STREAM_DRAW);

		// Set the vertex format expected by 3D points and lines:
		std::size_t offset = 0;

		glEnableVertexAttribArray(0); // in_Position (vec3)
		glVertexAttribPointer(
			/* index     = */ 0,
			/* size      = */ 3,
			/* type      = */ GL_FLOAT,
			/* normalize = */ GL_FALSE,
			/* stride    = */ sizeof(dd::DrawVertex),
			/* offset    = */ reinterpret_cast<void*>(offset));
		offset += sizeof(float) * 3;

		glEnableVertexAttribArray(1); // in_ColorPointSize (vec4)
		glVertexAttribPointer(
			/* index     = */ 1,
			/* size      = */ 4,
			/* type      = */ GL_FLOAT,
			/* normalize = */ GL_FALSE,
			/* stride    = */ sizeof(dd::DrawVertex),
			/* offset    = */ reinterpret_cast<void*>(offset));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void DebugInterface::CompileShader(const GLuint s)
	{
		glCompileShader(s);

		GLint status;
		glGetShaderiv(s, GL_COMPILE_STATUS, &status);

		if (status == GL_FALSE)
		{
			GLchar strInfoLog[1024] = { 0 };
			glGetShaderInfoLog(s, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
			std::cout << "DEBUG SHADER COMPILATION ERROR: " << strInfoLog << std::endl;
		}
	}

	void DebugInterface::LinkProgram(const GLuint p)
	{
		glLinkProgram(p);

		GLint status;
		glGetProgramiv(p, GL_LINK_STATUS, &status);

		if (status == GL_FALSE)
		{
			GLchar strInfoLog[1024] = { 0 };
			glGetProgramInfoLog(p, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
			std::cout << "DEBUG SHADER LINKING ERROR: " << strInfoLog << std::endl;
		}
	}
}