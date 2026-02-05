/**
 * @file api_groups.h
 * @brief api groups source file.
 */
/// \mainpage OpticsBenchUI API
/// This reference is organized by hardware type and supporting UI/utilities.
///
/// Group Overview:
/// - \ref motors "Motors": Actuator drivers and motor control APIs.
/// - \ref cameras "Cameras": Camera interfaces and device implementations.
/// - \ref dac "DAC/Comedi/Raspi/Advantech": Analog output devices and counters.
/// - \ref slm "SLM": Spatial Light Modulators (currently acquired via HDMI snapshot).
/// - \ref lasers "Lasers": Laser sources and their drivers.
/// - \ref ui "UI": Widgets and windows used in the GUI.
/// - \ref acquisition "Acquisition": Acquisition workflow threads and helpers.
/// - \ref analysis "Analysis": Analysis workflow threads and helpers.
/// - \ref utils "Utilities": Helpers, logging, and parsing.

/// \page getting_started Getting Started
/// The API reference is generated with Doxygen and lives under `docs/api`.
///
/// Build commands:
/// - `./make_api_doc.run` generates HTML output into `docs/api`.
/// - `./make_doc.run` rebuilds the Qt Help user manual in `docs/help`.

/// \defgroup hardware Hardware
/// Hardware control APIs.
/// \{

/// \defgroup motors Motors
/// Motor controllers and drivers.
/// \ingroup hardware
/// \{
/// \}

/// \defgroup cameras Cameras
/// Camera interfaces and implementations.
/// \ingroup hardware
/// \{
/// \}

/// \defgroup dac Dac
/// DAC, Comedi, Raspi, and Advantech output devices.
/// \ingroup hardware
/// \{
/// \}

/// \defgroup slm SLM
/// Spatial Light Modulators (currently acquired via HDMI snapshot in acquisition).
/// \ingroup hardware
/// \{
/// \}

/// \defgroup lasers Lasers
/// Laser sources and drivers.
/// \ingroup hardware
/// \{
/// \}

/// \}

/// \defgroup analysis Analysis
/// Analysis threads and task definitions.
/// \ingroup ui
/// \{
/// \}

/// \defgroup acquisition Acquisition
/// Acquisition threads and task definitions.
/// \ingroup ui
/// \{
/// \}

/// \defgroup ui UI
/// User interface widgets and windows.
/// \{
/// \defgroup ui_motor Motor UI
/// Motor UI windows and widgets.
/// \{
/// \}
/// \defgroup ui_camera Camera UI
/// Camera UI windows and widgets.
/// \{
/// \}
/// \defgroup ui_dac Dac UI
/// DAC UI windows and widgets.
/// \{
/// \}
/// \defgroup ui_lasers Laser UI
/// Laser UI windows and widgets.
/// \{
/// \}
/// \}

/// \defgroup utils Utilities
/// Utility helpers and logging.
/// \{
/// \class UtilsHelper
/// \ingroup utils
/// Documentation-only wrapper for the Utils namespace.
/// \}
