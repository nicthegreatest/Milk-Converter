#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <algorithm>
#include <cctype>

#include "PresetFileParser.hpp"

// Include internal headers from projectm-eval to access AST and context structures
extern "C" {
#include "vendor/projectm-master/vendor/projectm-eval/projectm-eval/api/projectm-eval.h"
#include "vendor/projectm-master/vendor/projectm-eval/projectm-eval/CompilerTypes.h"
#include "vendor/projectm-master/vendor/projectm-eval/projectm-eval/CompileContext.h"
#include "vendor/projectm-master/vendor/projectm-eval/projectm-eval/TreeFunctions.h"
}

// The public API uses an opaque pointer, so we must cast it to the internal type.
#define internal_context(ctx) (reinterpret_cast<prjm_eval_compiler_context_t*>(ctx))

// A map of MilkDrop built-in variables to their GLSL equivalents.
const std::unordered_map<std::string, std::string> milkToGLSLVars = {
    {"time", "iTime"},
    {"fps", "iFps"},
    {"frame", "iFrame"},
    {"progress", "iProgress"},
    {"bass", "iAudioBands.x"},
    {"mid", "iAudioBands.y"},
    {"treb", "iAudioBands.z"},
    {"bass_att", "iAudioBandsAtt.x"},
    {"mid_att", "iAudioBandsAtt.y"},
    {"treb_att", "iAudioBandsAtt.z"},
    {"x", "uv.x"},
    {"y", "uv.y"},
    {"rad", "length(uv - vec2(0.5))"},
    {"ang", "atan(uv.y - 0.5, uv.x - 0.5)"},
};

// Metadata for generating UI controls for writable variables.
struct UniformControl {
    std::string defaultValue;
    std::string widget;
    std::string min;
    std::string max;
    std::string step;
};

const std::unordered_map<std::string, UniformControl> uniformControls = {
    {"zoom", {"1.0", "slider", "0.5", "1.5", "0.01"}},
    {"zoomexp", {"1.0", "slider", "0.5", "2.0", "0.01"}},
    {"rot", {"0.0", "slider", "-0.1", "0.1", "0.001"}},
    {"warp", {"1.0", "slider", "0.0", "2.0", "0.01"}},
    {"cx", {"0.5", "slider", "0.0", "1.0", "0.01"}},
    {"cy", {"0.5", "slider", "0.0", "1.0", "0.01"}},
    {"dx", {"0.0", "slider", "-0.1", "0.1", "0.001"}},
    {"dy", {"0.0", "slider", "-0.1", "0.1", "0.001"}},
    {"sx", {"1.0", "slider", "0.5", "1.5", "0.01"}},
    {"sy", {"1.0", "slider", "0.5", "1.5", "0.01"}},
    {"wave_r", {"0.5", "slider", "0.0", "1.0", "0.01"}},
    {"wave_g", {"0.5", "slider", "0.0", "1.0", "0.01"}},
    {"wave_b", {"0.5", "slider", "0.0", "1.0", "0.01"}},
    {"wave_a", {"1.0", "slider", "0.0", "1.0", "0.01"}},
    {"wave_x", {"0.5", "slider", "0.0", "1.0", "0.01"}},
    {"wave_y", {"0.5", "slider", "0.0", "1.0", "0.01"}},
    {"wave_mystery", {"0.0", "slider", "-1.0", "1.0", "0.01"}},
    {"decay", {"0.98", "slider", "0.9", "1.0", "0.001"}},
    {"r", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"g", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"b", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"a", {"1.0", "slider", "0.0", "1.0", "0.01"}},
};


class GLSLGenerator {
public:
    GLSLGenerator(projectm_eval_context* context);
    std::string generate(const prjm_eval_exptreenode* tree);

private:
    std::string traverseNode(const prjm_eval_exptreenode* node);
    bool isOperator(const prjm_eval_exptreenode* node);
    bool isFunction(const prjm_eval_exptreenode* node);
    bool isAssignment(const prjm_eval_exptreenode* node);
    bool isConstant(const prjm_eval_exptreenode* node);
    bool isVariable(const prjm_eval_exptreenode* node);
    std::string getFunctionName(const prjm_eval_exptreenode* node);
    std::string getVariableName(const prjm_eval_exptreenode* node);
    std::string getOperator(const prjm_eval_exptreenode* node);

