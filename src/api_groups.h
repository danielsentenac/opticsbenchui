/**
 * @file api_groups.h
 * @brief api groups source file.
 */
/// \mainpage OpticsBenchUI API
/// OpticsBenchUI is a Qt application for controlling optics-bench experiments and
/// automated scans across cameras, motors, DACs, lasers, and counters.
///
/// Hardware families include USB/IEEE1394/GigE/Andor/Raptor/Raspberry Pi/Allied Vision
/// cameras, Newport/New Focus/PI/Micos/Standa motors, NKT SuperK lasers, and
/// Advantech/Comedi/Raspberry Pi DACs.
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
/// **INSTALLING OpticsBenchUI**
///
/// External packages needed prior to installation:
///
/// Graphical user interface is based on Qt 5 or Qt 6 for Linux.
///    Official Qt downloads: https://www.qt.io/download \n
///    Qt 5 source: git clone git://code.qt.io/qt/qt5.git \n
///    Qt 6 source: git clone git://code.qt.io/qt/qt6.git \n
/// OpticsBenchUI is compiled with Qt5 or Qt6 using the project file OpticsBenchUI.pro.
/// First you need to update OpticsBenchUI.pro according to optional librairies installed
/// and modify it with your personal path installations (INCLUDEPATH, LIBS tags)
///
/// \par Optional Librairies & SDKs (see Help -> User Manual / Getting Started)
///   - Firewire Camera compliant with DCAM (IIDC) standard is based on libdc1394:
///     http://damien.douxchamps.net/ieee1394/libdc1394/
///   - Advantech boards are controlled via the DAQNavi SDK:
///     https://us2.advantech.com/DAQNavi/
///   - Comedi boards controlled using the comedi library:
///     http://www.comedi.org
///   - HDF5 data library to manage data collection:
///     https://www.hdfgroup.org/solutions/hdf5/
///
///   - Aravis (GigE Vision): https://github.com/AravisProject/aravis
///   - libraw1394 (IEEE1394 support): https://www.kernel.org/doc/html/latest/driver-api/firewire.html
///   - Andor SDK3 (Neo/Zyla): https://andor.oxinst.com/products/software-development-kit/software-development-kit
///   - Allied Vision Vimba X: https://www.alliedvision.com/en/products/software/vimba-x-sdk
///   - Raptor support: https://www.raptorphotonics.com/support/
///   - EPIX XCLIB: https://www.epixinc.com/products/xclib.htm
///   - raspicam: http://www.uco.es/investiga/grupos/ava/portfolio/raspicam/
///   - Standa libximc: https://libximc.xisupport.com/
///   - bcm2835 (Raspi SPI): http://www.airspayce.com/mikem/bcm2835/
///   - Qwt (Comedi plotting): https://qwt.sourceforge.io/
///   - libusb: https://libusb.info/
///   - NKT SuperK control: https://www.nktphotonics.com/products/femtosecond-lasers/nkt-photonics-laser-control-software/
///   - Newport controllers: https://www.newport.com/p/pzc200, https://www.newport.com/p/AG-UC2
///   - PI Mercury / E-725: https://www.pi-usa.us/en/products/controllers-drivers/mercury-embedded-motion-controller-10244/,
///     https://static.pimicos.com/fileadmin/user_upload/pimicos/physik_instrumente/files/1_Datasheets/E-725_DataSheet.pdf
///
/// OpticsBenchUI installation steps:
///
/// In the source directory (where is present the OpticsBenchUI.pro file):
///
/// 1) qmake
///    - Qt 5: qmake-qt5 OpticsBenchUI.pro
///    - Qt 6: qmake6 OpticsBenchUI.pro
///
/// 2) make
///
/// This will create an OpticsBenchUI executable
///
/// Optional dependencies (HDF5, OpenGL headers, Qt Multimedia, Standa, USB, Qt SVG)
/// are detected at build time and disabled if missing.
///
/// 3) Launch OpticsBenchUI. The documentation is accessible from the GUI and
/// includes UI configuration guidance (Help -> User Manual / Getting Started).
///    See the "Compilation Options" help page for a structured list of build defines.
///
/// Build options description:
///
/// \par Core data handling
/// HDF5 / NO_HDF5: Enable/disable HDF5 storage.
///
/// \par Cameras
/// USBCAMERA: Qt Multimedia camera backend. \n
/// NO_MULTIMEDIA: Disable Qt Multimedia camera backend. \n
/// IEEE1394CAMERA: libdc1394 + libraw1394 FireWire cameras. \n
/// NO_DC1394: Disable libdc1394 camera support. \n
/// NO_RAW1394: Disable libraw1394 integration. \n
/// GIGECAMERA: Aravis GenICam GigE/USB3 cameras. \n
/// NO_ARAVIS: Disable Aravis support. \n
/// NEOCAMERA: Andor Neo (SDK3 / atcore). \n
/// ZYLACAMERA: Andor Zyla (SDK3 / atcore). \n
/// NO_NEO: Disable Andor SDK3 cameras. \n
/// ALLIEDVISIONCAMERA: Allied Vision Vimba X SDK. \n
/// RASPICAMERA: Raspberry Pi camera (raspicam). \n
/// NO_RASPI: Disable raspicam support. \n
/// RAPTORFALCONCAMERA: Raptor Falcon (EPIX XCLIB). \n
/// RAPTORNINOX640CAMERA: Raptor Ninox640 (EPIX XCLIB). \n
/// NO_RAPTOR: Disable Raptor/EPIX cameras. \n
/// NO_USB: Disable USB comms (libusb).
///
/// \par Motion / motors
/// NO_STANDA: Disable Standa XIMC motors.
///
/// \par DAC / IO
/// ADVANTECHDAC: Advantech DAQNavi SDK. \n
/// NO_ADVDAQ: Disable Advantech DAC support. \n
/// RASPIDAC: Raspberry Pi SPI DAC (bcm2835). \n
/// NO_SPI: Disable SPI DAC support. \n
/// COMEDICOUNTER: Enable Comedi counter support (requires Qwt). \n
/// COMEDIDAC: Enable Comedi DAC support. \n
/// NO_COMEDI: Disable all Comedi support.
///
/// \par Notes
/// These flags are defined in OpticsBenchUI.pro based on detected SDKs. \n
/// If a dependency is missing, its feature is disabled automatically.

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
