/**
 * @file
 */

#pragma once

#include <string>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "Renderer.h"
#include "core/GLM.h"
#include "core/Log.h"
#include "Types.h"

namespace video {

class UniformBuffer;

#ifndef VERTEX_POSTFIX
#define VERTEX_POSTFIX ".vert"
#endif

#ifndef FRAGMENT_POSTFIX
#define FRAGMENT_POSTFIX ".frag"
#endif

#ifndef GEOMETRY_POSTFIX
#define GEOMETRY_POSTFIX ".geom"
#endif

// activate this to validate that every uniform was set
#define VALIDATE_UNIFORMS 0

class Shader {
protected:
	typedef std::unordered_map<ShaderType, Id, EnumClassHash> ShaderMap;
	ShaderMap _shader;

	Id _program = InvalidId;
	bool _initialized = false;;
	mutable bool _active = false;

	typedef std::map<std::string, std::string> ShaderDefines;
	ShaderDefines _defines;

	typedef std::unordered_map<std::string, int> ShaderUniformArraySizes;
	ShaderUniformArraySizes _uniformArraySizes;

	ShaderUniforms _uniforms;

	// can be used to validate that every uniform was set. The value type is the location index
	mutable std::unordered_set<int> _usedUniforms;
	bool _recordUsedUniforms = false;
	void addUsedUniform(int location) const;

	ShaderAttributes _attributes;

	typedef std::unordered_map<int, int> AttributeComponents;
	AttributeComponents _attributeComponents;

	mutable uint32_t _time = 0u;

	std::string _name;

	const Uniform* getUniform(const std::string& name) const;

	int fetchUniforms();

	int fetchAttributes();

	bool createProgramFromShaders();

	std::string handleIncludes(const std::string& buffer) const;
public:
	Shader();
	virtual ~Shader();

	static std::string validGLSLPreprocessorName(const std::string& name);

	static int glslVersion;

	virtual void shutdown();

	bool load(const std::string& name, const std::string& buffer, ShaderType shaderType);

	std::string getSource(ShaderType shaderType, const std::string& buffer, bool finalize = true) const;

	/**
	 * If the shaders were loaded manually via @c ::load, then you have to initialize the shader manually, too
	 */
	bool init();

	virtual bool setup() {
		return false;
	}

	void recordUsedUniforms(bool state);

	void clearUsedUniforms();

	bool loadFromFile(const std::string& filename, ShaderType shaderType);

	/**
	 * @brief Loads a vertex and fragment shader for the given base filename.
	 *
	 * The filename is hand over to your @c Context implementation with the appropriate filename postfixes
	 *
	 * @see VERTEX_POSTFIX
	 * @see FRAGMENT_POSTFIX
	 */
	bool loadProgram(const std::string& filename);
	bool reload();

	/**
	 * @brief Returns the raw shader handle
	 */
	Id getShader(ShaderType shaderType) const;

	/**
	 * @brief Ticks the shader
	 */
	virtual void update(uint32_t deltaTime);

	/**
	 * @brief Bind the shader program
	 *
	 * @return @c true if is is useable now, @c false if not
	 */
	virtual bool activate() const;

	virtual bool deactivate() const;

	bool isActive() const;

	void checkAttribute(const std::string& attribute);
	void checkUniform(const std::string& uniform);
	void checkAttributes(std::initializer_list<std::string> attributes);
	void checkUniforms(std::initializer_list<std::string> uniforms);

	/**
	 * @brief Adds a new define in the form '#define value' to the shader source code
	 */
	void addDefine(const std::string& name, const std::string& value);

	void setUniformArraySize(const std::string& name, int size);
	void setAttributeComponents(int location, int size);
	int getAttributeComponents(int location) const;
	int getAttributeComponents(const std::string& name) const;

	/**
	 * @return -1 if uniform wasn't found, or no size is known. If the uniform is known, but
	 * it is no array, this might even return 0
	 */
	int getUniformArraySize(const std::string& name) const;

