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
#include <cmath>

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

#include "WaveModeRenderer.hpp"

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
    {"aspectx", "(iResolution.y / iResolution.x)"},
    {"aspecty", "(iResolution.x / iResolution.y)"},
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
    {"gamma", {"1.0", "slider", "0.1", "5.0", "0.01"}},
    {"brighten", {"0.0", "slider", "0.0", "1.0", "1.0"}},
    {"darken", {"0.0", "slider", "0.0", "1.0", "1.0"}},
    {"solarize", {"0.0", "slider", "0.0", "1.0", "1.0"}},
    {"wrap", {"1.0", "slider", "0.0", "1.0", "1.0"}},
    {"invert", {"0.0", "slider", "0.0", "1.0", "1.0"}},
    {"darken_center", {"0.0", "slider", "0.0", "1.0", "1.0"}},
    {"r", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"g", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"b", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"a", {"1.0", "slider", "0.0", "1.0", "0.01"}},
    {"ob_size", {"0.01", "slider", "0.0", "0.1", "0.001"}},
    {"ob_r", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"ob_g", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"ob_b", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"ob_a", {"1.0", "slider", "0.0", "1.0", "0.01"}},
    {"ib_size", {"0.01", "slider", "0.0", "0.1", "0.001"}},
    {"ib_r", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"ib_g", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"ib_b", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"ib_a", {"1.0", "slider", "0.0", "1.0", "0.01"}},
    {"mv_x", {"12.0", "slider", "0.0", "64.0", "1.0"}},
    {"mv_y", {"9.0", "slider", "0.0", "48.0", "1.0"}},
    {"mv_dx", {"0.0", "slider", "-0.1", "0.1", "0.001"}},
    {"mv_dy", {"0.0", "slider", "-0.1", "0.1", "0.001"}},
    {"mv_l", {"0.5", "slider", "0.0", "2.0", "0.01"}},
    {"mv_r", {"1.0", "slider", "0.0", "1.0", "0.01"}},
    {"mv_g", {"1.0", "slider", "0.0", "1.0", "0.01"}},
    {"mv_b", {"1.0", "slider", "0.0", "1.0", "0.01"}},
    {"mv_a", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"echo_zoom", {"1.0", "slider", "0.5", "2.0", "0.01"}},
    {"echo_alpha", {"0.0", "slider", "0.0", "1.0", "0.01"}},
    {"echo_orient", {"0.0", "slider", "0.0", "3.0", "1.0"}},
};

const std::unordered_map<std::string, std::string>& perPixelVariableRewrites() {
    static const std::unordered_map<std::string, std::string> rewrites = {
        {"red", "pixelColor.r"},
        {"green", "pixelColor.g"},
        {"blue", "pixelColor.b"},
        {"alpha", "pixelColor.a"}
    };
    return rewrites;
}

class GLSLGenerator {
public:
    GLSLGenerator(projectm_eval_context* context);
    std::string generate(const prjm_eval_exptreenode* tree);
    std::string generate(const prjm_eval_exptreenode* tree, const std::unordered_map<std::string, std::string>& variableOverrides);

private:
    std::string generateWithOverrides(const prjm_eval_exptreenode* tree, const std::unordered_map<std::string, std::string>* overrides);
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
    const std::unordered_map<std::string, std::string>* m_variableOverrides;
};

GLSLGenerator::GLSLGenerator(projectm_eval_context* context)
    : m_context(context)
    , m_variableOverrides(nullptr)
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
    m_func_map[(void*)prjm_eval_func_atan2] = "atan2";
    m_func_map[(void*)prjm_eval_func_sqrt] = "sqrt";
    m_func_map[(void*)prjm_eval_func_pow] = "pow";
    m_func_map[(void*)prjm_eval_func_exp] = "exp";
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
    m_func_map[(void*)prjm_eval_func_invsqrt] = "inversesqrt";
    m_func_map[(void*)prjm_eval_func_sigmoid] = "sigmoid_eel";
    m_func_map[(void*)prjm_eval_func_bnot] = "bnot";
    m_func_map[(void*)prjm_eval_func_boolean_and_func] = "band";
    m_func_map[(void*)prjm_eval_func_boolean_or_func] = "bor";
    m_func_map[(void*)prjm_eval_func_boolean_and_op] = "boolean_and_op_eel";
    m_func_map[(void*)prjm_eval_func_boolean_or_op] = "boolean_or_op_eel";
    m_func_map[(void*)prjm_eval_func_exec2] = "exec2_helper";
    m_func_map[(void*)prjm_eval_func_exec3] = "exec3_helper";
}

