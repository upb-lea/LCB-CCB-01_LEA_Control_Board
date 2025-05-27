# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0] - 2025-05-27
### Changed
 * USB connector and some components close to the keep-out-areas have moved a bit
 * Rotated 3D-model of processor

## [1.1.0] - 2025-02-21
### Changed
 * Update component names to more readable format
 * Update many 0402 components to 0603 components
 * Updated, more stable external debugger connector
 * Updated housings for error latch, easier to solder

 
### Added
 * Markings for the holders in the rework-station
 * Hole on debugger side to add custom plastic spacer to prevent board detaching from main pcb by mistake
 * Test points for 3.3 V, 1.2 V and GND
 * Reset button
 * 0 Ohm resistors for some electronics groups
 * Low active AND gate output to summarize the error latch signals

### Fixed
 * Fixed ADA01 workaround due to wrong comparator
 * ADC filters fixed from series connection to parallel connection
 * BOM for microcontroller
 
### Removed
 * Boot switch
 * Impedance buffer U902. Also works without this buffer IC.
 * R926 (not needed).

## [1.0.0] - 2023-04-09
### Added
 * LCB-CCB-01 Control Board initial release

[1.1.0]: https://github.com/upb-lea/LCB-CCB-01_LEA_Control_Board/compare/1.0.0...1.1.0
[1.0.0]: https://github.com/upb-lea/LCB-CCB-01_LEA_Control_Board/compare/1.0.0...1.0.0