    std::unordered_map<void*, std::string> m_func_map;
    projectm_eval_context* m_context;
};

GLSLGenerator::GLSLGenerator(projectm_eval_context* context)
    : m_context(context)
{
    m_func_map[(void*)prjm_eval_func_execute_list] = "execute_list";
    m_func_map[(void*)prjm_eval_func_add] = "+";
    m_func_map[(void*)prjm_eval_func_sub] = "-";
    m_func_map[(void*)prjm_eval_func_mul] = "*";
    m_func_map[(void*)prjm_eval_func_div] = "/";
    m_func_map[(void*)prjm_eval_func_mod] = "%";
    m_func_map[(void*)prjm_eval_func_bitwise_and] = "&";
    m_func_map[(void*)prjm_eval_func_bitwise_or] = "|";
    m_func_map[(void*)prjm_eval_func_equal] = "==";
    m_func_map[(void*)prjm_eval_func_notequal] = "!=";
    m_func_map[(void*)prjm_eval_func_above] = ">";
    m_func_map[(void*)prjm_eval_func_aboveeq] = ">=";
    m_func_map[(void*)prjm_eval_func_below] = "<";
    m_func_map[(void*)prjm_eval_func_beloweq] = "<=";
    m_func_map[(void*)prjm_eval_func_set] = "=";
    m_func_map[(void*)prjm_eval_func_sin] = "sin";
    m_func_map[(void*)prjm_eval_func_cos] = "cos";
    m_func_map[(void*)prjm_eval_func_tan] = "tan";
    m_func_map[(void*)prjm_eval_func_asin] = "asin";
    m_func_map[(void*)prjm_eval_func_acos] = "acos";
    m_func_map[(void*)prjm_eval_func_atan] = "atan";
    m_func_map[(void*)prjm_eval_func_sqrt] = "sqrt";
    m_func_map[(void*)prjm_eval_func_pow] = "pow";
    m_func_map[(void*)prjm_eval_func_abs] = "abs";
    m_func_map[(void*)prjm_eval_func_if] = "if";
    m_func_map[(void*)prjm_eval_func_sqr] = "sqr";
    m_func_map[(void*)prjm_eval_func_log] = "log";
    m_func_map[(void*)prjm_eval_func_log10] = "log10";
    m_func_map[(void*)prjm_eval_func_mem] = "megabuf";
    m_func_map[(void*)prjm_eval_func_sign] = "sign";
    m_func_map[(void*)prjm_eval_func_rand] = "rand";
    m_func_map[(void*)prjm_eval_func_min] = "min";
    m_func_map[(void*)prjm_eval_func_max] = "max";
    m_func_map[(void*)prjm_eval_func_floor] = "floor";
    m_func_map[(void*)prjm_eval_func_ceil] = "ceil";
    m_func_map[(void*)prjm_eval_func_boolean_and_func] = "band";
    m_func_map[(void*)prjm_eval_func_boolean_or_func] = "bor";
}

std::string GLSLGenerator::generate(const prjm_eval_exptreenode* tree) {
    if (!tree) {
        return "";
    }
    std::string result;
    if (tree->func == prjm_eval_func_execute_list) {
        if (tree->args) {
            for (int i = 0; tree->args[i] != nullptr; ++i) {
                result += "    " + traverseNode(tree->args[i]) + ";\n";
            }
        }
    } else {
        result += "    " + traverseNode(tree) + ";\n";
    }
    return result;
}

