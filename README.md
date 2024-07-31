# CronoS

__[EXAMPLES](/examples/) | [CHANGELOG](CHANGELOG.md) |__ [![PlatformIO CI](https://github.com/vortigont/CronoS/actions/workflows/pio_build.yml/badge.svg)](https://github.com/vortigont/CronoS/actions/workflows/pio_build.yml) | [![PlatformIO Registry](https://badges.registry.platformio.org/packages/vortigont/library/CronoS.svg)](https://registry.platformio.org/libraries/vortigont/CronoS)

**CronoS** - is a `Cron` `o`n RTO`S`
A tiny wrapper lib that provides classes to run scheduled tasks/callbacks with a help of a RTOS timers and rules defined via [CRONtab](https://en.wikipedia.org/wiki/Cron) expression syntax.
Cron is most suitable for scheduling repetitive tasks.


### Origin
`CronoS` relies on a excellent [supertinycron](https://github.com/exander77/supertinycron) lib that does all `crontab` syntax parsing.

### Compatibility
Tested only on ESP32's implementation of RTOS. Might worn on other platforms too, but not tested yet.

### Usage
Pls, check [supertinycron](https://github.com/exander77/supertinycron)'s page for `crontab` syntax implementation and details

#### Licence
This lib inherits [supertinycron](https://github.com/exander77/supertinycron)'s Apache License, Version 2.0
