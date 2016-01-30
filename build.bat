@echo off

:: ==== NOTE ====
:: - mlt_opt: Optimized build.
:: - mlt_nopt: Debug build.
set mlt_opt_level=%mlt_opt%



IF NOT EXIST build mkdir build
IF NOT EXIST build\SETUP_DONE call scripts\setup.bat

pushd build


set sdl_link_deps=Winmm.lib Version.lib Shell32.lib Ole32.lib OleAut32.lib Imm32.lib

set mlt_defines=-D_CRT_SECURE_NO_WARNINGS

:: Oy- disable frame pointer omission (equiv. to -f-no-omit-frame-pointer)
set mlt_opt=/Ox /Oy- /MT
set mlt_nopt=/Od /MTd


set mlt_compiler_flags=/Oi /Zi /GR- /Gm- /Wall /WX /fp:fast /nologo /FC /EHsc

REM 4100 Unreferenced func param (cleanup)
REM 4820 struct padding
REM 4255 () != (void)
REM 4668 Macro not defined. Subst w/0
REM 4710 Func not inlined
REM 4711 Auto inline
REM 4189 Init. Not ref
REM 4201 Nameless struct (GNU extension, but available in MSVC. Also, valid C11)
REM 4204 Non-constant aggregate initializer.
REM 4800 b32 to bool or int to bool perf warning
REM 4191 FARPROC from GetProcAddress
REM 5027 move assignment operator implicitly deleted
REM 4127 expression is constant. while(0) et al. Useful sometimes? Mostly annoying.
REM 4514 unreferenced inline function removed
set comment_for_cleanup=/wd4100 /wd4189 /wd4800 /wd4127
set mlt_disabled_warnings=%comment_for_cleanup% /wd4820 /wd4255 /wd4668 /wd4710 /wd4711 /wd4201 /wd4204 /wd4191 /wd5027 /wd4514
set mlt_includes=-I ..\third_party\ -I ..\third_party\imgui -I ..\third_party\SDL2-2.0.3\include -I ..\..\EasyTab -I ..\third_party\nativefiledialog\src\include

set sdl_dir=..\third_party\SDL2-2.0.3\VisualC\SDL\x64\Debug

set mlt_links=..\third_party\glew32s.lib OpenGL32.lib %sdl_dir%\SDL2.lib user32.lib gdi32.lib Comdlg32.lib %sdl_link_deps%

:: ---- Compile third_party libs with less warnings
:: Delete file build\SKIP_LIB_COMPILATION to recompile. Created by default to reduce build times.
IF EXIST SKIP_LIB_COMPILATION goto skip_lib_compilation
echo    [BUILD] -- Building dependencies...

echo    [BUILD] -- ... Release
cl %mlt_opt% %mlt_includes% /Zi /c /EHsc ..\src\headerlibs_impl.cc
if %errorlevel% NEQ 0 goto error_lib_compilation
lib headerlibs_impl.obj
copy headerlibs_impl.lib headerlibs_impl_opt.lib

echo    [BUILD] -- ... Debug
cl %mlt_nopt% %mlt_includes% /Zi /c /EHsc ..\src\headerlibs_impl.cc
if %errorlevel% NEQ 0 goto error_lib_compilation
lib headerlibs_impl.obj
copy headerlibs_impl.lib headerlibs_impl_nopt.lib

goto end_lib_compilation
:error_lib_compilation
echo [FATAL] -- Error building dependencies!!
goto fail

:end_lib_compilation

echo    [BUILD] -- Dependencies built!

echo    [BUILD] --   To rebuild them, call DEL build\SKIP_LIB_COMPILATION

:: Create file to skip this the next time
::copy /b SKIP_LIB_COMPILATION +,,
type nul >>SKIP_LIB_COMPILATION
:skip_lib_compilation

echo    [BUILD] -- Building Milton...

if "%mlt_opt_level%" == "%mlt_nopt%" set header_links=headerlibs_impl_nopt.lib
if "%mlt_opt_level%" == "%mlt_opt%" set header_links=headerlibs_impl_opt.lib

:: ---- Unity build for Milton
cl %mlt_opt_level% %mlt_compiler_flags% %mlt_disabled_warnings% %mlt_defines% %mlt_includes%  ..\src\milton_unity_build.cc /FeMilton.exe %mlt_links% %header_links%
if %errorlevel% equ 0 goto ok
goto fail

goto ok

:ok
echo    [BUILD]  -- Build success!
popd
goto end

:fail
echo    [FATAL] -- ... error building Milton
popd && (call)

:end
