#include <rack.hpp>
#include "plugin.hpp"
#include "../svg_theme.hpp"

using namespace rack;

struct DemoModule : Module
{
    std::string theme;
    svg_theme::SvgThemes themes;

    DemoModule() {
        // For demo purposes, log to the Rack log.
        // In a production Rack module in the library, logging to Rack's log is disallowed.
        // The logging is necessary only when authoring your theme and SVG.
        // Once your theme is correctly applying to the SVG, you do not need this logging
        // because it's useless to anyone other than someone modifying the SVG or theme.
        themes.setLog([](svg_theme::Severity severity, svg_theme::ErrorCode code, std::string info)->void
        {
            DEBUG("Theme %s (%d): %s", SeverityName(severity), code, info.c_str());
        });
    }

    // getters and setters
    void setTheme(std::string theme) { this->theme = theme; }
    std::string getTheme() { return theme; }

    // initThemes must be called to load themes.
    // It can be safely called multiple times and it only loads the first time.
    // In other words, it is "lazy" or just-in-time initialization.
    bool initThemes()
    {
        return themes.isLoaded()
            ? true
            : themes.load(asset::plugin(pluginInstance, "res/Demo-themes.json"));
    }
    svg_theme::SvgThemes& getThemes() { return themes; }

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

struct  DemoModuleWidget : ModuleWidget
{
    DemoModule* my_module = nullptr;

    DemoModuleWidget(DemoModule *module)
    {
        my_module = module;
        setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Demo.svg")));
        if (my_module && !isDefaultTheme()) {
            if (my_module->initThemes()) {
                setTheme(my_module->getTheme());
            }
        }
        // Add standard rack screws (if you want them)
        // addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // It's worth noting that Rack's widgets cannot be themed because the
        // Rack widget SVGs do not contain the element ids required for targeting.
    }

    bool isDefaultTheme() {
        if (!my_module) return true;
        auto theme = my_module->getTheme();
        return theme.empty() || 0 == theme.compare("Light");
    }

    std::string themeOrDefault() {
        return isDefaultTheme() ? "Light" : my_module->getTheme(); 
    }

    void setTheme(std::string theme) {
        if (!my_module) return;
        auto panel = dynamic_cast<rack::app::SvgPanel*>(getPanel());
        if (!panel) return;

        my_module->initThemes(); // load themes as necessary
        auto themes = my_module->getThemes();
        auto svg_theme = themes.getTheme(theme);
        if (themes.applyTheme(svg_theme, panel->svg->handle)) {
            // The SVG was changed, so we need to tell the widget to redraw
            EventContext ctx;
            DirtyEvent dirt;
            dirt.context = &ctx;
            panel->onDirty(dirt);

            // Let the module know what the new theme is so that it will be remembered.
            my_module->setTheme(theme);
        }
    }

    void appendContextMenu(Menu *menu) override
    {
        if (!my_module) return;
        if (!my_module->initThemes()) return;
        auto themes = my_module->getThemes();
        if (!themes.isLoaded()) return; // can't load themes, so no menu to display

        auto theme_names = themes.getThemeNames();
        if (theme_names.empty()) return; // no themes

        menu->addChild(new MenuSeparator);

        std::vector<rack::ui::MenuItem*> menus;
        for (auto theme : theme_names) {
            menus.push_back(createCheckMenuItem(
                theme, "", [=]() { return 0 == theme.compare(themeOrDefault()); },
                [=]() { setTheme(theme); }
            ));
        }
        for (auto child: menus) {
            menu->addChild(child);
        }
    }
};

Model *modelDemo = createModel<DemoModule, DemoModuleWidget>("svg-theme-demo");
