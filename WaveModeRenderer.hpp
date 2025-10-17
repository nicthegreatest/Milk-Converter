#pragma once

#include <map>
#include <memory>
#include <string>

/**
 * @brief Abstract strategy for generating GLSL waveform rendering code.
 *
 * Concrete subclasses implement vertex-generation helpers and draw functions
 * for specific MilkDrop wave modes. The factory method selects the correct
 * strategy based on the preset's nWaveMode value.
 */
class WaveModeRenderer {
public:
    explicit WaveModeRenderer(const std::map<std::string, std::string>& presetValues)
        : m_presetValues(presetValues) {}
    virtual ~WaveModeRenderer() = default;

    /// Optional helpers shared across modes (defaults to the common helpers).
    virtual std::string helperFunctions() const;

    /// Mode-specific vertex generation helpers (may be empty).
    virtual std::string vertexFunction() const = 0;

    /// Mode-specific draw function implementation. Must declare draw_wave().
    virtual std::string drawFunction() const = 0;

    /// Generate the calling code pattern for this mode's draw_wave function.
    virtual std::string callPattern() const = 0;

    /// Generate the GLSL snippet for the requested wave mode.
    static std::string generateWaveformGLSL(int nWaveMode, const std::map<std::string, std::string>& presetValues);

    /// Generate the draw_wave call pattern for the requested wave mode.
    static std::string generateCallPattern(int nWaveMode, const std::map<std::string, std::string>& presetValues);

protected:
    /// Factory method returning the appropriate renderer implementation.
    static std::unique_ptr<WaveModeRenderer> create(int nWaveMode, const std::map<std::string, std::string>& presetValues);

    /// Shared helper functions available to all strategies.
    static std::string generateCommonHelpers();

    /// Fallback GLSL when a mode is unsupported.
    static std::string generateFallback();

    float presetFloat(const std::string& key, float fallback) const;
    int presetInt(const std::string& key, int fallback) const;

    /// Preset values for tuning parameters
    const std::map<std::string, std::string>& m_presetValues;
};
