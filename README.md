# SEOwnedDE

Enhanced version of SEOwned with improved movement prediction.

## Automated Builds

This repository is configured with GitHub Actions to automatically build the project when changes are pushed to the main branch. The automated build:

- Builds only the x64 Release configuration with AVX2 instruction set
- Uploads DLL artifacts for easy access
- Includes build date, commit hash, and configuration information

## Recent Improvements

### Enhanced Movement Prediction

The movement prediction system now includes:

1. **Momentum Analysis**: Tracks velocity changes over multiple ticks to predict acceleration trends
2. **State-Based Prediction**: Handles different player states (ducking, airborne, etc.) with specialized calculations
3. **Distance-Based Scaling**: Adjusts prediction intensity based on distance to target

These improvements enhance hit accuracy by better modeling the movement patterns of enemies.

## Download Latest Build

You can download the latest automated build from the "Actions" tab above. Look for the most recent successful workflow run and download the "SEOwnedDE-x64-Release-AVX2" artifact. 

Based on [SEOwnedDE public](https://github.com/spook953/SEOwnedDE-public), and LNX's fork with various fixes and improvements. 