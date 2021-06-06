<!-- BADGES -->
[![Issues][issues-badge]][issues-url]
[![License][mit-badge]][mit-url]
[![Stargazers][stars-badge]][stars-url]
[![Forks][forks-badge]][forks-url]
[![Contributors][contributors-badge]][contributors-url]

<!-- HEADING -->
APPIMAGE-RUNTIME
================

→ Extended variant of the AppImage runtime ←

[Join Chat](https://web.libera.chat/#AppImage)
	·
[Bugs & Features](https://github.com/tim-janik/appimage-runtime/issues)

<!-- ABOUT -->
## About this AppImage-Runtime

This runtime is based on the [AppImageKit](https://github.com/AppImage/AppImageKit)
[src/runtime.c](https://github.com/AppImage/AppImageKit/blob/master/src/runtime.c)
with a few extensions:

<!-- ☑ ☐  -->
☑  All libraries except libc, libdl, libpthread are statically linked, in particular the libraries for SquashFs and FUSE.

☑  All compression formats supported by SquashFs are included in the `runtime` binary.

☑  A smaller `runtime-zstd` variant is provided that can only uncompress zstd SquashFs images.

<!-- USAGE -->
## Usage

Clone this project on Linux and run `make`.

Append a SquashFs image with a `/AppRun` executable to the resulting `runtime` or `runtime-zstd` to form a packed
[AppImage executable](https://discourse.appimage.org/t/how-to-run-an-appimage/80).

<!-- MARKDOWN LINKS & IMAGES -->
[contributors-badge]: https://img.shields.io/github/contributors/tim-janik/appimage-runtime.svg?style=for-the-badge
[contributors-url]: https://github.com/tim-janik/appimage-runtime/graphs/contributors
[forks-badge]: https://img.shields.io/github/forks/tim-janik/appimage-runtime.svg?style=for-the-badge
[forks-url]: https://github.com/tim-janik/appimage-runtime/network/members
[issues-badge]: https://img.shields.io/github/issues/tim-janik/appimage-runtime.svg?style=for-the-badge
[issues-url]: https://github.com/tim-janik/appimage-runtime/issues
[mit-badge]: https://img.shields.io/static/v1?label=License&message=MIT&color=9c0&style=for-the-badge
[mit-url]: https://github.com/tim-janik/appimage-runtime/blob/trunk/LICENSE
[stars-badge]: https://img.shields.io/github/stars/tim-janik/appimage-runtime.svg?style=for-the-badge
[stars-url]: https://github.com/tim-janik/appimage-runtime/stargazers
