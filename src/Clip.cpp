#include <rack.hpp>
#include "plugin.hpp"
#include "widgets.hpp"

using namespace rack;
using namespace svg_theme;

struct ClipModule : Module
{
    // The name of the selected theme
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
        j = json_object_get(root, "hi");
        if (j) {
            clip_max = json_boolean_value(j);
        }
        j = json_object_get(root, "cut");
        if (j) {
            cutoff = json_number_value(j);
        }
    }
    json_t* dataToJson() override {
        json_t* root = json_object();
        json_object_set_new(root, "theme", json_stringn(theme_name.c_str(), theme_name.size()));
        json_object_set_new(root, "hi", json_boolean(clip_max));
        json_object_set_new(root, "cut", json_real(cutoff));
        return root;
    }

    enum Params {
       P_CUTOFF,
       P_HILO,
       NUM_PARAMS
    };
    enum Inputs {
       IN_SIGNAL,
       NUM_INPUTS
    };
    enum Outputs {
       OUT_SIGNAL,
       NUM_OUTPUTS
    };
    enum Lights {
       NUM_LIGHTS
    };

    bool clip_max = true;
    float cutoff = 0.f;

    ClipModule()
    {
        config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
        
        // Configure each param, input, output, and light:
        configParam(Params::P_CUTOFF, -10.f, 10.f, 0.f, "Cutoff", "volts");
        configSwitch(Params::P_HILO, 0.f, 1.f, 0.f, "High/Low", {"max", "min"});
        configInput(Inputs::IN_SIGNAL, "Signal");
        configOutput(Outputs::OUT_SIGNAL, "Clipped signal");
    }

    const int PARAM_INTERVAL = 64; // process params every 64 frames
    void processParams()
    {
        // process parameters here
        clip_max = getParam(Params::P_HILO).getValue() < 0.5f;
        cutoff = getParam(Params::P_CUTOFF).getValue();
    }

    void process(const ProcessArgs &args) override
    {
        // Process params at intervals
        if (0 == ((args.frame + id) % PARAM_INTERVAL))
        {
            processParams();
        }

        if (getInput(IN_SIGNAL).isConnected() && getOutput(OUT_SIGNAL).isConnected())
        {
            float v = getInput(IN_SIGNAL).getVoltage(0);
            if (clip_max && (v > cutoff)) {
                v = cutoff;
            } else if (!clip_max &&( v < cutoff)) {
                v = cutoff;
            }
            getOutput(OUT_SIGNAL).setVoltage(v);
        }
    }
};


struct ClipModuleWidget : ModuleWidget, svg_theme::IThemeHolder
{
    ClipModule* my_module = nullptr;

    std::string panelFilename() { return asset::plugin(pluginInstance, "res/Clip.svg"); }

    ClipModuleWidget(ClipModule *module)
    {
        // ensure the theme engine has been initialized
        initThemeEngine();

        my_module = module;
        setModule(module);

        // acquire the active theme that we'll apply to the panel and the widgets.
        auto theme = theme_engine.getTheme(getThemeName());

        // svg_theme requires that we use its panel and widget creation helpers.
        setPanel(createThemedPanel(panelFilename(), theme_engine, theme));

        // Add standard rack screws
        addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, 0), theme_engine, theme));
        addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0), theme_engine, theme));
        addChild(createThemedWidget<ThemeScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));
        addChild(createThemedWidget<ThemeScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH), theme_engine, theme));

        // Add parameter widgets, inputs and outputs
        addChild(Center(createThemedParam<ThemeKnob>(Vec(75, 220), module, ClipModule::Params::P_CUTOFF, theme_engine, theme)));
        addChild(Center(createThemedParam<ThemeSwitchV2>(Vec(75, 300), module, ClipModule::Params::P_HILO, theme_engine, theme)));
        addChild(Center(createThemedInput<ThemePort>(Vec(42, 348), module, ClipModule::Inputs::IN_SIGNAL, theme_engine, theme)));
        addChild(Center(createThemedOutput<ThemePort>(Vec(106, 348), module, ClipModule::Outputs::OUT_SIGNAL, theme_engine, theme)));

        //theme_engine.showCache();

    }

    // true when the default theme is the current theme
    bool isDefaultTheme() {
        if (!my_module) return true;
        auto theme = my_module->getThemeName();
        return theme.empty() || 0 == theme.compare("Light");
    }

    // IThemeHolder used by the context menu helper
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

        auto svg_theme = theme_engine.getTheme(name);

        // For demo purposes, we are using a stock Rack SvgPanel
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

Model *modelClip = createModel<ClipModule, ClipModuleWidget>("svg-theme-clip");
