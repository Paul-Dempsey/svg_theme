#include <rack.hpp>
#include "plugin.hpp"
#include "../svgtheme.hpp"
#include "../svt_rack.hpp"
#include "widgets.hpp"

using namespace rack;

// Demo Blank module (no processing) demonstrating themeing using svg_theme.
struct DemoModule : Module
{
    // This is the name of the selected theme, to save in json
    std::string theme;
    // The svg_theme engine
    svg_theme::SvgThemes themes;

    DemoModule() {
        // For demo and authoring purposes, we log to the Rack log.
        //
        // In a production VCV Rack module in the library, logging to Rack's log is disallowed.
        // The logging is necessary only when authoring your theme and SVG.
        // Once your theme is correctly applying to the SVG, you do not need this logging
        // because it's useless to anyone other than someone modifying the SVG or theme.
        //
        themes.setLog([](svg_theme::Severity severity, svg_theme::ErrorCode code, std::string info)->void {
            DEBUG("Theme %s (%d): %s", SeverityName(severity), code, info.c_str());
        });
    }

    // get and set the theme to be persisted
    void setTheme(std::string theme) { this->theme = theme; }
    std::string getTheme() { return theme; }

    // access tot he themes engine for the ModuleWidget
    svg_theme::SvgThemes& getThemes() { return themes; }

    // initThemes must be called to load themes.
    // It can be safely called multiple times and it only loads the first time.
    // In other words, it is "lazy" or just-in-time initialization.
    bool initThemes()
    {
        return themes.isLoaded()
            ? true
            : themes.load(asset::plugin(pluginInstance, "res/Demo-themes.json"));
    }


    // Standard Rack persistence so that the selected theme is remembered with the patch.
    void dataFromJson(json_t* root) override {
        json_t* j = json_object_get(root, "theme");
        if (j) {
            theme = json_string_value(j);
        }
    }
    json_t* dataToJson() override {
        json_t* root = json_object();
        json_object_set_new(root, "theme", json_stringn(theme.c_str(), theme.size()));
        return root;
    }
};

struct  DemoModuleWidget : ModuleWidget, svg_theme::IThemeHolder
{
    DemoModule* my_module = nullptr;

    DemoModuleWidget(DemoModule* module)
    {
        my_module = module;
        setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Demo.svg")));

        // Rack's widgets cannot be themed because the Rack widget SVGs do not
        // contain the element ids required for targeting.
        // Here we've copied the Rack screws, added ids, and created our own screw subclass: "ThemeScrew".
        // See `widgets.hpp` for the definition of a ThemeScrew.
        addChild(createWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        if (my_module && !isDefaultTheme()) {
            // only initialize themes and modify the svg when the current hteme is not the default theme
            if (my_module->initThemes()) {
                setTheme(my_module->getTheme());
            }
        }
   }

    // true when the default theme is the current theme
    bool isDefaultTheme() {
        if (!my_module) return true;
        auto theme = my_module->getTheme();
        return theme.empty() || 0 == theme.compare("Light");
    }

    // IThemeHolder used by the menu helper
    std::string getTheme() override
    {
        return isDefaultTheme() ? "Light" : my_module->getTheme(); 
    }

    // IThemeHolder used by the menu helper, and also whenever 
    // we want to apply a new theme
    void setTheme(std::string theme) override
    {
        if (!my_module) return;
        auto panel = dynamic_cast<rack::app::SvgPanel*>(getPanel());
        if (!panel) return;

        my_module->initThemes(); // load themes as necessary
        auto themes = my_module->getThemes();
        auto svg_theme = themes.getTheme(theme);

        // For demo purposes, we are using a stock Rack SVGPanel
        // which does not implement IApplyTheme.so here we do it manually.
        // This shows how to apply themeing without implementing IApplyTheme
        // and using ApplyChildrenTheme.
        if (themes.applyTheme(svg_theme, panel->svg->handle)) {
            // The SVG was changed, so we need to tell the widget to redraw
            EventContext ctx;
            DirtyEvent dirt;
            dirt.context = &ctx;
            panel->onDirty(dirt);
        }
        // The preferred procedure is to subclass any widget you want to theme,
        // implementing IApplyTheme (which is quite simple to do in most cases),
        // and use this helper to apply the theme to the widget hierarchy.
        ApplyChildrenTheme(this, themes, svg_theme);

        // Let the module know what the new theme is so that it will be remembered.
        my_module->setTheme(theme);
    }

    void appendContextMenu(Menu *menu) override
    {
        if (!my_module) return;
        if (!my_module->initThemes()) return;
        auto themes = my_module->getThemes();
        if (!themes.isLoaded()) return; // Can't load themes, so no menu to display

        // Good practice to separate your module's menus from the Rack menus
        menu->addChild(new MenuSeparator); 

        // add the "Theme" menu
        svg_theme::AppendThemeMenu(menu, this, themes);
    }
};

Model *modelDemo = createModel<DemoModule, DemoModuleWidget>("svg-theme-demo");
