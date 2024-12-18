#include "plugin.hpp"
using namespace ::rack;

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelDemo);
	p->addModel(modelClip);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

// The svg_theme engine is shared by all modules in the plugin
svg_theme::SvgThemeEngine theme_engine;

// initThemeEngine must be called to load the themes.
//
// It can be safely called multiple times and it only loads the first time.
// In other words, it is "lazy" or just-in-time initialization.
//
bool initThemeEngine()
{
	if (theme_engine.isLoaded()) return true;

	// For demo and authoring purposes, we log to the Rack log.
	//
	// In a production VCV Rack module in the library, logging to Rack's log is disallowed.
	// The logging is necessary only when authoring your theme and SVG.
	// Once your theme is correctly applying to the SVG, you do not need this logging
	// because it's useless to anyone other than someone modifying the SVG or theme.
	//
	theme_engine.setLog([](svg_theme::Severity severity, svg_theme::ErrorCode code, std::string info)->void {
		DEBUG("Theme %s (%d): %s", SeverityName(severity), code, info.c_str());
	});

	return theme_engine.load(asset::plugin(pluginInstance, "res/Demo-themes.json"));
}