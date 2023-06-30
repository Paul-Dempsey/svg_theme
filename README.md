# svg_theme

Lean in-memory SVG theming for VCV Rack plugins.

[Documentation](docs/svg_theme.md)

The only file you need is `svg_theme.hpp`.

One (and only one) source file in your project must contain:

```cpp
#define IMPLEMENT_SVG_THEME
#include "svg_theme.hpp"
```

In this project, it's done in it's own .cpp file, but that is not required.
See [`svg_theme_impl.cpp`](src/svg_theme_impl.cpp).

Everything else here is documentation and a demo module you can build to see svg_theme in action.
