#ifndef SHADER_H
#define SHADER_H

class Shader;

/*--------------------------------------------//
Includes
//--------------------------------------------*/
    #include "../../deps/glm/glm.hpp"
    #include "../../deps/glm/gtc/matrix_transform.hpp"
    #include "../../deps/glm/gtc/type_ptr.hpp"
    #include "../../deps/gl/glew.h"
    #include "../common/vector.h"

    #include <stdlib.h>
    #include <string.h>
    #include <fstream>
    #include <sstream>
    #include <iostream>

/*--------------------------------------------//
Shader Class
//--------------------------------------------*/
    class Shader{
        private:
            /*--------------------------------------------//
            Functions
            //--------------------------------------------*/
                void printShaderInfoLog(GLint shader);
                void checkCompileErrors(GLuint shader, char* type);
                std::string loadFile(const char *fname);
        
        public: 
            /*--------------------------------------------//
            Class Variables
            //--------------------------------------------*/
                unsigned int ID;   
            /*--------------------------------------------//
            constructor
            //--------------------------------------------*/
                Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);
            
            /*--------------------------------------------//
            Functions
            //--------------------------------------------*/
                void use();
                int getAttribLoc(const std::string &name) const;
                int getUniformLoc(const std::string &name) const;
                void setBool(const std::string &name, bool value) const;
                void setInt(const std::string &name, int value) const;
                void setFloat(const std::string &name, float value) const;
                void setVec2(const std::string &name, const vec2 &value) const;
                void setVec2(const std::string &name, float x, float y) const;
                void setVec3(const std::string &name, const vec3 &value) const;
                void setVec3(const std::string &name, float x, float y, float z) const;
                void setVec4(const std::string &name, const vec4 &value) const;
                void setVec4(const std::string &name, float x, float y, float z, float w) const;
                void setMat4(const std::string &name, float* value) const;
                void setMat2(const std::string &name, const glm::mat2 &mat) const;
                void setMat3(const std::string &name, const glm::mat3 &mat) const;
                void setMat4(const std::string &name, const glm::mat4 &mat) const;
    };
#endif