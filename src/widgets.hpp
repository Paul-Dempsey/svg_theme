#include <rack.hpp>
#include "plugin.hpp"
#include "../svgtheme.hpp"

using namespace rack;

// A Themed screw, based on the standard VCV Rack screw.
// This is the typical subclass logic to provide an alternate SVG for a widget,
// plus the complete implementation of IApplyTheme so that the module widget can
// call a single helper to update the entire UI of the module with a new theme.
//
struct ThemeScrew : app::SvgScrew, svg_theme::IApplyTheme
{
    ThemeScrew()
    {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/Screw.svg")));
    }

    // implement IApplyTheme
    bool applyTheme(svg_theme::SvgThemes& themes, std::shared_ptr<svg_theme::Theme> theme) override
    {
        return themes.applyTheme(theme, sw->svg->handle);
    }
};