std::string GLSLGenerator::traverseNode(const prjm_eval_exptreenode* node) {
    if (!node) return "/* null node */";
    if (isConstant(node)) {
        std::stringstream ss;
        ss << node->value;
        std::string val_str = ss.str();
        if (val_str.find('.') == std::string::npos && val_str.find('e') == std::string::npos) {
            val_str += ".0";
        }
        return val_str;
    }
    if (isVariable(node)) {
        std::string varName = getVariableName(node);
        if (milkToGLSLVars.count(varName)) {
            return milkToGLSLVars.at(varName);
        }
        return varName;
    }
    if (isAssignment(node)) {
        return traverseNode(node->args[0]) + " = " + traverseNode(node->args[1]);
    }
    if (isOperator(node)) {
        return "(" + traverseNode(node->args[0]) + " " + getOperator(node) + " " + traverseNode(node->args[1]) + ")";
    }
    if (isFunction(node)) {
        std::string funcName = getFunctionName(node);
        if (funcName == "if") {
            return "((" + traverseNode(node->args[0]) + " != 0.0) ? (" + traverseNode(node->args[1]) + ") : (" + traverseNode(node->args[2]) + "))";
        }
        if (funcName == "sqr") {
            std::string arg = traverseNode(node->args[0]);
            return "((" + arg + ")*(" + arg + "))";
        }
        if (funcName == "band") {
            return "((" + traverseNode(node->args[0]) + " != 0.0) && (" + traverseNode(node->args[1]) + " != 0.0))";
        }
        if (funcName == "bor") {
            return "((" + traverseNode(node->args[0]) + " != 0.0) || (" + traverseNode(node->args[1]) + " != 0.0))";
        }
        std::string args;
        if (node->args) {
            for (int i = 0; node->args[i] != nullptr; ++i) {
                if (i > 0) args += ", ";
                args += traverseNode(node->args[i]);
            }
        }
        return funcName + "(" + args + ")";
    }
    return "/* unknown node */";
}

bool GLSLGenerator::isOperator(const prjm_eval_exptreenode* n) {
    if (!n || !n->func) return false;
    auto it = m_func_map.find((void*)n->func);
    if (it == m_func_map.end()) return false;
    const auto& name = it->second;
    return name == "+" || name == "-" || name == "*" || name == "/" || name == "%" || name == "==" || name == "!=" || name == ">" || name == ">=" || name == "<" || name == "<=";
}
bool GLSLGenerator::isFunction(const prjm_eval_exptreenode* n) {
    if (!n || !n->func) return false;
    return !isOperator(n) && !isAssignment(n) && !isVariable(n) && !isConstant(n);
}
bool GLSLGenerator::isAssignment(const prjm_eval_exptreenode* n) { return n && n->func == prjm_eval_func_set; }
bool GLSLGenerator::isConstant(const prjm_eval_exptreenode* n) { return n && n->func == prjm_eval_func_const; }
bool GLSLGenerator::isVariable(const prjm_eval_exptreenode* n) { return n && n->func == prjm_eval_func_var; }
std::string GLSLGenerator::getFunctionName(const prjm_eval_exptreenode* n) {
    if (!n || !n->func) return "";
    auto it = m_func_map.find((void*)n->func);
    return (it != m_func_map.end()) ? it->second : "";
}
std::string GLSLGenerator::getVariableName(const prjm_eval_exptreenode* n) {
    if (!n || !n->var) return "/* unknown_var */";
    if (!m_context) return "/* no_ctx */";
    prjm_eval_variable_entry_t* current = internal_context(m_context)->variables.first;
    while (current) {
        if (&current->variable->value == n->var) return current->variable->name;
        current = current->next;
    }
    return "/* var_not_found */";
}
std::string GLSLGenerator::getOperator(const prjm_eval_exptreenode* n) {
    if (!n || !n->func) return "";
    auto it = m_func_map.find((void*)n->func);
    return (it != m_func_map.end()) ? it->second : "";
}

std::set<std::string> findUserVars(prjm_eval_compiler_context_t* ctx) {
    std::set<std::string> userVars;
    prjm_eval_variable_entry_t* current = ctx->variables.first;
    while(current) {
        std::string varName = current->variable->name;
        if (milkToGLSLVars.count(varName) == 0 && uniformControls.count(varName) == 0 && !std::regex_match(varName, std::regex("q[1-9][0-9]?|t[1-8]"))) {
            userVars.insert(varName);
        }
        current = current->next;
    }
    return userVars;
}

