# svg_theme

Lean in-memory SVG theming for VCV Rack plugins.

See the [Documentation](docs/svg_theme.md) for details on authoring themeable SVGs and themes, and creating themable widgets.

The only files you need to reference from your module are `svgtheme.hpp` and `svt_rack.hpp`.

One (and only one) source file in your project must contain:

```cpp
#define IMPLEMENT_SVG_THEME
#include "svgtheme.hpp" // SVG themeing
#include "svt_rack.hpp" // VCV Rack-specific SVG theming helpers
```

In this project, it's done in it's own .cpp file, but that is not required.
I recommend this way, but you can put it whatever source file is convenient for you (perhaps `plugin.cpp`).
Everthing is namespaced, so _your_ namespaces won't be polluted.
See [`svg_theme_impl.cpp`](src/svg_theme_impl.cpp) for an example of the implementation unit.
Feel free to copy it into your project.

Everything else here is documentation and a demo plugin you can build to see svg_theme in action.

Try it out! If you find a bug, open an issue.

I hang out in the [VCV Rack Community](https://community.vcvrack.com/) 
and the [VCV Discord](https://discord.com/channels/369861961514483723/370128910869987329)
if you have comments or need help.

Special thanks to **landgrvi**, who found issues caused by a bad interaction with Rack's SVG caching (issue #3),
and not only reported the problem but built a solution and made an (accepted) PR to solve it.
Honors for first community contribution!
I've built on the that PR to make a working solution that works for multi-module plugins
and adds support for themeable widgets.

## For Users of the initial release

Latest update is breaking, of necessity to fix the issues with the Rack SVG cache.
Names of the classes and helpers have changed, requiring updates at most places you use the code.
This is a good thing, becuase the initial strategy used was broken WRT to the Rack cache,
duplicated modules, multiple modules, and multiple-module plugins.

This update is a more real solution, covering these cases and adding support for widgets,
which turns out to be a bit more work to do properly.

The core SVG theming remains pretty much the same.
The logistics of implementation in a plugin have changed significantly, so review the Demo carefully.

Major changes:

- The engine moves to plugin scope from the module.
  This is necessary for multi-module plugins, sin part o that their widget SVGs can be shared.
- The engine now carries it's own SVG cache, and all SVG loading must go through svg_theme's methods.
- Updates to the panel creation logic.
- New facilities for building themeable wiodgets, and basic implementtions for ports, klnobs, and switches.

There's moe work to do. We have support for knobs, switches, and jacks,
but we still need lights, sliders, and the rest of the Rack classes of widgets.
PRs welcome!
