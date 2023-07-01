// svgtheme.hpp - lightweight SVG themeing based on nanosvg,
// designed primarily for VCV Rack.
//
// See the end of this file for copyright and license information.

// VCV Rack-specific functionality is provided by also including 
// `svt_rack.hpp` as shown in the followong example.
//
// One source file in your project must contain:
//
// ```cpp
// #define IMPLEMENT_SVG_THEME
// #include "svg_theme.hpp" // this file
// #include "svt_rack.hpp" // VCV-Rack helpers
// ```

#ifndef SVG_THEME_H
#define SVG_THEME_H
#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <nanosvg.h>
#include <jansson.h>

namespace svg_theme {

// nanosvg colors are 8-bit (0-255) abgr packed into an unsigned int.
typedef unsigned int PackedColor;

enum Severity { Info, Warn, Error, Critical };
const char * SeverityName(Severity sev);

enum ErrorCode {
    Unspecified                  = 0,
    NoError                      = 1,
    CannotOpenJsonFile           = 2,
    JsonParseFailed              = 3,
    ArrayExpected                = 4,
    ObjectExpected               = 5,
    ObjectOrStringExpected       = 6,
    StringExpected               = 7,
    NumberExpected               = 8,
    IntegerExpected              = 9,
    NameExpected                 = 10,
    ThemeExpected                = 11,
    InvalidHexColor              = 12,
    OneOfColorOrGradient         = 13,
    TwoGradientStopsMax          = 14,
    GradientStopIndexZeroOrOne   = 15,
    GradientStopNotPresent       = 16,
    RemovingGradientNotSupported = 17,
    GradientNotPresent           = 18,
};

// logging callback function you provide.
typedef std::function<void(Severity severity, ErrorCode code, std::string info)> LogCallback;

struct GradientStop {
    int index = -1;
    float offset = 0.f;
    PackedColor color = 0;
 
    GradientStop() {}
    GradientStop(int i, float off, PackedColor co)  : index(i), offset(off), color(co) {}
};

struct Gradient {
    int nstops = 0;
    GradientStop stops[2];
};

enum class PaintKind { Unset, Color, Gradient, None };
class Paint {
    PaintKind kind = PaintKind::Unset;
    union {
        PackedColor color;
        Gradient gradient;
    };

public:
    Paint() {}
    Paint(PackedColor color) { setColor(color); }
    Paint(const Gradient& gradient) { setGradient(gradient); }

    PaintKind Kind() { return kind; }
    void setColor(PackedColor new_color) {
        kind = PaintKind::Color;
        color = new_color;
    }
    void setGradient(const Gradient& g) {
        kind = PaintKind::Gradient;
        gradient = g;
    }
    void setNone() {
        kind = PaintKind::None;
    }
    bool isColor() { return kind == PaintKind::Color; }
    bool isGradient() { return kind == PaintKind::Gradient; }
    bool isNone()  { return kind == PaintKind::None; }
    PackedColor getColor() { return isColor() ? color : 0; }
    const Gradient* getGradient() { return isGradient() ? &gradient : nullptr; }
    bool isApplicable() { return kind != PaintKind::Unset; }
};

struct Style {
    Paint fill;
    Paint stroke;
	float opacity = 1.f;
	float stroke_width = 1.f;
    bool apply_stroke_width = false;
    bool apply_opacity = false;

    void setFill(Paint paint) { fill = paint; }
    void setStroke(Paint paint) { stroke = paint; }
    void setOpacity(float alpha) {
        opacity = alpha;
        apply_opacity = true;
    }
    void setStrokeWidth(float width) {
        stroke_width = width;
        apply_stroke_width = true;
    }
    bool isApplyFill() { return fill.isApplicable(); }
    bool isApplyStroke() { return stroke.isApplicable(); }
    bool isApplyOpacity() { return apply_opacity; }
    bool isApplyStrokeWidth() { return apply_stroke_width; }
};

struct Theme {
    std::string name;
    std::unordered_map<std::string, std::shared_ptr<Style>> styles;

    std::shared_ptr<Style> getStyle(std::string name) {
        auto found = styles.find(name);
        if (found != styles.end()) {
            return found->second;
        }
        return nullptr;
    }
};


class SvgThemes
{
public:

    // Set a logging callback to receive more detailed information, warnings,
    // and errors when working with svg themes.
    void setLog(LogCallback log) { this->log = log; }