	int checkAttributeLocation(const std::string& name) const;
	int getAttributeLocation(const std::string& name) const;

	int getUniformLocation(const std::string& name) const;
	// returns a vector with offsets for the specified member names in the same order as the names
	// these offsets can be used to e.g. memcpy the data in.
	std::vector<int> getUniformBlockOffsets(const char **names, int amount) const;

	void setUniformui(const std::string& name, unsigned int value) const;
	void setUniform(const std::string& name, TextureUnit value) const;
	void setUniform(int location, TextureUnit value) const;
	void setUniformfv(int location, const float* values, int length, int components) const;
	void setUniformi(const std::string& name, int value) const;
	void setUniformi(const std::string& name, int value1, int value2) const;
	void setUniformi(const std::string& name, int value1, int value2, int value3) const;
	void setUniformi(const std::string& name, int value1, int value2, int value3, int value4) const;
	void setUniform1iv(const std::string& name, const int* values, int length) const;
	void setUniform2iv(const std::string& name, const int* values, int length) const;
	void setUniform3iv(const std::string& name, const int* values, int length) const;
	void setUniformf(const std::string& name, float value) const;
	void setUniformf(const std::string& name, float value1, float value2) const;
	void setUniformf(const std::string& name, float value1, float value2, float value3) const;
	void setUniformf(const std::string& name, float value1, float value2, float value3, float value4) const;
	void setUniformfv(const std::string& name, const float* values, int length, int components) const;
	void setUniform1fv(const std::string& name, const float* values, int length) const;
	void setUniform2fv(const std::string& name, const float* values, int length) const;
	void setUniform3fv(const std::string& name, const float* values, int length) const;
	void setUniformVec2(const std::string& name, const glm::vec2& value) const;
	void setUniformVec2(int location, const glm::vec2& value) const;
	void setUniformVec2v(const std::string& name, const glm::vec2* value, int length) const;
	void setUniformVec3(const std::string& name, const glm::vec3& value) const;
	void setUniformVec3v(const std::string& name, const glm::vec3* value, int length) const;
	void setUniform4fv(const std::string& name, const float* values, int length) const;
	void setUniformVec4(const std::string& name, const glm::vec4& value) const;
	void setUniformVec4(int location, const glm::vec4& value) const;
	void setUniformVec4v(const std::string& name, const glm::vec4* value, int length) const;
	void setUniformMatrix(const std::string& name, const glm::mat4& matrix, bool transpose = false) const;
	void setUniformMatrix(int location, const glm::mat4& matrix, bool transpose = false) const;
	void setUniformMatrix(const std::string& name, const glm::mat3& matrix, bool transpose = false) const;
	void setUniformMatrix(int location, const glm::mat3& matrix, bool transpose = false) const;
	void setUniformMatrixv(const std::string& name, const glm::mat4* matrixes, int amount, bool transpose = false) const;
	void setUniformMatrixv(const std::string& name, const glm::mat3* matrixes, int amount, bool transpose = false) const;
	void setUniformMatrixv(int location, const glm::mat3* matrixes, int amount, bool transpose = false) const;
	void setUniformf(const std::string& name, const glm::vec2& values) const;
	void setUniformf(int location, const glm::vec2& values) const;
	void setUniformf(const std::string& name, const glm::vec3& values) const;
	void setUniformf(int location, const glm::vec3& values) const;
	void setUniformf(const std::string& name, const glm::vec4& values) const;
	void setUniformf(int location, const glm::vec4& values) const;
	void setVertexAttribute(const std::string& name, int size, DataType type, bool normalize, int stride, const void* buffer) const;
	void setVertexAttributeInt(const std::string& name, int size, DataType type, int stride, const void* buffer) const;
	void disableVertexAttribute(const std::string& name) const;
	int enableVertexAttributeArray(const std::string& name) const;
	bool hasAttribute(const std::string& name) const;
	bool hasUniform(const std::string& name) const;
	bool isUniformBlock(const std::string& name) const;