std::string GLSLGenerator::generate(const prjm_eval_exptreenode* tree) {
    return generateWithOverrides(tree, nullptr);
}

std::string GLSLGenerator::generate(const prjm_eval_exptreenode* tree, const std::unordered_map<std::string, std::string>& variableOverrides) {
    return generateWithOverrides(tree, &variableOverrides);
}

std::string GLSLGenerator::generateWithOverrides(const prjm_eval_exptreenode* tree, const std::unordered_map<std::string, std::string>* overrides) {
    if (!tree) return "";
    const auto* previousOverrides = m_variableOverrides;
    m_variableOverrides = overrides;

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

    m_variableOverrides = previousOverrides;
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
        if (m_variableOverrides) {
            auto overrideIt = m_variableOverrides->find(varName);
            if (overrideIt != m_variableOverrides->end()) {
                return overrideIt->second;
            }
        }
        auto it = milkToGLSLVars.find(varName);
        return (it != milkToGLSLVars.end()) ? it->second : varName;
    }
    if (isAssignment(node)) {
        return traverseNode(node->args[0]) + " = " + traverseNode(node->args[1]);
    }
    if (node->func == prjm_eval_func_neg) {
        return "(-" + traverseNode(node->args[0]) + ")";
    }
    if (node->func == prjm_eval_func_add_op || node->func == prjm_eval_func_sub_op || node->func == prjm_eval_func_mul_op ||
        node->func == prjm_eval_func_div_op || node->func == prjm_eval_func_mod_op || node->func == prjm_eval_func_bitwise_and_op ||
        node->func == prjm_eval_func_bitwise_or_op || node->func == prjm_eval_func_pow_op) {
        std::string lhs = traverseNode(node->args[0]);
        std::string rhs = traverseNode(node->args[1]);
        if (node->func == prjm_eval_func_add_op) return lhs + " = " + lhs + " + " + rhs;
        if (node->func == prjm_eval_func_sub_op) return lhs + " = " + lhs + " - " + rhs;
        if (node->func == prjm_eval_func_mul_op) return lhs + " = " + lhs + " * " + rhs;
        if (node->func == prjm_eval_func_div_op) return lhs + " = " + lhs + " / " + rhs;
        if (node->func == prjm_eval_func_mod_op) return lhs + " = mod(" + lhs + ", " + rhs + ")";
        if (node->func == prjm_eval_func_bitwise_and_op) return lhs + " = float(int(" + lhs + ") & int(" + rhs + "))";
        if (node->func == prjm_eval_func_bitwise_or_op) return lhs + " = float(int(" + lhs + ") | int(" + rhs + "))";
        return lhs + " = pow(" + lhs + ", " + rhs + ")";
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
        if (funcName == "atan2") {
            return "atan(" + traverseNode(node->args[0]) + ", " + traverseNode(node->args[1]) + ")";
        }
        if (funcName == "sqr") return "((" + traverseNode(node->args[0]) + ")*(" + traverseNode(node->args[0]) + "))";
        if (funcName == "rand") return "(rand(uv) * " + traverseNode(node->args[0]) + ")";
        if (funcName == "bnot") return "float_from_bool(" + traverseNode(node->args[0]) + " == 0.0)";
        if (funcName == "band") return "float_from_bool((" + traverseNode(node->args[0]) + " != 0.0) && (" + traverseNode(node->args[1]) + " != 0.0))";
        if (funcName == "bor") return "float_from_bool((" + traverseNode(node->args[0]) + " != 0.0) || (" + traverseNode(node->args[1]) + " != 0.0))";
        if (funcName == "boolean_and_op_eel") return "boolean_and_op_eel(" + traverseNode(node->args[0]) + ", " + traverseNode(node->args[1]) + ")";
        if (funcName == "boolean_or_op_eel") return "boolean_or_op_eel(" + traverseNode(node->args[0]) + ", " + traverseNode(node->args[1]) + ")";
        if (funcName == "sigmoid_eel") return "sigmoid_eel(" + traverseNode(node->args[0]) + ", " + traverseNode(node->args[1]) + ")";

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

std::string clean_code(const std::string& code) {
    std::string cleaned = code;

    // Remove comments
    size_t pos = 0;
    while ((pos = cleaned.find("//", pos)) != std::string::npos) {
        size_t end = cleaned.find('\n', pos);
        if (end == std::string::npos) end = cleaned.size();
        cleaned.erase(pos, end - pos);
    }

    // Handle line continuations: replace tokens, \n with , (space)
    pos = 0;
    while ((pos = cleaned.find(",\n", pos)) != std::string::npos) {
        cleaned.replace(pos, 2, ", ");
    }

    // Now split into individual statements and ensure ; termination
    std::stringstream ss(cleaned);
    std::string line;
    cleaned.clear();
    while (std::getline(ss, line, '\n')) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty()) continue;

        // Ensure ; at end if not present
        if (line.back() != ';') {
            line += ';';
        }

        cleaned += line + "\n";
    }

    return cleaned;
}

