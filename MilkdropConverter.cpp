#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <regex>
#include <sstream>
#include <algorithm>

#include "PresetFileParser.hpp"

// A map of MilkDrop built-in variables to their GLSL equivalents.
const std::unordered_map<std::string, std::string> milkToGLSL = {
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

// All known variables that are not user-defined.
const std::vector<std::string> knownVars = {
    "time", "fps", "frame", "progress", "bass", "mid", "treb", "bass_att", "mid_att", "treb_att",
    "x", "y", "rad", "ang", "meshx", "meshy", "pixelsx", "pixelsy", "aspectx", "aspecty",
};

// Function to find user-defined variables by parsing assignment statements.
std::set<std::string> findUserVars(const std::string& code) {
    std::set<std::string> userVars;
    std::regex assignmentRegex("([a-zA-Z_][a-zA-Z0-9_]*)\\s*=");

    auto words_begin = std::sregex_iterator(code.begin(), code.end(), assignmentRegex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::string varName = (*i)[1].str();

        bool isKnown = (milkToGLSL.count(varName) > 0 ||
                        uniformControls.count(varName) > 0 ||
                        std::find(knownVars.begin(), knownVars.end(), varName) != knownVars.end() ||
                        std::regex_match(varName, std::regex("q[1-9][0-9]?|t[1-8]")));

        if (!isKnown) {
            userVars.insert(varName);
        }
    }
    return userVars;
}


// Function to translate a single MilkDrop expression line to GLSL
std::string translateExpression(const std::string& expression) {
    std::string translated = expression;

    // Replace variables
    for (const auto& pair : milkToGLSL) {
        translated = std::regex_replace(translated, std::regex("\\b" + pair.first + "\\b"), pair.second);
    }

    // Naive function replacement
    translated = std::regex_replace(translated, std::regex("\\bsin\\b"), "sin");
    translated = std::regex_replace(translated, std::regex("\\bcos\\b"), "cos");
    translated = std::regex_replace(translated, std::regex("\\btan\\b"), "tan");
    translated = std::regex_replace(translated, std::regex("\\basin\\b"), "asin");
    translated = std::regex_replace(translated, std::regex("\\bacos\\b"), "acos");
    translated = std::regex_replace(translated, std::regex("\\batan\\b"), "atan");
    translated = std::regex_replace(translated, std::regex("\\babs\\b"), "abs");
    translated = std::regex_replace(translated, std::regex("\\bsqrt\\b"), "sqrt");
    translated = std::regex_replace(translated, std::regex("\\bpow\\b"), "pow");
    translated = std::regex_replace(translated, std::regex("\\blog\\b"), "log");
    translated = std::regex_replace(translated, std::regex("\\blog10\\b"), "log10");
    translated = std::regex_replace(translated, std::regex("\\bmin\\b"), "min");
    translated = std::regex_replace(translated, std::regex("\\bmax\\b"), "max");
    translated = std::regex_replace(translated, std::regex("\\bsign\\b"), "sign");
    translated = std::regex_replace(translated, std::regex("\\bint\\b"), "int");

    // Translate if(cond, then, else) to mix(else, then, step(0.0001, cond))
    std::regex ifRegex("if\\s*\\(([^,]+),([^,]+),([^\\)]+)\\)");
    translated = std::regex_replace(translated, ifRegex, "mix($3, $2, step(0.0001, $1))");

    // Translate sqr(x) to (x*x) for efficiency and correctness
    std::regex sqrRegex("sqr\\s*\\(([^\\)]+)\\)");
    translated = std::regex_replace(translated, sqrRegex, "(($1)*($1))");

    // Ensure all integer literals are converted to floats, e.g. 2 -> 2.0
    translated = std::regex_replace(translated, std::regex("(\\b\\d+\\b)(?!\\.)"), "$1.0");

    return translated;
}

// Function to translate MilkDrop expressions to GLSL
std::string translateToGLSL(const std::string& perFrame, const std::string& perPixel) {
    auto userVars = findUserVars(perFrame + perPixel);

    std::string glsl = "#version 330 core\n\n";
    glsl += "out vec4 FragColor;\n";
    glsl += "in vec2 uv;\n\n";

    // Standard uniforms
    glsl += "// Standard RaymarchVibe uniforms\n";
    glsl += "uniform float iTime;\n";
    glsl += "uniform vec2 iResolution;\n";
    glsl += "uniform float iFps;\n";
    glsl += "uniform float iFrame;\n";
    glsl += "uniform float iProgress;\n";
    glsl += "uniform vec4 iAudioBands;\n";
    glsl += "uniform vec4 iAudioBandsAtt;\n\n";

    // Annotated uniforms for UI controls
    glsl += "// Preset-specific uniforms with UI annotations\n";
    for(const auto& pair : uniformControls) {
        glsl += "uniform float u_" + pair.first + "; // {";
        glsl += "\"widget\":\"" + pair.second.widget + "\",";
        glsl += "\"default\":" + pair.second.defaultValue + ",";
        glsl += "\"min\":" + pair.second.min + ",";
        glsl += "\"max\":" + pair.second.max + ",";
        glsl += "\"step\":" + pair.second.step;
        glsl += "}\n";
    }
    glsl += "\n";

    glsl += "void main() {\n";

    // Declare and initialize local variables from uniforms
    glsl += "    // Initialize local variables from uniforms\n";
    for(const auto& pair : uniformControls) {
        glsl += "    float " + pair.first + " = u_" + pair.first + ";\n";
    }
    glsl += "\n";

    // Declare q, t, and user-defined variables
    glsl += "    // State variables\n";
    for (int i = 1; i <= 32; ++i) {
        glsl += "    float q" + std::to_string(i) + " = 0.0;\n";
    }
    for (int i = 1; i <= 8; ++i) {
        glsl += "    float t" + std::to_string(i) + " = 0.0;\n";
    }
    if (!userVars.empty()) {
        for (const auto& var : userVars) {
            glsl += "    float " + var + " = 0.0;\n";
        }
    }
    glsl += "\n";

    // Add translated per-frame code
    glsl += "    // Per-frame logic\n";
    std::stringstream perFrameStream(perFrame);
    std::string line;
    while (std::getline(perFrameStream, line, ';')) {
        line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");
        if (!line.empty()) {
            glsl += "    " + translateExpression(line) + ";\n";
        }
    }

    glsl += "\n    // Per-pixel logic\n";
    std::stringstream perPixelStream(perPixel);
    while (std::getline(perPixelStream, line, ';')) {
        line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");
        if (!line.empty()) {
            glsl += "    " + translateExpression(line) + ";\n";
        }
    }

    glsl += "\n";
    glsl += "    FragColor = vec4(r, g, b, a);\n";
    glsl += "}\n";

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
        std::cerr << "Error: Could not read or parse the input file: " << inputFile << "\n";
        return 1;
    }

    std::string perFrame = parser.GetCode("per_frame_");
    std::string perPixel = parser.GetCode("per_pixel_");

    std::string glsl = translateToGLSL(perFrame, perPixel);

    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Error: Could not open the output file for writing: " << outputFile << "\n";
        return 1;
    }

    out << glsl;

    std::cout << "Successfully converted " << inputFile << " to " << outputFile << "\n";

    return 0;
}