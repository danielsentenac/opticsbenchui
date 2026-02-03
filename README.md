## OpticsBenchUI

OpticsBenchUI is a Qt application for controlling optics-bench experiments and
automated scans across cameras, motors, DACs, lasers, and counters.

Hardware families include USB/IEEE1394/GigE/Andor/Raptor/Raspberry Pi/Allied Vision
cameras, Newport/New Focus/PI/Micos/Standa motors, NKT SuperK lasers, and
Advantech/Comedi/Raspberry Pi DACs.

### External Libraries (.pro)

The `OpticsBenchUI.pro` file wires optional third‑party SDKs/libraries through
`DEFINES`, `INCLUDEPATH`, and `LIBS`. If a dependency is missing, the related
feature is disabled at build time.

Data and plotting (`HDF5` / `NO_HDF5`, `COMEDICOUNTER`, `COMEDIDAC`):
- HDF5 data files (HDF5 / NO_HDF5): HDF Group HDF5 library. (https://www.hdfgroup.org/solutions/hdf5/)
- Comedi drivers/libraries (COMEDICOUNTER, COMEDIDAC / NO_COMEDI): Comedi project. (http://www.comedi.org)
- Qwt plotting (COMEDICOUNTER): Qwt project. (https://qwt.sourceforge.io/)

Cameras (`USBCAMERA` / `NO_MULTIMEDIA`, `IEEE1394CAMERA`, `GIGECAMERA`, `NEOCAMERA`, `ZYLACAMERA`, `RAPTORFALCONCAMERA`, `RAPTORNINOX640CAMERA`, `RASPICAMERA`, `ALLIEDVISIONCAMERA`, `NO_USB`):
- libusb (NO_USB / USB comm): USB device access. (https://libusb.info/)
- libdc1394 (IEEE1394CAMERA / NO_DC1394): IEEE1394/DCAM camera control. (http://damien.douxchamps.net/ieee1394/libdc1394/)
- libraw1394 (IEEE1394CAMERA / NO_RAW1394): Linux FireWire stack. (https://www.kernel.org/doc/html/latest/driver-api/firewire.html)
- Aravis (GIGECAMERA / NO_ARAVIS): GenICam GigE/USB3 Vision. (https://github.com/AravisProject/aravis)
- Andor SDK3 (NEOCAMERA, ZYLACAMERA / NO_NEO): Andor/Oxford Instruments SDK. (https://andor.oxinst.com/products/software-development-kit)
- Allied Vision Vimba X (ALLIEDVISIONCAMERA): Allied Vision SDK. (https://www.alliedvision.com/en/products/software/vimba-x-sdk/)
- Raspberry Pi raspicam (RASPICAMERA / NO_RASPI): UCO Raspberry Pi camera library. (http://www.uco.es/investiga/grupos/ava/node/40)
- Raptor xclib (RAPTORFALCONCAMERA, RAPTORNINOX640CAMERA / NO_RAPTOR): Raptor/EPIX frame grabber SDKs. (https://www.raptorphotonics.com/support/) and (https://www.epixinc.com/products/xclib.htm)

Motors (`NO_STANDA` / Standa when available):
- Standa XIMC (NO_STANDA): Standa/XIMC SDK. (https://libximc.xisupport.com/)

DACs and SPI (`ADVANTECHDAC` / `NO_ADVDAQ`, `RASPIDAC` / `NO_SPI`):
- Advantech DAQNavi (ADVANTECHDAC / NO_ADVDAQ): Advantech SDK. (https://us2.advantech.com/DAQNavi/)
- bcm2835 SPI (RASPIDAC / NO_SPI): bcm2835 C library. (http://www.airspayce.com/mikem/bcm2835/)

### Build

- Qt 5: `qmake-qt5 OpticsBenchUI.pro` then `make -j4`
- Qt 6: `qmake6 OpticsBenchUI.pro` then `make -j4`
