# Sanar Seurat

Monet color engine for Material You and a color daemon that distributes the resulting theme across the desktop. 

## Dependencies

- C17 compiler
- Meson (>= 1.0) and Ninja
- libdbus-1 (daemon)

## Build

```sh
meson setup build
meson compile -C build
meson test -C build
```

## Interfaces

`seuratd` publishes the current scheme over two channels.

D-Bus (session bus): name `org.nuros.sanar.seurat`, object `/org/nuros/sanar/seurat`, interface `org.nuros.sanar.Seurat`.

- `GetScheme() -> a{su}`: role name to opaque ARGB.
- `GetSourceColor() -> u`: source color as ARGB.
- Signal `SchemeChanged(a{su})`.

Unix socket: `$XDG_RUNTIME_DIR/seurat.sock`. A client receives the current scheme on connect and again on every change, as text blocks:

```
scheme source=#6750a4 mode=light
primary=#65558f
...
end
```

## Configuration

`$XDG_CONFIG_HOME/seurat/seurat.conf` (default `~/.config/seurat/seurat.conf`), section `[seurat]`:

- `enabled`: `true` or `false`. The daemon exits when set to `false`.
- `color`: `#rrggbb`. Empty or absent selects the color automatically.
- `color_scheme`: `default`, `light` or `dark`.

The file is reloaded on change and on `SIGHUP`.

## Links

- [CONTRIBUTING](CONTRIBUTING.md)

## License

GPL-3.0-only. See [LICENSE](LICENSE).