    // load themes from the specified file.
    bool load(const std::string& filename);

    // true if any themes are available after calling load.
    bool isLoaded() { return !themes.empty(); }

    // Get a theme by name
    std::shared_ptr<Theme> getTheme(const std::string& name);

    // Apply the theme to an NSVGImage*
    // return true if the SVG was modified. 
    // You must send a Dirty event to any widget where applyTheme to any of it's component SVGs returns true.
    bool applyTheme(std::shared_ptr<Theme> theme, NSVGimage* svg);

    // Get a list of themes defined in the style sheet
    std::vector<std::string> getThemeNames()
    {
        std::vector<std::string> result;
        for (auto theme: themes) {
            result.push_back(theme->name);
        }
        return result;
    }

private:

    static void LogNothing(Severity severity, ErrorCode code, std::string info) {}

    std::vector<std::shared_ptr<Theme>> themes;
    LogCallback log = LogNothing;

    void logInfo(std::string info) {
        log(Severity::Info, ErrorCode::NoError, info);
    }
    void logError(ErrorCode code, std::string info) {
        log(Severity::Error, code, info);
    }
    void logWarning(ErrorCode code, std::string info) {
        log(Severity::Warn, code, info);
    }
    bool requireValidHexColor(std::string hex, const char * name);
    bool requireArray(json_t* j, const char * name);
    bool requireObject(json_t* j, const char * name);
    bool requireObjectOrString(json_t* j, const char * name);
    bool requireString(json_t* j, const char * name);
    bool requireNumber(json_t* j, const char * name);
    bool requireInteger(json_t* j, const char * name);

    bool parseFill(json_t* root, std::shared_ptr<Style>);
    bool parseStroke(json_t* root, std::shared_ptr<Style>);
    bool parseOpacity(json_t* root, std::shared_ptr<Style>);
    bool parseStyle(const char * name, json_t* root, std::shared_ptr<Theme> theme);
    bool parseTheme(json_t* root, std::shared_ptr<Theme> theme);
    bool parseGradient(json_t* root, Gradient& gradient);

