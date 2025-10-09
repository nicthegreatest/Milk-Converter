// Dummy implementations for mutex functions required by the projectm-eval library.
// This is a single-threaded application, so no actual locking is needed.
extern "C" {
void projectm_eval_memory_host_lock_mutex() {}
void projectm_eval_memory_host_unlock_mutex() {}
}

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
#include <clocale>

#include "PresetFileParser.hpp"

// Include internal headers from projectm-eval to access AST and context structures
extern "C" {
#include "projectm-eval.h"
#include "projectm-eval/CompilerTypes.h"
#include "projectm-eval/CompileContext.h"
#include "projectm-eval/TreeFunctions.h"
#include "projectm-eval/ExpressionTree.h"
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
    bool isComparison(const prjm_eval_exptreenode* node);
    bool isFunction(const prjm_eval_exptreenode* node);
    bool isAssignment(const prjm_eval_exptreenode* node);
    bool isConstant(const prjm_eval_exptreenode* node);
    bool isVariable(const prjm_eval_exptreenode* node);
    std::string getFunctionName(const prjm_eval_exptreenode* node);
    std::string getVariableName(const prjm_eval_exptreenode* node);
    std::string getOperator(const prjm_eval_exptreenode* node);

    std::unordered_map<void*, std::string> m_func_map;
    std::set<void*> m_comparison_funcs;
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

    m_comparison_funcs.insert((void*)prjm_eval_func_equal);
    m_comparison_funcs.insert((void*)prjm_eval_func_notequal);
    m_comparison_funcs.insert((void*)prjm_eval_func_above);
    m_comparison_funcs.insert((void*)prjm_eval_func_aboveeq);
    m_comparison_funcs.insert((void*)prjm_eval_func_below);
    m_comparison_funcs.insert((void*)prjm_eval_func_beloweq);

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
    m_func_map[(void*)prjm_eval_func_bnot] = "bnot";
    m_func_map[(void*)prjm_eval_func_boolean_and_func] = "band";
    m_func_map[(void*)prjm_eval_func_boolean_or_func] = "bor";
}