	// particular renderer api must implement this

	bool setUniformBuffer(const std::string& name, const UniformBuffer& buffer);
	void setUniformui(int location, unsigned int value) const;
	void setUniformi(int location, int value) const;
	void setUniformi(int location, int value1, int value2) const;
	void setUniformi(int location, int value1, int value2, int value3) const;
	void setUniformi(int location, int value1, int value2, int value3, int value4) const;
	void setUniform1iv(int location, const int* values, int length) const;
	void setUniform2iv(int location, const int* values, int length) const;
	void setUniform3iv(int location, const int* values, int length) const;
	void setUniformf(int location, float value) const;
	void setUniformf(int location, float value1, float value2) const;
	void setUniformf(int location, float value1, float value2, float value3) const;
	void setUniformf(int location, float value1, float value2, float value3, float value4) const;
	void setUniform1fv(int location, const float* values, int length) const;
	void setUniform2fv(int location, const float* values, int length) const;
	void setUniformVec2v(int location, const glm::vec2* value, int length) const;
	void setUniformVec3(int location, const glm::vec3& value) const;
	void setUniform4fv(int location, const float* values, int length) const;
	void setUniform3fv(int location, const float* values, int length) const;
	void setUniformVec3v(int location, const glm::vec3* value, int length) const;
	void setUniformVec4v(int location, const glm::vec4* value, int length) const;
	void setUniformMatrixv(int location, const glm::mat4* matrixes, int amount, bool transpose = false) const;
	void setVertexAttribute(int location, int size, DataType type, bool normalize, int stride, const void* buffer) const;
	void setVertexAttributeInt(int location, int size, DataType type, int stride, const void* buffer) const;
	void disableVertexAttribute(int location) const;
	void enableVertexAttributeArray(int location) const;
	bool setDivisor(int location, uint32_t divisor) const;
};

inline void Shader::addUsedUniform(int location) const {
	_usedUniforms.insert(location);
}

inline void Shader::recordUsedUniforms(bool state) {
	_recordUsedUniforms = state;
}

inline void Shader::clearUsedUniforms() {
	_usedUniforms.clear();
}

inline void Shader::setUniformi(const std::string& name, int value) const {
	const int location = getUniformLocation(name);
	setUniformi(location, value);
}

inline void Shader::setUniform(const std::string& name, TextureUnit value) const {
	const int location = getUniformLocation(name);
	setUniform(location, value);
}

inline void Shader::setUniform(int location, TextureUnit value) const {
	setUniformi(location, std::enum_value(value));
}

inline void Shader::setUniformui(const std::string& name, unsigned int value) const {
	const int location = getUniformLocation(name);
	setUniformui(location, value);
}

inline void Shader::setUniformi(const std::string& name, int value1, int value2) const {
	const int location = getUniformLocation(name);
	setUniformi(location, value1, value2);
}

inline void Shader::setUniformi(const std::string& name, int value1, int value2, int value3) const {
	const int location = getUniformLocation(name);
	setUniformi(location, value1, value2, value3);
}

inline void Shader::setUniformi(const std::string& name, int value1, int value2, int value3, int value4) const {
	const int location = getUniformLocation(name);
	setUniformi(location, value1, value2, value3, value4);
}

inline void Shader::setUniform1iv(const std::string& name, const int* values, int length) const {
	const int location = getUniformLocation(name);
	setUniform1iv(location, values, length);
}

inline void Shader::setUniform2iv(const std::string& name, const int* values, int length) const {
	const int location = getUniformLocation(name);
	setUniform2iv(location, values, length);
}

inline void Shader::setUniform3iv(const std::string& name, const int* values, int length) const {
	const int location = getUniformLocation(name);
	setUniform3iv(location, values, length);
}

inline void Shader::setUniformf(const std::string& name, float value) const {
	const int location = getUniformLocation(name);
	setUniformf(location, value);
}

inline void Shader::setUniformf(const std::string& name, float value1, float value2) const {
	const int location = getUniformLocation(name);
	setUniformf(location, value1, value2);
}

inline void Shader::setUniformf(const std::string& name, float value1, float value2, float value3) const {
	const int location = getUniformLocation(name);
	setUniformf(location, value1, value2, value3);
}

inline void Shader::setUniformf(const std::string& name, float value1, float value2, float value3, float value4) const {
	const int location = getUniformLocation(name);
	setUniformf(location, value1, value2, value3, value4);
}

inline void Shader::setUniform1fv(const std::string& name, const float* values, int length) const {
	const int location = getUniformLocation(name);
	setUniform1fv(location, values, length);
}

inline void Shader::setUniform2fv(const std::string& name, const float* values, int length) const {
	const int location = getUniformLocation(name);
	setUniform2fv(location, values, length);
}

inline void Shader::setUniform3fv(const std::string& name, const float* values, int length) const {
	const int location = getUniformLocation(name);
	setUniform3fv(location, values, length);
}

inline void Shader::setUniformVec2(const std::string& name, const glm::vec2& value) const {
	const int location = getUniformLocation(name);
	setUniformVec2(location, value);
}

inline void Shader::setUniformVec2(int location, const glm::vec2& value) const {
	setUniform2fv(location, glm::value_ptr(value), 2);
}

inline void Shader::setUniformVec2v(const std::string& name, const glm::vec2* value, int length) const {
	const int location = getUniformLocation(name);
	setUniformVec2v(location, value, length);
}

inline void Shader::setUniformVec3(const std::string& name, const glm::vec3& value) const {
	const int location = getUniformLocation(name);
	setUniformVec3(location, value);
}

inline void Shader::setUniformVec3(int location, const glm::vec3& value) const {
	setUniform3fv(location, glm::value_ptr(value), 3);
}

inline void Shader::setUniformVec3v(const std::string& name, const glm::vec3* value, int length) const {
	const int location = getUniformLocation(name);
	setUniformVec3v(location, value, length);
}

inline void Shader::setUniformfv(const std::string& name, const float* values, int length, int components) const {
	const int location = getUniformLocation(name);
	setUniformfv(location, values, length, components);
}

inline void Shader::setUniformfv(int location, const float* values, int length, int components) const {
	if (components == 1) {
		setUniform1fv(location, values, length);
	} else if (components == 2) {
		setUniform2fv(location, values, length);
	} else if (components == 3) {
		setUniform3fv(location, values, length);
	} else {
		setUniform4fv(location, values, length);
	}
}

inline void Shader::setUniform4fv(const std::string& name, const float* values, int length) const {
	int location = getUniformLocation(name);
	setUniform4fv(location, values, length);
}

inline void Shader::setUniformVec4(const std::string& name, const glm::vec4& value) const {
	const int location = getUniformLocation(name);
	setUniformVec4(location, value);
}

inline void Shader::setUniformVec4(int location, const glm::vec4& value) const {
	setUniform4fv(location, glm::value_ptr(value), 4);
}

inline void Shader::setUniformVec4v(const std::string& name, const glm::vec4* value, int length) const {
	const int location = getUniformLocation(name);
	setUniformVec4v(location, value, length);
}

inline void Shader::setUniformMatrix(const std::string& name, const glm::mat4& matrix, bool transpose) const {
	setUniformMatrixv(name, &matrix, 1, transpose);
}

inline void Shader::setUniformMatrix(int location, const glm::mat4& matrix, bool transpose) const {
	setUniformMatrixv(location, &matrix, 1, transpose);
}

inline void Shader::setUniformMatrix(const std::string& name, const glm::mat3& matrix, bool transpose) const {
	setUniformMatrixv(name, &matrix, 1, transpose);
}

inline void Shader::setUniformMatrix(int location, const glm::mat3& matrix, bool transpose) const {
	setUniformMatrixv(location, &matrix, 1, transpose);
}

inline void Shader::setUniformMatrixv(const std::string& name, const glm::mat4* matrixes, int amount, bool transpose) const {
	const int location = getUniformLocation(name);
	setUniformMatrixv(location, matrixes, amount, transpose);
}

inline void Shader::setUniformMatrixv(const std::string& name, const glm::mat3* matrixes, int amount, bool transpose) const {
	const int location = getUniformLocation(name);
	setUniformMatrixv(location, matrixes, amount, transpose);
}

inline void Shader::setUniformf(const std::string& name, const glm::vec2& values) const {
	setUniformf(name, values.x, values.y);
}

inline void Shader::setUniformf(int location, const glm::vec2& values) const {
	setUniformf(location, values.x, values.y);
}

inline void Shader::setUniformf(const std::string& name, const glm::vec3& values) const {
	setUniformf(name, values.x, values.y, values.z);
}

inline void Shader::setUniformf(int location, const glm::vec3& values) const {
	setUniformf(location, values.x, values.y, values.z);
}

inline void Shader::setUniformf(const std::string& name, const glm::vec4& values) const {
	setUniformf(name, values.x, values.y, values.z, values.w);
}

inline void Shader::setUniformf(int location, const glm::vec4& values) const {
	setUniformf(location, values.x, values.y, values.z, values.w);
}

inline void Shader::setVertexAttribute(const std::string& name, int size, DataType type, bool normalize, int stride, const void* buffer) const {
	core_assert_msg(type == DataType::Float || type == DataType::Double, "unexpected data type given: %i", std::enum_value(type));
	const int location = getAttributeLocation(name);
	if (location == -1) {
		return;
	}
	setVertexAttribute(location, size, type, normalize, stride, buffer);
}

inline void Shader::setVertexAttributeInt(const std::string& name, int size, DataType type, int stride, const void* buffer) const {
	core_assert_msg(type != DataType::Float && type != DataType::Double, "unexpected data type given: %i", std::enum_value(type));
	const int location = getAttributeLocation(name);
	if (location == -1) {
		return;
	}
	setVertexAttributeInt(location, size, type, stride, buffer);
}

inline void Shader::disableVertexAttribute(const std::string& name) const {
	const int location = getAttributeLocation(name);
	if (location == -1) {
		return;
	}
	disableVertexAttribute(location);
}

inline int Shader::enableVertexAttributeArray(const std::string& name) const {
	int location = getAttributeLocation(name);
	if (location == -1) {
		return -1;
	}
	enableVertexAttributeArray(location);
	return location;
}

inline int Shader::getAttributeComponents(int location) const {
	auto i = _attributeComponents.find(location);
	if (i != _attributeComponents.end()) {
		return i->second;
	}
	return -1;
}

inline int Shader::getAttributeComponents(const std::string& name) const {
	const int loc = getAttributeLocation(name);
	if (loc == -1) {
		return -1;
	}
	return getAttributeComponents(loc);
}

inline bool Shader::hasAttribute(const std::string& name) const {
	return _attributes.find(name) != _attributes.end();
}

inline bool Shader::hasUniform(const std::string& name) const {
	return _uniforms.find(name) != _uniforms.end();
}

inline bool Shader::isUniformBlock(const std::string& name) const {
	auto i = _uniforms.find(name);
	if (i == _uniforms.end()) {
		return false;
	}
	return i->second.block;
}

inline bool Shader::isActive() const {
	return _active;
}

class ScopedShader {
private:
	const Shader& _shader;
public:
	ScopedShader(const Shader& shader) :
			_shader(shader) {
		_shader.activate();
	}

	virtual ~ScopedShader() {
		_shader.deactivate();
	}
};

#define shaderSetUniformIf(shader, func, var, ...) if (shader.hasUniform(var)) { shader.func(var, __VA_ARGS__); }

typedef std::shared_ptr<Shader> ShaderPtr;

}
