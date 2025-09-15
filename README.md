# BassicManager

![BassicManager](ui.png)\
[![](https://github.com/carthach/BassicManager/workflows/CMake/badge.svg)](https://github.com/carthach/BassicManager/actions)

BassicManager is an open source plugin for bass management of X.1 surround sound projects. Stick this on your 5.1 master bus to hear how your mix will sound on bass management systems. It is important to note that this is a MONITORING TOOL. Make sure to disable or bypass the plugin when you are rendering and mixing down.

## What is bass management?

Home theatres generally don't have full range main system loudspeakers, so bass management is the process by which low frequency content in the L, R, C, LS, and RS channels is rerouted and combined with the low passed LFE signal to a subwoofer. In professional cinema theatres, each main speaker is (supposed to be) full-range and can reproduce its information fully so don't need to do this,.

In recording studio monitor systems, subwoofers are typically connected directly to an audio interface and hence might not have bass management. This tool is a monitoring aid to recreate and check what bass management will do to your project on home theatre systems. 

How this is done in software follows the block diagram below:

![BlockDiagram](https://upload.wikimedia.org/wikipedia/commons/thumb/d/da/BassManagement5.1.png/550px-BassManagement5.1.png)

For more information check [Wikipedia](https://en.wikipedia.org/wiki/Bass_management)

## Installing

Download the artifacts from [Github Actions](https://github.com/carthach/BassicManager/actions)

I refuse to shell out 100 euro to Apple for codesigning. You might need to clear the quarantine attributes after downloading on Mac:

```sh
sudo xattr -r -d com.apple.quarantine /path/to/plugin.vst3/
```


## Licence

The code is GPL, if you want to use it commercially contact me.

The downloadable plugins are donationware.

## Acknowledgements

Software built with [JUCE](https://juce.com/)

Excellent CI/CD template by [Pamplejuce](https://github.com/sudara/pamplejuce)