std::string GLSLGenerator::generate(const prjm_eval_exptreenode* tree) {
    if (!tree) return "";
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
        auto it = milkToGLSLVars.find(varName);
        return (it != milkToGLSLVars.end()) ? it->second : varName;
    }
    if (isAssignment(node)) {
        return traverseNode(node->args[0]) + " = " + traverseNode(node->args[1]);
    }
    std::string funcName = getFunctionName(node);
    if (isOperator(node)) {
        if (isComparison(node)) {
            return "float_from_bool((" + traverseNode(node->args[0]) + " " + funcName + " " + traverseNode(node->args[1]) + "))";
        }
        if (funcName == "%") {
            return "mod(" + traverseNode(node->args[0]) + ", " + traverseNode(node->args[1]) + ")";
        }
        return "(" + traverseNode(node->args[0]) + " " + funcName + " " + traverseNode(node->args[1]) + ")";
    }
    if (isFunction(node)) {
        if (funcName == "if") {
            std::string cond = traverseNode(node->args[0]);
            // If the condition is a float_from_bool call, we can unwrap it to get the raw boolean expression.
            if (cond.rfind("float_from_bool(", 0) == 0) {
                std::string inner_cond = cond.substr(16, cond.length() - 17); // "float_from_bool(".length()
                return "((" + inner_cond + ") ? (" + traverseNode(node->args[1]) + ") : (" + traverseNode(node->args[2]) + "))";
            }
            // Otherwise, treat it as a float and compare to 0.0
            return "((" + cond + " != 0.0) ? (" + traverseNode(node->args[1]) + ") : (" + traverseNode(node->args[2]) + "))";
        }
        if (funcName == "sqr") return "((" + traverseNode(node->args[0]) + ")*(" + traverseNode(node->args[0]) + "))";
        if (funcName == "bnot") return "float_from_bool(" + traverseNode(node->args[0]) + " == 0.0)";
        if (funcName == "band") return "float_from_bool((" + traverseNode(node->args[0]) + " != 0.0) && (" + traverseNode(node->args[1]) + " != 0.0))";
        if (funcName == "bor") return "float_from_bool((" + traverseNode(node->args[0]) + " != 0.0) || (" + traverseNode(node->args[1]) + " != 0.0))";

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
    return n->func == prjm_eval_func_add || n->func == prjm_eval_func_sub || n->func == prjm_eval_func_mul || n->func == prjm_eval_func_div || n->func == prjm_eval_func_mod || n->func == prjm_eval_func_equal || n->func == prjm_eval_func_notequal || n->func == prjm_eval_func_above || n->func == prjm_eval_func_aboveeq || n->func == prjm_eval_func_below || n->func == prjm_eval_func_beloweq;
}
bool GLSLGenerator::isComparison(const prjm_eval_exptreenode* n) {
    if (!n || !n->func) return false;
    return m_comparison_funcs.count((void*)n->func) > 0;
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
std::string GLSLGenerator::getOperator(const prjm_eval_exptreenode* n) { return getFunctionName(n); }

// Helper function to compile a block of statements into a single AST
prjm_eval_exptreenode* compile_statements(projectm_eval_context* ctx, const std::string& code) {
    std::vector<prjm_eval_program_t*> programs;
    std::stringstream ss(code);
    std::string statement;

    while (std::getline(ss, statement, ';')) {
        // Trim leading/trailing whitespace
        statement.erase(0, statement.find_first_not_of(" \t\n\r"));
        statement.erase(statement.find_last_not_of(" \t\n\r") + 1);

        if (statement.empty()) {
            continue;
        }

        prjm_eval_program_t* program = prjm_eval_compile_code(internal_context(ctx), statement.c_str());
        if (program && program->program) {
            programs.push_back(program);
        } else {
            int line, col;
            const char* error = projectm_eval_get_error(ctx, &line, &col);
            std::cerr << "Error parsing statement '" << statement << "': " << (error ? error : "Unknown error") << " at line " << line << ", col " << col << std::endl;
            if(program) {
                prjm_eval_destroy_code(program);
            }
            // Clean up already compiled programs
            for(auto p : programs) {
                prjm_eval_destroy_code(p);
            }
            return nullptr;
        }
    }

    if (programs.empty()) {
        return nullptr;
    }

    // Manually create an execute_list node to combine all statements
    prjm_eval_exptreenode* combined_ast = (prjm_eval_exptreenode*) calloc(1, sizeof(prjm_eval_exptreenode));
    combined_ast->func = prjm_eval_func_execute_list;
    combined_ast->args = (prjm_eval_exptreenode**) calloc(programs.size() + 1, sizeof(prjm_eval_exptreenode*));

    for (size_t i = 0; i < programs.size(); ++i) {
        combined_ast->args[i] = programs[i]->program;
        programs[i]->program = nullptr; // Avoid double-free by nulling the pointer
    }
    combined_ast->args[programs.size()] = nullptr; // Null-terminate the list

    // Cleanup the now-empty program containers
    for(auto p : programs) {
        prjm_eval_destroy_code(p);
    }

    return combined_ast;
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

    prjm_eval_exptreenode* perFrameAST = compile_statements(context, perFrame);
    prjm_eval_exptreenode* perPixelAST = compile_statements(context, perPixel);

    auto userVars = findUserVars(internal_context(context));
    GLSLGenerator generator(context);
    std::string perFrameGLSL = generator.generate(perFrameAST);
    std::string perPixelGLSL = generator.generate(perPixelAST);

    // Cleanup the combined ASTs
    if(perFrameAST) prjm_eval_destroy_exptreenode(perFrameAST);
    if(perPixelAST) prjm_eval_destroy_exptreenode(perPixelAST);

    projectm_eval_context_destroy(context);

    std::string glsl = "#version 330 core\n\nout vec4 FragColor;\nin vec2 uv;\n\n";
    glsl += "float float_from_bool(bool b) { return b ? 1.0 : 0.0; }\n\n";
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
    glsl += R"___(
    // Apply coordinate transformations calculated in per-pixel logic.
    // This emulates the 'warp' part of a MilkDrop shader.
    vec2 transformed_uv = uv - vec2(cx, cy); // Center on cx, cy

    mat2 rotation_matrix = mat2(cos(rot), -sin(rot), sin(rot), cos(rot));
    transformed_uv = rotation_matrix * transformed_uv;

    transformed_uv /= zoom;
    transformed_uv /= vec2(sx, sy);

    transformed_uv += vec2(dx, dy); // Pan
    transformed_uv += vec2(cx, cy); // Un-center

)___";
    glsl += "\n    // Final color composition\n";
    glsl += "    FragColor = vec4(ob_r, ob_g, ob_b, ob_a);\n\n";
    glsl += "    // In a real engine, transformed_uv would sample a feedback buffer.\n";
    glsl += "    // To visualize the warp effect, we modulate the color by the transformed UVs.\n";
    glsl += "    FragColor.r *= transformed_uv.x;\n";
    glsl += "    FragColor.g *= transformed_uv.y;\n";
    glsl += "}\n";
    return glsl;
}


int main(int argc, char* argv[]) {
    std::setlocale(LC_NUMERIC, "C");
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

    std::string perFrameCode = parser.GetCode("per_frame_");
    std::string perPixelCode = parser.GetCode("per_pixel_");

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