#include <rack.hpp>
#include "plugin.hpp"
#include "widgets.hpp"

using namespace rack;

// Demo Blank module (no processing) demonstrating themeing using svg_theme.
struct DemoModule : Module
{
    // The name of the selected theme, to persist
    std::string theme_name;

    // get and set the theme to be persisted
    void setThemeName(std::string name) { this->theme_name = name; }
    std::string getThemeName() { return theme_name; }

    // Standard Rack persistence so that the selected theme is remembered with the patch.
    void dataFromJson(json_t* root) override {
        json_t* j = json_object_get(root, "theme");
        if (j) {
            theme_name = json_string_value(j);
        }
    }
    json_t* dataToJson() override {
        json_t* root = json_object();
        json_object_set_new(root, "theme", json_stringn(theme_name.c_str(), theme_name.size()));
        return root;
    }
};

struct DemoModuleWidget : ModuleWidget, svg_theme::IThemeHolder
{
    DemoModule* my_module = nullptr;

    std::string panelFilename() {
        return asset::plugin(pluginInstance, "res/Demo.svg");
    }
    
    DemoModuleWidget(DemoModule* module)
    {
        my_module = module;
        setModule(module);

        initThemeEngine();

        auto theme = theme_engine.getTheme(getThemeName());
		setPanel(createThemedPanel(panelFilename(), theme_engine, theme));

        // Rack's widgets cannot be themed because the Rack widget SVGs do not
        // contain the element ids required for targeting.
        // Here we've copied the Rack screws, added ids, and created our own screw subclass: "ThemeScrew".
        // See `widgets.hpp` for the definition of a ThemeScrew.
        addChild(svg_theme::createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), theme_engine, theme));
        addChild(svg_theme::createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0), theme_engine, theme));
        addChild(svg_theme::createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
        addChild(svg_theme::createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));

        //theme_engine.showCache();
   }

    // true when the default theme is the current theme
    bool isDefaultTheme() {
        if (!my_module) return true;
        auto name = my_module->getThemeName();
        return name.empty() || 0 == name.compare("Light");
    }

    // IThemeHolder used by the menu helper
    std::string getThemeName() override
    {
        return isDefaultTheme() ? "Light" : my_module->getThemeName(); 
    }

    // IThemeHolder used by the menu helper, and also whenever 
    // we want to apply a new theme
    void setThemeName(std::string name) override
    {
        if (!my_module) return;
        auto panel = dynamic_cast<rack::app::SvgPanel*>(getPanel());
        if (!panel) return;

        if (!initThemeEngine()) {
            assert(false);
            return;
        }

        auto svg_theme = theme_engine.getTheme(name);
        if (!svg_theme) return;

        // For demo purposes, we are using a stock Rack SVGPanel
        // which does not implement IApplyTheme.so here we do it manually.
        // This shows how to apply themeing without implementing IApplyTheme
        // and using ApplyChildrenTheme.
        std::shared_ptr<Svg> newSvg = panel->svg;
        if (theme_engine.applyTheme(svg_theme, panelFilename(), newSvg)) {
			panel->setBackground(newSvg);
        }
        // The preferred procedure is to subclass any widget you want to theme,
        // implementing IApplyTheme (which is quite simple to do in most cases),
        // and use this helper to apply the theme to the widget hierarchy.
        ApplyChildrenTheme(this, theme_engine, svg_theme);

        // Let the module know what the new theme is so that it will be remembered.
        my_module->setThemeName(name);
    }

    void appendContextMenu(Menu *menu) override
    {
        if (!my_module) return;
        if (!initThemeEngine()) return;
        if (!theme_engine.isLoaded()) return; // Can't load themes, so no menu to display

        // Good practice to separate your module's menus from the Rack menus
        menu->addChild(new MenuSeparator); 

        // add the "Theme" menu
        svg_theme::AppendThemeMenu(menu, this, theme_engine);
    }
};

Model *modelDemo = createModel<DemoModule, DemoModuleWidget>("svg-theme-demo");
