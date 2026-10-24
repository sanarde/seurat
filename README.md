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

## Links

- [CONTRIBUTING](CONTRIBUTING.md)

## License

GPL-3.0-only. See [LICENSE](LICENSE).
