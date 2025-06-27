# 🚀 Release Cheat Sheet

Quick reference for creating releases of the Microphone Volume Control Service.

## 📋 Pre-Release Checklist

- [ ] Code is tested and working
- [ ] All changes are committed and pushed
- [ ] Version number updated in `version.h`
- [ ] README.md updated if needed
- [ ] No build errors locally

## 🏷️ Creating a Release

### Option 1: Automated Script (Recommended)

```cmd
# 1. Update version in version.h
# 2. Commit changes
git add .
git commit -m "Release v1.0.3 - Add new features"
git push

# 3. Create and push release tag
create_release.bat
```

### Option 2: Manual

```cmd
# Create tag
git tag -a "v1.0.3" -m "Release v1.0.3"

# Push tag (triggers GitHub Actions)
git push origin v1.0.3
```

## 🔢 Version Numbering

Edit `version.h`:

```cpp
#define VERSION_MAJOR 1    // Breaking changes
#define VERSION_MINOR 0    // New features  
#define VERSION_PATCH 3    // Bug fixes
```

**Examples:**
- `1.0.0` → `1.0.1` (bug fix)
- `1.0.1` → `1.1.0` (new feature)
- `1.1.0` → `2.0.0` (breaking change)

## 🤖 What Happens Automatically

When you push a tag starting with `v`:

1. **GitHub Actions builds** the project
2. **Tests** the executable
3. **Creates release package** with:
   - `MicrophoneVolumeService.exe`
   - Documentation (`README.md`, `EXAMPLES.md`)
   - Helper scripts (`*.bat`)
   - Version info
4. **Publishes GitHub Release** with generated notes

## 📦 Release Contents

Each release includes:

### ZIP Package: `MicrophoneVolumeService-vX.X.X-x64.zip`
```
📁 MicrophoneVolumeService-v1.0.3-x64/
├── 📄 MicrophoneVolumeService.exe
├── 📄 README.md
├── 📄 EXAMPLES.md  
├── 📄 VERSION.txt
├── 📄 build.bat
├── 📄 install_service.bat
├── 📄 uninstall_service.bat
├── 📄 test_service.bat
└── 📄 check_service.bat
```

### Standalone: `MicrophoneVolumeService.exe`
- Just the executable for users who only need the binary

## 🔍 Monitoring Release

1. **Check Actions tab**: `https://github.com/[user]/[repo]/actions`
2. **Wait for green checkmark** ✅
3. **Release appears**: `https://github.com/[user]/[repo]/releases`

**Build time**: ~5-10 minutes

## 🐛 Troubleshooting

### Build Fails
- Check GitHub Actions logs
- Test build locally with `build.bat`
- Verify Visual Studio project settings

### Release Not Created
- Ensure tag starts with `v` (case sensitive)
- Check tag was pushed: `git push origin v1.0.3`
- Verify GitHub Actions workflow permissions

### Wrong Version in Release
- Version comes from `version.h`, not git tag
- Update `version.h` and create new tag

## 📝 Release Notes Template

GitHub Actions auto-generates release notes, but you can customize the tag message:

```cmd
git tag -a "v1.0.3" -m "Release v1.0.3 - Bug Fixes

- Fixed microphone detection issue
- Improved service startup reliability  
- Better error handling for invalid devices
- Updated documentation examples

This release improves compatibility with various microphone drivers."
```

## 🏃‍♂️ Quick Commands

```cmd
# Check current version
findstr "VERSION_" version.h

# See recent tags  
git tag --sort=-version:refname | head -5

# Delete tag (if mistake)
git tag -d v1.0.3
git push origin :refs/tags/v1.0.3

# Check release status
curl -s https://api.github.com/repos/[user]/[repo]/releases/latest | findstr "tag_name"
```

## 🎯 Best Practices

1. **Test locally** before releasing
2. **Update documentation** with new features
3. **Use semantic versioning** (Major.Minor.Patch)
4. **Write meaningful commit messages**
5. **Keep releases small** and focused
6. **Test installation** on clean system

---

💡 **Pro tip**: GitHub Actions automatically creates professional-looking releases with download statistics, installation instructions, and feature summaries!
