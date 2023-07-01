# svg_theme

Lean in-memory SVG theming for VCV Rack plugins.

See the [Documentation](docs/svg_theme.md) for details on authoring themeable SVGs and themes.

The only files you need to reference from your module are `svgtheme.hpp` and `svt_rack.hpp`.

One (and only one) source file in your project must contain:

```cpp
#define IMPLEMENT_SVG_THEME
#include "svgtheme.hpp" // SVG themeing
#include "svt_rack.hpp" // VCV Rack-specific SVG theming helpers
```

In this project, it's done in it's own .cpp file, but that is not required.
I recommend this way, but you can put it whatever source file is convenient for you.
Everthing is namespaced, so your namespaces won't be polluted.
See [`svg_theme_impl.cpp`](src/svg_theme_impl.cpp) for an example.

Everything else here is documentation and a demo module you can build to see svg_theme in action.

Try it out! If you find a bug, open an issue.

I hang out in the [VCV Rack Community](https://community.vcvrack.com/) 
and the [VCV Discord](https://discord.com/channels/369861961514483723/370128910869987329)
if you have comments or need help.