// Helper function to compile a block of statements into a single AST
prjm_eval_exptreenode* compile_statements(projectm_eval_context* ctx, std::string code) {
    code = clean_code(code);
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

std::string generateWaveformGLSL(const libprojectM::PresetFileParser::ValueMap& presetValues) {
    // Extract nWaveMode from preset values, default to 6
    // PresetFileParser lowercases all keys, so we need to look for "nwavemode"
    int nWaveMode = 6;
    auto it = presetValues.find("nwavemode");
    if (it != presetValues.end()) {
        try {
            nWaveMode = std::stoi(it->second);
        } catch (const std::logic_error& e) {
            // ignore and use default
        }
    }

    // Use the WaveModeRenderer strategy to generate the appropriate GLSL
    return WaveModeRenderer::generateWaveformGLSL(nWaveMode, presetValues);
}

std::string translateToGLSL(const std::string& perFrame, const std::string& perPixel, const libprojectM::PresetFileParser::ValueMap& presetValues) {
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
    std::string perPixelGLSL = generator.generate(perPixelAST, perPixelVariableRewrites());

    // Cleanup the combined ASTs
    if(perFrameAST) prjm_eval_destroy_exptreenode(perFrameAST);
    if(perPixelAST) prjm_eval_destroy_exptreenode(perPixelAST);

    projectm_eval_context_destroy(context);

    std::string waveformGLSL = generateWaveformGLSL(presetValues);

    std::string glsl = "#version 330 core\n\n";
    glsl += "out vec4 FragColor;\n\n";
    glsl += "float float_from_bool(bool b) { return b ? 1.0 : 0.0; }\n\n";
    glsl += R"___(
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
)___";
    glsl += "const float EPSILON_EEL = 0.00001;\n";
    glsl += "float sigmoid_eel(float value, float response) {\n";
    glsl += "    float t = 1.0 + exp(-(value) * response);\n";
    glsl += "    return (abs(t) > EPSILON_EEL) ? (1.0 / t) : 0.0;\n";
    glsl += "}\n";
    glsl += "float boolean_and_op_eel(float lhs, float rhs) {\n";
    glsl += "    return (abs(lhs) > EPSILON_EEL && abs(rhs) > EPSILON_EEL) ? 1.0 : 0.0;\n";
    glsl += "}\n";
    glsl += "float boolean_or_op_eel(float lhs, float rhs) {\n";
    glsl += "    return (abs(lhs) > EPSILON_EEL) ? 1.0 : ((abs(rhs) > EPSILON_EEL) ? 1.0 : 0.0);\n";
    glsl += "}\n";
    glsl += "float exec2_helper(float first, float second) {\n";
    glsl += "    return second;\n";
    glsl += "}\n";
    glsl += "float exec3_helper(float first, float second, float third) {\n";
    glsl += "    return third;\n";
    glsl += "}\n";
    glsl += waveformGLSL;
    glsl += "\n// Standard RaymarchVibe uniforms\n";
    glsl += "uniform float iTime;\n";
    glsl += "uniform vec2 iResolution;\n";
    glsl += "uniform float iFps;\n";
    glsl += "uniform float iFrame;\n";
    glsl += "uniform float iProgress;\n";
    glsl += "uniform vec4 iAudioBands;\n";
    glsl += "uniform vec4 iAudioBandsAtt;\n";
    glsl += "uniform sampler2D iChannel0; // Feedback buffer\n";
    glsl += "uniform sampler2D iChannel1;\n";
    glsl += "uniform sampler2D iChannel2;\n";
    glsl += "uniform sampler2D iChannel3;\n\n";
    glsl += "// Preset-specific uniforms with UI annotations\n";
    for (const auto& pair : uniformControls) {
        std::string defaultValue = pair.second.defaultValue;
        std::string sliderMin = pair.second.min;
        std::string sliderMax = pair.second.max;

        float numericDefault = 0.0f;
        bool hasNumericDefault = false;

        if (auto it = presetValues.find(pair.first); it != presetValues.end()) {
            try {
                numericDefault = std::stof(it->second);
                defaultValue = it->second;
                hasNumericDefault = true;
            } catch (const std::logic_error&) {
                // Keep fallback default if preset value is not numeric
            }
        }

        if (!hasNumericDefault) {
            try {
                numericDefault = std::stof(defaultValue);
                hasNumericDefault = true;
            } catch (const std::logic_error&) {
                // Leave numericDefault unused if parsing fails
            }
        }

        if (hasNumericDefault) {
            try {
                float sliderMinNumeric = std::stof(pair.second.min);
                float sliderMaxNumeric = std::stof(pair.second.max);
                if (numericDefault < sliderMinNumeric) {
                    sliderMin = defaultValue;
                }
                if (numericDefault > sliderMaxNumeric) {
                    sliderMax = defaultValue;
                }
            } catch (const std::logic_error&) {
                // Preserve original slider bounds if parsing fails
            }
        }

        glsl += "uniform float u_" + pair.first + " = " + defaultValue + "; // {\"widget\":\"" + pair.second.widget + "\",\"default\":" + defaultValue + ",\"min\":" + sliderMin + ",\"max\":" + sliderMax + ",\"step\":" + pair.second.step + "}\n";
    }
    glsl += "\nvoid main() {\n";
    glsl += "    // Calculate UV coordinates from screen position\n";
    glsl += "    vec2 uv = gl_FragCoord.xy / iResolution.xy;\n\n";
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
    glsl += "    vec4 pixelColor = vec4(0.0, 0.0, 0.0, 0.0);\n";
    glsl += "\n    // Per-frame logic\n";
    glsl += perFrameGLSL;
    glsl += "\n    // Per-pixel logic\n";
    glsl += perPixelGLSL;
    glsl += R"___(
    // Apply coordinate transformations using per-pixel state.
    vec2 pixelCenter = vec2(cx, cy);
    vec2 pixelTranslate = vec2(dx, dy);
    vec2 pixelScale = vec2(sx, sy);
    float pixelZoom = zoom;
    float pixelZoomExp = zoomexp;
    float pixelWarp = warp;
    float pixelRotate = rot;
    float pixelDecay = decay;
    pixelColor = vec4(r, g, b, a);
    vec2 pixelUV = uv;

    vec2 centeredUV = pixelUV - pixelCenter;
    mat2 rotationMatrix = mat2(cos(pixelRotate), -sin(pixelRotate), sin(pixelRotate), cos(pixelRotate));
    centeredUV = rotationMatrix * centeredUV;

    float zoomDenominator = max(0.0001, pow(max(0.0001, pixelZoom), pixelZoomExp));
    vec2 scaleMagnitude = max(abs(pixelScale), vec2(0.0001));
    vec2 scaleSign = vec2(pixelScale.x >= 0.0 ? 1.0 : -1.0, pixelScale.y >= 0.0 ? 1.0 : -1.0);
    vec2 safeScale = scaleSign * scaleMagnitude;
    vec2 scaledUV = centeredUV / safeScale;
    scaledUV /= zoomDenominator;
    scaledUV *= pixelWarp;

    vec2 sampleUV = pixelCenter + scaledUV + pixelTranslate;
    sampleUV = clamp(sampleUV, vec2(0.001), vec2(0.999));

    // Fetch feedback using the transformed UV and apply decay.
    vec4 feedback = texture(iChannel0, sampleUV);
    float decayFactor = clamp(pixelDecay, 0.0, 1.0);
    feedback.rgb *= decayFactor;

    // Blend feedback with per-pixel color output.
    vec4 perPixelColor = clamp(pixelColor, 0.0, 1.0);
    float perPixelAlpha = clamp(perPixelColor.a, 0.0, 1.0);
    vec4 composedColor = mix(feedback, perPixelColor, perPixelAlpha);

    // Preserve existing border tint.
    vec4 border_color = clamp(vec4(ob_r, ob_g, ob_b, ob_a), 0.0, 1.0);
    composedColor = mix(composedColor, border_color, border_color.a);

    // Overlay waveforms.
    vec4 wave_color = clamp(vec4(wave_r, wave_g, wave_b, wave_a), 0.0, 1.0);
    float wave_intensity = draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery);
    composedColor.rgb = mix(composedColor.rgb, wave_color.rgb, clamp(wave_intensity * wave_color.a, 0.0, 1.0));

    FragColor = vec4(clamp(composedColor.rgb, 0.0, 1.0), clamp(composedColor.a, 0.0, 1.0));
}
)___";
    return glsl;
}


