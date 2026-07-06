# Build Notes

Expected natID layout:

- `%USERPROFILE%/natID.SDK`
- `%USERPROFILE%/natID.RAMDisk`

Configure from this folder:

```powershell
cmake -S . -B out/build
cmake --build out/build --config Debug
```

The plugin target is `tgti`, built as a shared library (`.dll` on Windows, `.dylib` on macOS, `.so` on Linux).

Current implementation status:

- Plugin interface follows `sc::IPlugin`.
- GUI has converter, options, and preview tabs.
- Conversion runs in a worker thread.
- Progress is reported from a second thread through `gui::ProgressIndicator`.
- MATPOWER `mpc.gen` is parsed into `dense::DblMatrix`.
- Selected generator receives Turbine Governor Type I equations.
- Other generators receive a standard placeholder mechanical power model.

Next validation step is to compile against a machine with an available C++ compiler and local dTwin plugin loader.