    bool applyPaint(std::string tag, NSVGpaint & target, Paint& source);
    bool applyStroke(std::string tag, NSVGshape* shape, std::shared_ptr<Style> style);
    bool applyFill(std::string tag, NSVGshape* shape, std::shared_ptr<Style> style);

};

// Widgets that support theming should implement IApplyTheme.
//
// IApplyTheme is what enables the VCV Rack helper ApplyChildrenTheme to update 
// all your UI to a new theme in one line of code.
//
// The modified flag returned by themes.applyTheme(theme, ...) should be 
// propagated back to the caller, so it can can initiate the appropriate
// Dirty event as needed.
//
// Here's the complete example implementation of a themable SVG screw (not 
// including the svg and the theme). 
// See the Demo to see all the bits and pieces come together.
//
// ```cpp
// struct ThemeScrew : app::SvgScrew, svg_theme::IApplyTheme {
//     ThemeScrew() {
//         setSvg(Svg::load(asset::plugin(pluginInstance, "res/Screw.svg")));
//     }
//     // implement IApplyTheme
//     bool applyTheme(svg_theme::SvgThemes& themes, std::shared_ptr<svg_theme::Theme> theme) override
//     {
//         return themes.applyTheme(theme, sw->svg->handle);
//     }
// };
// ```
//
struct IApplyTheme
{
    virtual bool applyTheme(svg_theme::SvgThemes& themes, std::shared_ptr<svg_theme::Theme> theme) = 0;
};

// Implement IThemeHolder to enable the AppendThemeMenu helper from the
// VCV Rack helpers in `svt_rack.h`.
// This is usually most conveniently implemented by your module widget,
// as it is in the Demo.
struct IThemeHolder
{
    virtual std::string getTheme() = 0;
    virtual void setTheme(std::string theme_name) = 0;
};

// ============================================================================

#ifdef IMPLEMENT_SVG_THEME

const char * SeverityName(Severity sev) {
    switch (sev) {
        case Severity::Info: return "Info";
        case Severity::Warn: return "Warn";
        case Severity::Error: return "Error";
        case Severity::Critical: return "Critical";
    }
    return "[unknown]";
}

std::string GetTag(NSVGshape* shape)
{
    if (!shape) return "";
    if (!shape->id[0]) return "";
    auto id = std::string(shape->id);
    auto dashes = id.rfind("--");
    if (dashes != std::string::npos) {
        id = id.substr(dashes + 2);
    }
    return id;
}

inline int hex_value(unsigned char ch) {
    if (ch > 'f' || ch < '0') { return -1; }
    if (ch <= '9') { return ch & 0xF; }
    if (ch < 'A') { return -1; }
    if (ch < 'G') { return 10 + ch - 'A'; }
    if (ch < 'a') { return -1; }
    return 10 + ch - 'a';
}

inline PackedColor PackRGB(unsigned int r, unsigned int g, unsigned int b) {
    return r | (g << 8) | (b << 16) | (255 << 24);
}
inline PackedColor PackRGBA(unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
    return r | (g << 8) | (b << 16) | (a << 24);
}

bool isValidHexColor(std::string hex) {
    switch (hex.size()) {
        case 1 + 3: 
        case 1 + 4:
        case 1 + 6: 
        case 1 + 8: break;
        default: return false;
    }
    if (*hex.begin() != '#') return false;
    return std::string::npos == hex.find_first_not_of("0123456789ABCDEFabcdef", 1);
}

std::vector<unsigned char> ParseHex(std::string hex) {
    std::vector<unsigned char> result;

    // Color representations:
    //  short: #rgb, #rgba
    //  long: #rrggbb, #rrggbbaa
    bool long_hex = true;
    switch (hex.size()) {
        case 1 + 3: 
        case 1 + 4: long_hex = false; break;
        case 1 + 6: 
        case 1 + 8: long_hex = true; break;
        default: return result;
    }

    enum State { Hex = -1, R1, R2, G1, G2, B1, B2, A1, A2, End };
    int state = State::Hex;
    int value = 0;
    for (unsigned char ch: hex) {
        if (state == State::Hex) {
            if (ch == '#') {
                ++state;
            } else {
                return result;
            }
        } else {
            auto nibble = hex_value(ch);
            if (-1 == nibble) {
                result.clear();
                return result;
            }
            if (state & 1) { // odd
                value |= nibble;
                result.push_back(value);
                value = 0;
                ++state;
            } else { // even
                value = nibble << 4;
                if (long_hex) {
                    ++state;
                } else {
                    result.push_back(value);
                    value = 0;
                    state += 2;
                }
            }
        }
        if (state >= State::End) {
            break;
        }
    }
    return result;
}

std::string format_string(const char *fmt, ...)
{
    const int len = 256;
    std::string s(len, '\0');
    va_list args;
    va_start(args, fmt);
    auto r = std::vsnprintf(&(*s.begin()), len + 1, fmt, args);
    return r < 0 ? "??" : s;
}

const PackedColor OPAQUE_BLACK = 255 << 24;

std::shared_ptr<Theme> SvgThemes::getTheme(const std::string& name)
{
    auto r = std::find_if(themes.begin(), themes.end(), [=](const std::shared_ptr<Theme> theme) {
        return 0 == theme->name.compare(name);
    });
    return r == themes.end() ? nullptr : *r;
}

bool SvgThemes::requireValidHexColor(std::string hex, const char * name)
{
    if (isValidHexColor(hex)) return true;
    logError(ErrorCode::InvalidHexColor, format_string("'%s': invalid hex color: '%s'", name, hex.c_str()));
    return false;
}
bool SvgThemes::requireArray(json_t* j, const char * name)
{
    if (json_is_array(j)) return true;
    logError(ErrorCode::ArrayExpected, format_string("'%s': array expected", name));
    return false;
}
bool SvgThemes::requireObject(json_t* j, const char * name)
{
    if (json_is_object(j)) return true;
    logError(ErrorCode::ObjectExpected, format_string("'%s': object expected", name));
    return false;
}
bool SvgThemes::requireObjectOrString(json_t* j, const char * name)
{
    if (json_is_object(j) || json_is_string(j)) return true;
    logError(ErrorCode::ObjectOrStringExpected, format_string("'%s': Object or string expected", name));
    return false;
}
bool SvgThemes::requireString(json_t* j, const char * name)
{
    if (json_is_string(j)) return true;
    logError(ErrorCode::StringExpected, format_string("'%s': String expected", name));
    return false;
}
bool SvgThemes::requireNumber(json_t* j, const char * name)
{
    if (json_is_number(j)) return true;
    logError(ErrorCode::NumberExpected, format_string("'%s': Number expected", name));
    return false;
}
bool SvgThemes::requireInteger(json_t* j, const char * name)
{
    if (json_is_integer(j)) return true;
    logError(ErrorCode::IntegerExpected, format_string("'%s': Integer expected", name));
    return false;
}

PackedColor parseColor(const char * text)
{
    auto parts = ParseHex(text);
    if (parts.size() == 3) {
        return PackRGB(parts[0], parts[1], parts[2]);
    }
    if (parts.size() == 4) {
        return PackRGBA(parts[0], parts[1], parts[2], parts[3]);
    }
    // user error coding the color
    return OPAQUE_BLACK;
}

float getNumber(json_t * j)
{
    if (json_is_real(j)) return json_real_value(j);
    if (json_is_number(j)) return json_number_value(j);
    if (json_is_integer(j)) return static_cast<float>(json_integer_value(j));
    return 1.f;
}

bool SvgThemes::parseOpacity(json_t * root, std::shared_ptr<Style> style)
{
    auto oopacity = json_object_get(root, "opacity");
    if (oopacity) {
        if (!requireNumber(oopacity, "opacity")) return false;
        style->setOpacity(std::max(0.f, std::min(1.f, getNumber(oopacity))));
    }
    return true;
}

bool SvgThemes::parseGradient(json_t* ogradient, Gradient& gradient)
{
    bool ok = true;
    gradient.nstops = 0;
    if (ogradient) {
        if (!requireArray(ogradient, "gradient")) return false;

        int index;
        PackedColor color;
        float offset;

        json_t * item; size_t n;
        json_array_foreach(ogradient, n, item) {
            if (n > 1) {
                logError(ErrorCode::TwoGradientStopsMax, "A maximum of two gradient stops is allowed");
                return false;
            }
            auto oindex = json_object_get(item, "index");
            if (oindex) {
                if (requireInteger(oindex, "index")) {
                    index = json_integer_value(oindex);
                    if (!(0 == index || 1 == index)) {
                        logError(ErrorCode::GradientStopIndexZeroOrOne, "Gradient stop index must be 0 or 1");
                        ok = false;
                    } 
                } else {
                    index = 0;
                    ok = false;
                }
            }
            
            auto ocolor = json_object_get(item, "color");
            if (ocolor) {
                if (requireString(ocolor, "color")) {
                    auto hex = json_string_value(ocolor);
                    if (requireValidHexColor(hex, "color")) {
                        color = parseColor(hex);
                    } else {
                        color = 0;
                        ok = false;
                    }
                } else {
                    color = 0;
                    ok = false;
                }
            }
            
            auto ooffset = json_object_get(item, "offset");
            if (ooffset) {
                if (requireNumber(ooffset, "offset")) {
                    offset = getNumber(ooffset);
                } else {
                    offset = 0.f;
                    ok = false;
                }
            }

            if (ok) {
                gradient.stops[index] = GradientStop(index, offset, color);
            }
        }
        if (ok) {
            int count = 0;
            if (gradient.stops[0].index >= 0) ++count;
            if (gradient.stops[1].index >= 0) ++count;
            gradient.nstops = count;
        }
    }
    return ok;
}

bool SvgThemes::parseFill(json_t* root, std::shared_ptr<Style> style)
{
    auto ofill = json_object_get(root, "fill");
    if (!ofill) return true;
    if (!requireObjectOrString(ofill, "fill")) return false;
    if (json_is_string(ofill)) {
        auto value = json_string_value(ofill);
        if (0 == strcmp(value, "none")) {
            style->fill.setNone();
        } else {
            if (!requireValidHexColor(value, "fill")) return false;
            style->fill.setColor(parseColor(value));
        }
    } else {
        auto ocolor = json_object_get(ofill, "color");
        if (ocolor) {
            if (!requireString(ocolor, "color")) return false;
            auto hex = json_string_value(ocolor);
            if (!requireValidHexColor(hex, "color")) return false;
            style->fill.setColor(parseColor(hex));
        }
        auto ogradient = json_object_get(ofill, "gradient");
        if (ogradient) {
            if (ocolor) {
                logError(ErrorCode::OneOfColorOrGradient, "'fill': Only one of 'color' or 'gradient' allowed");
                return false;
            }
            Gradient gradient;
            if (parseGradient(ogradient, gradient) && gradient.nstops > 0) {
                style->fill.setGradient(gradient);
            }
        }
    }
    return true;
}

bool SvgThemes::parseStroke(json_t* root, std::shared_ptr<Style> style)
{
    auto ostroke = json_object_get(root, "stroke");
    if (ostroke) {
        if (!requireObjectOrString(ostroke, "stroke")) return false;

        if (json_is_string(ostroke)) {
            auto value = json_string_value(ostroke);
            if (0 == strcmp(value, "none")) {
                style->stroke.setNone();
            } else {
                if (!requireValidHexColor(value, "stroke")) return false;
                style->stroke.setColor(parseColor(value));
            }
        } else {
            auto owidth = json_object_get(ostroke, "width");
            if (owidth) {
                if (!requireNumber(owidth, "width")) return false;
                style->setStrokeWidth(getNumber(owidth));
            }

            auto ocolor = json_object_get(ostroke, "color");
            if (ocolor) {
                if (!requireString(ocolor, "color")) return false;
                auto hex = json_string_value(ocolor);
                if (!requireValidHexColor(hex, "color")) return false;
                style->stroke.setColor(parseColor(hex));
            }

            auto ogradient = json_object_get(ostroke, "gradient");
            if (ogradient) {
                if (ocolor) {
                    logError(ErrorCode::OneOfColorOrGradient, "'stroke': Only one of 'color' or 'gradient' allowed");
                    return false;
                }
                Gradient gradient;
                if (parseGradient(ogradient, gradient) && gradient.nstops > 0) {
                    style->stroke.setGradient(gradient);
                }
            }
        }
    }
    return true;
}

bool SvgThemes::parseStyle(const char * name, json_t* root, std::shared_ptr<Theme> theme)
{
    logInfo(format_string("Parsing '%s'", name));
    auto style = std::make_shared<Style>();
    if (!parseFill(root, style)) return false;
    if (!parseStroke(root, style)) return false;
    if (!parseOpacity(root, style)) return false;
    theme->styles[name] = style;
    return true;
}

bool SvgThemes::parseTheme(json_t* root, std::shared_ptr<Theme> theme)
{
    void * n = nullptr;
    const char* key = nullptr;
    json_t* j = nullptr;

    json_object_foreach_safe(root, n, key, j) {
        if (json_is_object(j)) {
            if (!parseStyle(key, j, theme)) return false;
        } else {
            logError(ErrorCode::ObjectExpected, format_string("Theme '%s': Each style must be an object", theme->name.c_str()));
            return false;
        }
    }
    return true;
}

bool SvgThemes::load(const std::string& filename)
{
    bool ok = true;
	FILE* file = std::fopen(filename.c_str(), "r");
	if (!file) {
        log(Severity::Critical, ErrorCode::CannotOpenJsonFile, filename.c_str());
        return false;
    }

	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root)
    {
        logError(ErrorCode::JsonParseFailed, format_string("Parse error - %s %d:%d %s",
            error.source, error.line, error.column, error.text));
        std::fclose(file);
        return false;
    }