bool runSelfTests() {
    projectm_eval_context* context = projectm_eval_context_create(nullptr, nullptr);
    if (!context) {
        std::cerr << "Self-test: failed to create evaluation context." << std::endl;
        return false;
    }

    prjm_eval_exptreenode* perPixelAst = compile_statements(context, "red = min(max(zoomexp, 0.0), 1.0);\nalpha = 1;\n");
    if (!perPixelAst) {
        std::cerr << "Self-test: failed to compile synthetic per-pixel code." << std::endl;
        projectm_eval_context_destroy(context);
        return false;
    }

    GLSLGenerator generator(context);
    std::string perPixelGLSL = generator.generate(perPixelAst, perPixelVariableRewrites());
    bool rewriteOk = perPixelGLSL.find("pixelColor.r") != std::string::npos && perPixelGLSL.find("/* unknown node */") == std::string::npos;

    prjm_eval_destroy_exptreenode(perPixelAst);
    projectm_eval_context_destroy(context);

    if (!rewriteOk) {
        std::cerr << "Self-test: per-pixel variable rewrite failed." << std::endl;
        return false;
    }

    libprojectM::PresetFileParser parser;
    if (!parser.Read("baked.milk")) {
        std::cerr << "Self-test: unable to read baked.milk preset." << std::endl;
        return false;
    }

    std::string bakedGLSL = translateToGLSL(parser.GetCode("per_frame_"), parser.GetCode("per_pixel_"), parser.PresetValues());
    if (bakedGLSL.find("warp = 1.42") == std::string::npos || bakedGLSL.find("/* unknown node */") != std::string::npos) {
        std::cerr << "Self-test: baked.milk translation missing expected per-pixel output." << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    std::setlocale(LC_NUMERIC, "C");
    if (argc == 2 && std::string(argv[1]) == "--self-test") {
        bool success = runSelfTests();
        if (success) {
            std::cout << "Self-tests passed" << std::endl;
            return 0;
        }
        std::cerr << "Self-tests failed" << std::endl;
        return 1;
    }
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


    std::string glsl = translateToGLSL(perFrameCode, perPixelCode, parser.PresetValues());
    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Error: Could not open output file for writing: " << outputFile << "\n";
        return 1;
    }
    out << glsl;
    std::cout << "Successfully converted " << inputFile << " to " << outputFile << "\n";
    return 0;
}