std::string translateToGLSL(const std::string& perFrame, const std::string& perPixel) {
    projectm_eval_context* context = projectm_eval_context_create(nullptr, nullptr);
    if (!context) {
        std::cerr << "Failed to create projectm-eval context." << std::endl;
        return "";
    }
    prjm_eval_program_t* perFrameAST = prjm_eval_compile_code(internal_context(context), perFrame.c_str());
    if (!perFrameAST) {
        int line, col;
        const char* error = projectm_eval_get_error(context, &line, &col);
        std::cerr << "Error parsing per-frame code: " << (error ? error : "Unknown error")
                  << " at line " << line << ", col " << col << std::endl;
    }

    prjm_eval_program_t* perPixelAST = prjm_eval_compile_code(internal_context(context), perPixel.c_str());
    if (!perPixelAST) {
        int line, col;
        const char* error = projectm_eval_get_error(context, &line, &col);
        std::cerr << "Error parsing per-pixel code: " << (error ? error : "Unknown error")
                  << " at line " << line << ", col " << col << std::endl;
    }

    auto userVars = findUserVars(internal_context(context));
    GLSLGenerator generator(context);
    std::string perFrameGLSL = generator.generate(perFrameAST ? perFrameAST->program : nullptr);
    std::string perPixelGLSL = generator.generate(perPixelAST ? perPixelAST->program : nullptr);
    prjm_eval_destroy_code(perFrameAST);
    prjm_eval_destroy_code(perPixelAST);
    projectm_eval_context_destroy(context);

    std::string glsl = "#version 330 core\n\nout vec4 FragColor;\nin vec2 uv;\n\n";
    glsl += "// Standard RaymarchVibe uniforms\n";
    glsl += "uniform float iTime;\nuniform vec2 iResolution;\nuniform float iFps;\nuniform float iFrame;\nuniform float iProgress;\nuniform vec4 iAudioBands;\nuniform vec4 iAudioBandsAtt;\n\n";
    glsl += "// Preset-specific uniforms with UI annotations\n";
    for(const auto& pair : uniformControls) {
        glsl += "uniform float u_" + pair.first + "; // {\"widget\":\"" + pair.second.widget + "\",\"default\":" + pair.second.defaultValue + ",\"min\":" + pair.second.min + ",\"max\":" + pair.second.max + ",\"step\":" + pair.second.step + "}\n";
    }
    glsl += "\nvoid main() {\n";
    glsl += "    // Initialize local variables from uniforms\n";
    for(const auto& pair : uniformControls) {
        glsl += "    float " + pair.first + " = u_" + pair.first + ";\n";
    }
    glsl += "\n    // State variables\n";
    for (int i = 1; i <= 32; ++i) glsl += "    float q" + std::to_string(i) + " = 0.0;\n";
    for (int i = 1; i <= 8; ++i) glsl += "    float t" + std::to_string(i) + " = 0.0;\n";
    if (!userVars.empty()) {
        for (const auto& var : userVars) glsl += "    float " + var + " = 0.0;\n";
    }
    glsl += "\n    // Per-frame logic\n";
    glsl += perFrameGLSL;
    glsl += "\n    // Per-pixel logic\n";
    glsl += perPixelGLSL;
    glsl += "\n    FragColor = vec4(r, g, b, a);\n}\n";
    return glsl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.milk> <output.frag>\n";
        return 1;
    }
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    libprojectM::PresetFileParser parser;
    if (!parser.Read(inputFile)) {
        std::cerr << "Error: Could not read or parse input file: " << inputFile << "\n";
        return 1;
    }
    std::string perFrameCode, perPixelCode;
    auto toLower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    };
    const auto& values = parser.PresetValues();
    for (const auto& pair : values) {
        std::string lowerKey = toLower(pair.first);
        if (lowerKey.find("per_frame_") == 0) {
            perFrameCode += pair.second + ";";
        } else if (lowerKey.find("per_pixel_") == 0) {
            perPixelCode += pair.second + ";";
        }
    }
    std::string glsl = translateToGLSL(perFrameCode, perPixelCode);
    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Error: Could not open output file for writing: " << outputFile << "\n";
        return 1;
    }
    out << glsl;
    std::cout << "Successfully converted " << inputFile << " to " << outputFile << "\n";
    return 0;
}