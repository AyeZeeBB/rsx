@echo off
echo Building ZSTD library...

:: Set up Visual Studio environment (adjust path as needed)
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
if errorlevel 1 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" 2>nul
)

:: Create output directory
if not exist "lib" mkdir lib

:: Compile all necessary ZSTD source files for decompression
cl /c /O2 /MD /DZSTD_STATIC_LINKING_ONLY /I. ^
    common\debug.c ^
    common\entropy_common.c ^
    common\error_private.c ^
    common\fse_decompress.c ^
    common\pool.c ^
    common\threading.c ^
    common\xxhash.c ^
    common\zstd_common.c ^
    decompress\huf_decompress.c ^
    decompress\zstd_ddict.c ^
    decompress\zstd_decompress.c ^
    decompress\zstd_decompress_block.c

:: Create the library
lib /OUT:libzstd.lib *.obj

:: Clean up object files
del *.obj

echo ZSTD library built successfully!
pause