    if (json_is_array(root)) {
        json_t * item; size_t n;
        json_array_foreach(root, n, item) {
            if (json_is_object(item)) {
                json_t* j = json_object_get(item, "name");
                const char * name = nullptr;
                if (j && json_is_string(j)) {
                    name = json_string_value(j);
                }
                if (name && *name) {
                    j = json_object_get(item, "theme");
                    if (j && json_is_object(j)) {
                        logInfo(format_string("Parsing theme '%s'", name));
                        auto theme = std::make_shared<Theme>();
                        theme->name = name;
                        if (parseTheme(j, theme)) {
                            themes.push_back(theme);
                        } else {
                            ok = false;
                            break;
                        }
                    } else {
                        logError(ErrorCode::ThemeExpected, "Expected a 'theme' object");
                        ok = false;
                        break;
                    }
                } else {
                    logError(ErrorCode::NameExpected, "Each theme must have a non-empty name");
                    ok = false;
                    break;
                }
            } else {
                logError(ErrorCode::ObjectExpected, "Expected a 'theme' object");
                ok = false;
                break;
            }
        }
    } else {
        logError(ErrorCode::ArrayExpected, "The top level element must be an array");
        ok = false;
    }

	json_decref(root);
    std::fclose(file);
    if (!ok) {
        themes.clear();
    }
    return ok;
}

