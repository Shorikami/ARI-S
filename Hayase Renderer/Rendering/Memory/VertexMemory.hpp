#ifndef VERTEXMEMORY_HPP
#define VERTEXMEMORY_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <map>

namespace Hayase
{
	class VertexBuffer
	{
	public:
		GLuint id;
		GLenum type;

		// Empty vertex buffer
		VertexBuffer()
		{
		}

		// Vertex buffer with specified type
		// type - Type of buffer (Element, regular)
		VertexBuffer(GLenum type)
			: type(type)
		{
		}

		// Generate buffer
		void Generate()
		{
			glGenBuffers(1, &id);
		}

		// Delete buffer
		void Cleanup()
		{
			glDeleteBuffers(1, &id);
		}

		// Bind buffer to use
		void Bind()
		{
			glBindBuffer(type, id);
		}

		// Unbind current buffer
		void Unbind()
		{
			glBindBuffer(type, 0);
		}

		// Set the data of the currently bound buffer
		// elements - Number of elements in the data to set
		// data - The data to upload
		// usage - How the data is drawn (Usually STATIC_DRAW)
		template<typename T>
		void SetData(GLuint elements, T* data, GLenum usage)
		{
			glBufferData(type, elements * sizeof(T), data, usage);
		}

		// Update the data of the currently bound buffer
		// offset - The offset in the data to begin at (in bytes)
		// elements - Number of elements in the data to set
		// data - The data to update with
		template<typename T>
		void UpdateData(GLintptr offset, GLuint elements, T* data)
		{
			glBufferSubData(type, offset, elements * sizeof(T), data);
		}

		// Specify an attribute pointer in the currently bound buffer
		// idx - The index of the attribute
		// size - The size of the attribute
		// type - The primitive type for this attribute
		// stride - The stride of the pointer
		// offset - The offset of the pointer
		// divisor - The specification of where to divide this pointer. This is
		// typically used when uploading matrices instead of vectors
		template<typename T>
		void SetAttPointer(GLuint idx, GLint size, GLenum type, GLuint stride, GLuint offset, GLuint divisor = 0)
		{
			glVertexAttribPointer(idx, size, type, GL_FALSE, stride * sizeof(T), (void*)(offset * sizeof(T)));
			glEnableVertexAttribArray(idx);

			if (divisor > 0)
			{
				glVertexAttribDivisor(idx, divisor);
			}
		}
	};

	class VertexArray
	{
	public:
		GLuint id;
		std::map<const char*, VertexBuffer> buffers;

		// Operator overload for locating a specific buffer
		// in a vertex array
		VertexBuffer& operator[](const char* key)
		{
			return buffers[key];
		}

		VertexBuffer& operator[](std::string key)
		{
			return buffers[key.c_str()];
		}


		// Generate the vertex array
		void Generate()
		{
			glGenVertexArrays(1, &id);
		}

		// Bind the vertex array
		void Bind()
		{
			glBindVertexArray(id);
		}

		// Draw the elements of a vertex array
		// mode - Specify how to draw the elements (LINES or TRIANGLES)
		// count - Number of indices in the index buffer of this vertex array
		// type - Please use GL_UNSIGNED_INT. That should be the primitive type for the indices
		void Draw(GLenum mode, GLuint count, GLenum type)
		{
			glDrawElements(mode, count, type, nullptr);
		}

		// Draw the elements of a vertex array, but instanced
		// THIS GOES UNUSED FOR NOW (This exists because of importing from my other renderer)
		void Draw(GLenum mode, GLuint count, GLenum type, GLint indices, GLuint instanceCount = 1)
		{
			glDrawElementsInstanced(mode, count, type, (void*)indices, instanceCount);
		}

		// Clean up the vertex array
		void Cleanup()
		{
			glDeleteVertexArrays(1, &id);
			for (auto& a : buffers)
			{
				a.second.Cleanup();
			}
		}

		// Unbind the current vertex array
		void Clear()
		{
			glBindVertexArray(0);
		}

	private:

	};
}

#endif