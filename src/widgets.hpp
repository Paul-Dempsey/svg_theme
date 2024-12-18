#include "plugin.hpp"
#include "../svgtheme.hpp"

using namespace svg_theme;

// This file contains themed widgets subclassed from Rack widgets.
// Rack widgets load their SVGs in the default constructor, 
// which is the constructor used by the Rack widget creation templates
// such as CreateInputCentered or CreateParamWidget.
// A default constructor doesn't let us access the theme engine or know what theme to 
// apply via arguments, so these must be created using the createThemedWidget templates
// defined in svt_rack.h.

// A Themed screw, based on the standard Rack screw.
struct ThemeScrew : app::SvgScrew, IApplyTheme
{
    // IApplyTheme
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        setSvg(theme_engine.loadSvg(asset::plugin(pluginInstance, "res/Screw.svg"), theme));
        return true;
    }
};

// A themed Port
struct ThemePort : app::SvgPort, IApplyTheme
{
    // IApplyTheme
    bool applyTheme(SvgThemeEngine& theme_engine, std::shared_ptr<SvgTheme> theme) override
    {
        setSvg(theme_engine.loadSvg(asset::plugin(pluginInstance, "res/Port.svg"), theme));
        return true;
    }
};

struct ThemeSwitchV2 : app::SvgSwitch, IApplyTheme
{
    ThemeSwitchV2()
    {
        shadow->opacity = 0.f; // hide the default shadow, like the Rack vertical switches do
    }

    // IApplyTheme
    //
    // For an SvgSwitch, Rack selects the current presentation (this->sw) from one of
    // a series of backing SVGs ("frames"). A simple invalidation (DirtyEvent) of the widget doesn't force 
    // selection of the frame. In order to set the correct frame for the current value of the parameter
    // backing the switch, we send a ChangeEvent and the handler calculates the correct frame for us.
    // If we only dirty, the new theme isn't shown until the value of the switch changes.
    //
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        // Check if we're refreshing the widget with a new theme (and thus need to send a change event),
        // or being initialized by the creation helper.
        bool refresh = frames.size() > 0; 
        if (refresh) {
            frames.clear();
        }

        addFrame(engine.loadSvg(asset::plugin(pluginInstance, "res/vswitch2-0.svg"), theme));
        addFrame(engine.loadSvg(asset::plugin(pluginInstance, "res/vswitch2-1.svg"), theme));

        if (refresh) {
            // send change event to ensure the switch ui is set to the correct frame
            EventContext ctx;
            Widget::ChangeEvent eChange;
            eChange.context = &ctx;
            onChange(eChange);
        }
        return refresh;
    }
};

struct ThemeKnob : rack::RoundKnob, IApplyTheme
{
    // IApplyTheme
    bool applyTheme(SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) override
    {
        this->bg->setSvg(engine.loadSvg(asset::plugin(pluginInstance, "res/Knob-bg.svg"), theme));
        setSvg(engine.loadSvg(asset::plugin(pluginInstance, "res/Knob.svg"), theme));
        return true;
    }
};