bool SvgThemes::applyPaint(std::string tag, NSVGpaint & target, Paint& source)
{
    if (!source.isApplicable()) return false;

    switch (source.Kind()) {
        case PaintKind::None:
            if (target.type != NSVG_PAINT_NONE) {
                if ((target.type == NSVG_PAINT_RADIAL_GRADIENT)
                    || (target.type == NSVG_PAINT_LINEAR_GRADIENT)) {
                    logWarning(ErrorCode::RemovingGradientNotSupported, 
                        format_string("'%s': Removing gradient not supported (leaks memory)", tag.c_str()));
                    return false;
                }
                target.type = NSVG_PAINT_NONE;
                return true;
            }
            break;

        case PaintKind::Color: {
                auto source_color = source.getColor();
                if ((target.type != NSVG_PAINT_COLOR) || (target.color != source_color)) {
                    if ((target.type == NSVG_PAINT_RADIAL_GRADIENT)
                        || (target.type == NSVG_PAINT_LINEAR_GRADIENT)) {
                        logWarning(ErrorCode::RemovingGradientNotSupported, 
                             format_string("'%s': Removing gradient not supported (leaks memory)", tag.c_str()));
                        return false;
                    }
                    target.type = NSVG_PAINT_COLOR;
                    target.color = source_color;
                    return true;
                }
            }
            break;

        case PaintKind::Gradient: {
                auto gradient = source.getGradient();
                if (!gradient) return false; // unexpected - defensive

                if (!((target.type == NSVG_PAINT_RADIAL_GRADIENT)
                    || (target.type == NSVG_PAINT_LINEAR_GRADIENT))) {
                    logWarning(ErrorCode::GradientNotPresent, 
                        format_string("'%s': Skipping SVG element without a gradient", tag.c_str()));
                    return false;
                }

                bool changed = false;
                for (auto n = 0; n < gradient->nstops; ++n) {
                    const GradientStop& stop = gradient->stops[n];
                    if (stop.index > target.gradient->nstops) {
                        logWarning(ErrorCode::GradientStopNotPresent, 
                            format_string("'%s': Gradient stop %d not present in SVG", tag.c_str()));
                    } else {
                        NSVGgradientStop& target_stop = target.gradient->stops[stop.index];
                        if (target_stop.offset != stop.offset) {
                            target_stop.offset = stop.offset;
                            changed = true;
                        }
                        if (target_stop.color != stop.color) {
                            target_stop.color = stop.color;
                            changed = true;
                        }
                    }
                }
                return changed;
            }

        default:
            return false;
    }
    return false;
}


