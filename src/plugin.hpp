#pragma once
#include <rack.hpp>
#include "../svgtheme.hpp"
#include "../svt_rack.hpp"

// Declare the Plugin, defined in plugin.cpp
extern rack::Plugin* pluginInstance;

// Declare each Model, defined in each module source file

extern rack::Model* modelDemo;
extern rack::Model* modelClip;

extern svg_theme::SvgThemeEngine theme_engine;
bool initThemeEngine();