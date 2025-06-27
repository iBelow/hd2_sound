@echo off
REM Script to create a Git tag for new version release

echo GitHub Actions Release Tag Creator
echo ===================================
echo.

REM Check if we're in a Git repository
git status >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Not in a Git repository!
    echo Make sure you're in the project root directory.
    pause
    exit /b 1
)

REM Check for uncommitted changes
git diff --quiet --exit-code 2>nul
if %ERRORLEVEL% neq 0 (
    echo [WARNING] You have uncommitted changes!
    echo Please commit all changes before creating a release tag.
    echo.
    git status --short
    echo.
    set /p "choice=Continue anyway? (y/n): "
    if /i not "%choice%"=="y" (
        echo Aborted.
        pause
        exit /b 1
    )
)

REM Get current version from version.h
echo Reading version from version.h...
for /f "tokens=3" %%a in ('findstr "VERSION_MAJOR" version.h') do set MAJOR=%%a
for /f "tokens=3" %%a in ('findstr "VERSION_MINOR" version.h') do set MINOR=%%a
for /f "tokens=3" %%a in ('findstr "VERSION_PATCH" version.h') do set PATCH=%%a

set VERSION=%MAJOR%.%MINOR%.%PATCH%
echo Current version: v%VERSION%
echo.

REM Check if tag already exists
git tag | findstr /x "v%VERSION%" >nul
if %ERRORLEVEL% equ 0 (
    echo [ERROR] Tag v%VERSION% already exists!
    echo Either increment the version in version.h or delete the existing tag:
    echo   git tag -d v%VERSION%
    echo   git push origin :refs/tags/v%VERSION%
    pause
    exit /b 1
)

REM Show what will happen
echo This will:
echo 1. Create tag: v%VERSION%
echo 2. Push tag to origin
echo 3. Trigger GitHub Actions to build and create release
echo.

set /p "choice=Create and push tag v%VERSION%? (y/n): "
if /i not "%choice%"=="y" (
    echo Aborted.
    pause
    exit /b 0
)

REM Create and push tag
echo.
echo Creating tag v%VERSION%...
git tag -a "v%VERSION%" -m "Release v%VERSION% - Microphone Volume Control Service

Features:
- Automatic microphone volume control (100%)
- Configurable check intervals
- Microphone filtering support  
- Smart logging (only on changes)
- Windows Event Log support
- Auto-start service configuration

Fixes Helldivers 2 microphone volume bug."

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Failed to create tag!
    pause
    exit /b 1
)

echo Pushing tag to origin...
git push origin "v%VERSION%"

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Failed to push tag!
    echo Tag was created locally. You can push it manually:
    echo   git push origin v%VERSION%
    pause
    exit /b 1
)

echo.
echo ✅ SUCCESS!
echo.
echo Tag v%VERSION% created and pushed to GitHub.
echo GitHub Actions will now:
echo 1. Build the project automatically
echo 2. Create a release with binaries
echo 3. Upload MicrophoneVolumeService.exe
echo.
echo Check the Actions tab on GitHub to monitor progress:
echo https://github.com/[your-username]/[repo-name]/actions
echo.
echo The release will be available at:
echo https://github.com/[your-username]/[repo-name]/releases/tag/v%VERSION%
echo.
pause