bool SvgThemes::applyFill(std::string tag, NSVGshape* shape, std::shared_ptr<Style> style)
{
    return style->isApplyFill() ? applyPaint(tag, shape->fill, style->fill) : false;
}

bool SvgThemes::applyStroke(std::string tag, NSVGshape* shape, std::shared_ptr<Style> style)
{
    return style->isApplyStroke() ? applyPaint(tag, shape->stroke, style->stroke) : false;
}

bool SvgThemes::applyTheme(std::shared_ptr<Theme> theme, NSVGimage* svg)
{
    if (!theme || !svg || !svg->shapes) return false;
    bool modified = false;
    for (NSVGshape* shape = svg->shapes; nullptr != shape; shape = shape->next) {
        std::string tag = GetTag(shape);
        if (tag.empty()) continue;
        auto style = theme->getStyle(tag);
        if (style) {
            if (style->isApplyOpacity() && (shape->opacity != style->opacity)) {
                shape->opacity = style->opacity;
                modified = true;
            }
            if (style->isApplyStrokeWidth() && (shape->strokeWidth != style->stroke_width)) {
                shape->strokeWidth = style->stroke_width;
                modified = true;
            }
            if (applyFill(tag, shape, style)) {
                modified = true;
            }
            if (applyStroke(tag, shape, style)) {
                modified = true;
            }
        }
    }
    return modified;
}

#endif // IMPLEMENT_SVG_THEME
} // namespace svg_theme
#endif //SVG_THEME_H

/* Copyright (C) 2023 Paul Chase Dempsey pcdempsey@live.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * This software depends on and extends nanosvg.
 * nanosvg, Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 * See nanosvg.h for license information.
 * 
 * This software depends on Jansson for JSON deserialization
 * Jansson, Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org> licensed under the MIT license.
 *
 */
