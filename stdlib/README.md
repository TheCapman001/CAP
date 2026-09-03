# CAP Standard Library

This directory is reserved for CAP-native standard-library modules.

CAP 1.0 keeps the core runtime dependency-free. Module loading is recognized syntactically but intentionally reports that modules are not available yet. This avoids shipping a fake standard library.

Planned safe library areas:
- text utilities
- math helpers
- collections
- file utilities
- time/date helpers
- structured data
- networking with explicit, permission-aware APIs
