# GitHub Actions Setup

This project includes automated CI/CD with GitHub Actions for building and releasing the Microphone Volume Control Service.

## Workflows

### 1. Build Check (`build-check.yml`)
- **Triggers**: Push to main/master branch, Pull Requests
- **Purpose**: Verify that the code builds successfully
- **Artifacts**: Test builds (7-day retention)

### 2. Build and Release (`build-and-release.yml`)
- **Triggers**: Git tags starting with `v` (e.g., `v1.0.0`)
- **Purpose**: Build release version and create GitHub release
- **Outputs**: 
  - GitHub Release with release notes
  - ZIP package with executable and documentation
  - Individual executable file

## Creating a Release

### Automatic Method (Recommended)

1. **Update version** in `version.h`:
   ```cpp
   #define VERSION_MAJOR 1
   #define VERSION_MINOR 0  
   #define VERSION_PATCH 1  // Increment this
   ```

2. **Commit your changes**:
   ```cmd
   git add .
   git commit -m "Release v1.0.1 - Fix microphone detection"
   git push
   ```

3. **Create release tag**:
   ```cmd
   create_release.bat
   ```

The script will:
- Read version from `version.h`
- Create annotated Git tag
- Push to GitHub
- Trigger automatic build and release

### Manual Method

```cmd
# Create tag manually
git tag -a "v1.0.1" -m "Release v1.0.1"
git push origin v1.0.1
```

## Release Package Contents

Each release includes:

- **`MicrophoneVolumeService-vX.X.X-x64.zip`**: Complete package with:
  - `MicrophoneVolumeService.exe` - Main executable
  - `README.md` - Documentation
  - `EXAMPLES.md` - Usage examples  
  - `*.bat` - Helper scripts
  - `VERSION.txt` - Build information

- **`MicrophoneVolumeService.exe`**: Standalone executable

## Build Requirements

The GitHub Actions runner includes:

- Windows Server 2022
- Visual Studio 2022 Build Tools
- Windows 10/11 SDK
- MSBuild
- NuGet

## Configuration

### Environment Variables

- `BUILD_CONFIGURATION`: Release
- `BUILD_PLATFORM`: x64

### Windows SDK Version

Currently set to `20348` (Windows 11 SDK). This can be changed in the workflow files if needed.

## Monitoring Builds

1. **Go to Actions tab** in your GitHub repository
2. **Check workflow runs** for build status
3. **Download artifacts** from successful builds
4. **View release** in the Releases section

## Troubleshooting

### Build Fails

1. Check the workflow logs in GitHub Actions
2. Verify `MicrophoneVolumeService.sln` builds locally
3. Ensure all files are committed to the repository

### Release Not Created

1. Verify the tag starts with `v` (e.g., `v1.0.0`)
2. Check that the tag was pushed to GitHub
3. Review the release workflow logs

### Missing Dependencies

The workflow should handle all dependencies automatically. If builds fail due to missing components:

1. Update the Windows SDK version in workflow files
2. Add any missing NuGet packages
3. Verify the MSBuild platform toolset version

## Security

- Uses GitHub's built-in `GITHUB_TOKEN` for releases
- No external secrets required
- Builds run in isolated GitHub-hosted runners
- All dependencies come from official Microsoft sources
