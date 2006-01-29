package i2c;

/**
 * <P>Main class for communication with U2C-Devices. The Java Native Interface
 * is used to communicate with the wrapper library.</p>
 *
 * <hr>
 * <p>Copyright (c) 2006, Stefan Engelke (contact@stefanengelke.de)<br>
 * All rights reserved.</p>
 *
 * <p>Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:</p>
 *
 * <p>* Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.</p>
 *
 * <p>* Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.</p>
 *
 * <p>* Neither the name of the copyright holders nor the names of
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.</p>
 *
 *  <p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.</p>
 *
 * @author Stefan Engelke
 * @version 1.0
 */


public class U2CWrapper {
  // I2C bus frequency values:
  public final static int  U2C_I2C_FREQ_FAST = 0,
                           U2C_I2C_FREQ_STD = 1,
                           U2C_I2C_FREQ_83KHZ = 2,
                           U2C_I2C_FREQ_71KHZ = 3,
                           U2C_I2C_FREQ_62KHZ = 4,
                           U2C_I2C_FREQ_50KHZ = 6,
                           U2C_I2C_FREQ_25KHZ = 16,
                           U2C_I2C_FREQ_10KHZ = 46,
                           U2C_I2C_FREQ_5KHZ = 96,
                           U2C_I2C_FREQ_2KHZ = 242;

  // SPI bus frequency values:
  public final static int  U2C_SPI_FREQ_200KHZ = 0,
                           U2C_SPI_FREQ_100KHZ = 1,
                           U2C_SPI_FREQ_83KHZ = 2,
                           U2C_SPI_FREQ_71KHZ = 3,
                           U2C_SPI_FREQ_62KHZ = 4,
                           U2C_SPI_FREQ_50KHZ = 6,
                           U2C_SPI_FREQ_25KHZ = 16,
                           U2C_SPI_FREQ_10KHZ = 46,
                           U2C_SPI_FREQ_5KHZ = 96,
                           U2C_SPI_FREQ_2KHZ = 242;

  // Line state:
  public final static int LS_RELEASED = 1,
                          LS_DROPPED_BY_I2C_BRIDGE = 2,
                          LS_DROPPED_BY_SLAVE=3,
                          LS_RAISED_BY_I2C_BRIDGE=4;

  private static boolean dllLoaded=false;

  // --------------------------------------------------------------------------
  // I2CBridge Board Initialization Routines
  /**
   * Checks how many U2C-12 devices are currently attached.
   * @return The number of the U2C-12 devices detected on current computer.
   */
  public static byte getDeviceCount() {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    return getDeviceCountN();
  }
  public static native byte getDeviceCountN();

   /**
    * Retrieves the Serial Number of the current device. This is unique Serial
    * Number. It can be used to identify device when you are using a number of
    * U2C-12 devices simultaneously.
    * @param device U2C-12 device that will be checked.
    * @return Serial Number.
    */
  public static long getSerialNum(U2CDevice device) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    int status=getSerialNumN(device);
    U2CException.checkDeviceStatus(status);
    return device.getSerial();
  }
  private static native int getSerialNumN(U2CDevice device);

  /**
   * Checks whether the given device reference is currently attached to the USB
   * and can be used by SW.
   * @param device U2C-12 device that will be checked.
   * @return Boolean, whether the device is present or not.
   */
  public static boolean isDeviceValid(U2CDevice device) throws U2CException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    int status=isDeviceValidN(device);
    return status==U2CException.U2C_SUCCESS;
  }
  private static native int isDeviceValidN(U2CDevice device);

  /**
     * Opens a U2C-12 device.
     * @param nDevice The device number to open.
     * @return U2CDevice object.
     */
  public static U2CDevice openDevice(int nDevice) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    U2CDevice device=new U2CDevice();
    int status=openDeviceN(nDevice,device);
    U2CException.checkDeviceStatus(status);
    return device;
  }
  private static native int openDeviceN(int nDevice,U2CDevice device);

  /**
     * Opens the U2C-12 device with specified Serial Number. This is a unique
     * Serial Number. It can be used to identify device when you are using a
     * number of U2C-12 devices simultaneously.
     * @param nSerialNum The Serial Number of the device to open.
     * @return U2CDevice object.
     */
  public static U2CDevice openDeviceBySerialNum(long nSerialNum) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    U2CDevice device=new U2CDevice();
    int status=openDeviceBySerialNumN(nSerialNum,device);
    U2CException.checkDeviceStatus(status);
    return device;
  }
  private static native int openDeviceBySerialNumN(long nSerialNum,U2CDevice device);

  /**
     * Closes a open device.
     * @param device U2C-12 device to close.
     */
  public static void closeDevice(U2CDevice device) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    int status=closeDeviceN(device);
    U2CException.checkDeviceStatus(status);
  }
  private static native int closeDeviceN(U2CDevice device);

  /**
     * Retrieves the version of the firmware
     * currently loaded into the U2C-12 device
     * @param device U2C-12 device to obtain the version firmware version from.
     * @return Firmware version number.
     */
  public static U2CVersionInfo getFirmwareVersion(U2CDevice device) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    U2CVersionInfo ver=new U2CVersionInfo();
    int status=getFirmwareVersionN(device,ver);
    U2CException.checkDeviceStatus(status);
    return ver;
  }
  private static native int getFirmwareVersionN(U2CDevice device,U2CVersionInfo ver);

  /**
   * Retrieves the version of the driver used to communicate with U2C-12 device.
   * @param device U2C-12 device to obtain the version of the driver.
   * @return Driver version number.
   */
  public static U2CVersionInfo getDriverVersion(U2CDevice device) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    U2CVersionInfo ver=new U2CVersionInfo();
    int status=getDriverVersionN(device,ver);
    U2CException.checkDeviceStatus(status);
    return ver;
  }
  private static native int getDriverVersionN(U2CDevice device,U2CVersionInfo ver);

  /**
   * Retrieves the version of the I2CBrdg.dll dynamic link library or shared
   * library for Linux.
   * @return I2CBrdg.dll dynamic link library version number.
   */
  public static U2CVersionInfo getDllVersion() {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    U2CVersionInfo ver=new U2CVersionInfo();
    getDllVersionN(ver);
    return ver;
  }
  private static native void getDllVersionN(U2CVersionInfo ver);

  // --------------------------------------------------------------------------
  // I2C high level and configuration routines
  /**
   * Configures I2C bus frequency.
   * @param device U2C-12 device to configure.
   * @param frequency The frequency of I2C bus, where:<br>
   * 0 corresponds to I2C bus fast mode (400 kHz).<br>
   * 1 corresponds to I2C bus standard mode (100 kHz).<br>
   * 1+n corresponds to clock period of I2C bus equal to 10 + 2*n uS.<br>
   * For convenience following constants were introduced:<br>
   * U2C_I2C_FREQ_FAST    I2C bus fast mode (400 kHz)<br>
   * U2C_I2C_FREQ_STD     I2C bus standard mode (100 kHz)<br>
   * U2C_I2C_FREQ_83KHZ   83 kHz<br>
   * U2C_I2C_FREQ_71KHZ   71 kHz<br>
   * U2C_I2C_FREQ_62KHZ   62 kHz<br>
   * U2C_I2C_FREQ_50KHZ   50 kHz<br>
   * U2C_I2C_FREQ_25KHZ   25 kHz<br>
   * U2C_I2C_FREQ_10KHZ   10 kHz<br>
   * U2C_I2C_FREQ_5KHZ     5 kHz<br>
   * U2C_I2C_FREQ_2KHZ     2 kHz<br>
   */
  public static void setI2cFreq(U2CDevice device, int frequency)  throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    int status=setI2cFreqN(device,frequency);
    U2CException.checkDeviceStatus(status);
  }
  private static native int setI2cFreqN(U2CDevice device, int frequency);

  /**
   * Obtains I2C bus frequency.
   * @param device U2C-12 device to obtain the I2C bus frequency.
   * @return I2C bus frequency. See {@link i2c.U2CWrapper#setI2cFreq(U2CDevice device, int frequency)}
   */
  public static int getI2cFreq(U2CDevice device) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    WrapedInt freq=new WrapedInt();
    int status=getI2cFreqN(device,freq);
    U2CException.checkDeviceStatus(status);
    return freq.get();
  }
  private static native int getI2cFreqN(U2CDevice device,WrapedInt freq);

  /**
   * Enables I2C bus clock synchronization.<br>
   * Clock synchronization (clock stretching) is used in situations where an I2C
   * slave is not able to co-operate with the clock speed provided by the U2C-12
   * I2C master and needs to slow down an I2C bus. I2C slave holds down the SCL
   * line low and in this way signals the I2C master about a wait state. If I2C
   * bus clock synchronization is enabled, U2C-12 device will wait until I2C
   * slave device releases the SCL line. <br>
   * <b>Warning:</b>
   * I2C bus clock synchronization (clock stretching) is implemented for I2C bus
   * frequences up to 100kHz. See U2C_SetI2cFreq() to learn how to change I2C
   * bus frequency.
   * @param device U2C-12 device.
   * @param enable Boolean whether clock stretching should be enabled.
   */
  public static void setClockSynch(U2CDevice device, boolean enable) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    int status=setClockSynchN(device,enable);
    U2CDeviceException.checkDeviceStatus(status);
  }
  private static native int setClockSynchN(U2CDevice device, boolean enable);

  /**
   * Checks if the I2C bus clock synchronization is enabled.<br>
   * See {@link i2c.U2CWrapper#setClockSynch(U2CDevice device, boolean enable)}
   * for more details
   * @param device U2C-12 device.
   * @return enable Boolean whether clock stretching is enabled.
   */
  public static boolean getClockSynch(U2CDevice device) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    WrapedInt enabled=new WrapedInt();
    int status=getClockSynchN(device,enabled);
    U2CException.checkDeviceStatus(status);
    return enabled.get()==1;
  }
  private static native int getClockSynchN(U2CDevice device,WrapedInt enabled);

  /**
   * Reads up to 256 bytes from the I2C slave device.
   * @param device U2C-12 device.
   * @param transaction Structure to be used during the I2C read transaction.
   */
  public static void read(U2CDevice device, U2CTransaction transaction)
      throws U2CDeviceException,U2CSlaveException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    int status=readN(device,transaction,transaction.buffer);
    U2CException.checkDeviceStatus(status);
    U2CException.checkSlaveStatus(status);
  }
  private static native int readN(U2CDevice device, U2CTransaction transaction,byte[] buffer);

  /**
   * Writes up to 256 bytes from the I2C slave device.
   * @param device U2C-12 device.
   * @param transaction Structure to be used during the I2C write transaction.
   */
  public static void write(U2CDevice device, U2CTransaction transaction)
      throws U2CDeviceException,U2CSlaveException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    int status=writeN(device,transaction,transaction.buffer);
    U2CException.checkDeviceStatus(status);
    U2CException.checkSlaveStatus(status);
  }
  private static native int writeN(U2CDevice device, U2CTransaction transaction,byte[] buffer);

  /**
   * Scans slave device addresses currently occupied by I2C slave devices
   * connected to the I2C bus.
   * @param device U2C-12 device.
   * @return List of slave devices.
   */
  public static U2CSlaveAdrList scanForSlaves(U2CDevice device) throws U2CDeviceException {
    if (!dllLoaded) {
      System.loadLibrary("U2CWrapper");
      dllLoaded=true;
    }
    U2CSlaveAdrList list=new U2CSlaveAdrList();
    int status=scanForSlavesN(device,list,list.list);
    U2CException.checkDeviceStatus(status);
    return list;
  }
  private static native int scanForSlavesN(U2CDevice device,U2CSlaveAdrList list,byte[] buf);

/* not implemented yet

// I2C low level routines
   public static native void start(U2CDevice device) throws U2CDeviceException;
     public static native void repeatedStart(U2CDevice device) throws U2CDeviceException;
   public static native void stop(U2CDevice device) throws U2CDeviceException;
     public static native void putByte(U2CDevice device, byte data) throws U2CDeviceException;
   public static native byte getByte(U2CDevice device) throws U2CDeviceException;
     public static native void putByteWithAck(U2CDevice device, byte data) throws U2CDeviceException;
     public static native byte getByteWithAck(U2CDevice device, boolean ack) throws U2CDeviceException;
     public static native putAck(U2CDevice device, boolean ack) throws U2CDeviceException;
     public static native boolean ack getAck(U2CDevice device) throws U2CDeviceException;

// I2c wire level routines
   public static native U2C_ReadScl(U2CDevice device, U2C_LINE_STATE *pState) throws U2CDeviceException;
   public static native U2C_ReadSda(U2CDevice device, U2C_LINE_STATE *pState) throws U2CDeviceException;
   public static native U2C_ReleaseScl(U2CDevice device) throws U2CDeviceException;
   public static native U2C_ReleaseSda(U2CDevice device) throws U2CDeviceException;
   public static native U2C_DropScl(U2CDevice device) throws U2CDeviceException;
   public static native U2C_DropSda(U2CDevice device) throws U2CDeviceException;

// GPIO routines
   public static native U2C_SetIoDirection(U2CDevice device, ULONG Value, ULONG Mask) throws U2CDeviceException;
   public static native U2C_GetIoDirection(U2CDevice device, ULONG *pValue) throws U2CDeviceException;
   public static native U2C_IoWrite(U2CDevice device, ULONG Value, ULONG Mask) throws U2CDeviceException;
   public static native U2C_IoRead(U2CDevice device, ULONG *pValue) throws U2CDeviceException;
   public static native U2C_SetSingleIoDirection(U2CDevice device, ULONG IoNumber, BOOL bOutput) throws U2CDeviceException;
   public static native U2C_GetSingleIoDirection(U2CDevice device, ULONG IoNumber, BOOL *pbOutput) throws U2CDeviceException;
   public static native U2C_SingleIoWrite(U2CDevice device, ULONG IoNumber, BOOL Value) throws U2CDeviceException;
   public static native U2C_SingleIoRead(U2CDevice device, ULONG IoNumber, BOOL *pValue) throws U2CDeviceException;

// SPI configuration routines
   public static native U2C_SpiSetConfig(U2CDevice device, BYTE CPOL, BYTE CPHA) throws U2CDeviceException;
   public static native U2C_SpiGetConfig(U2CDevice device, BYTE *pCPOL, BYTE *pCPHA) throws U2CDeviceException;
   public static native U2C_SpiSetConfigEx(U2CDevice device, DWORD Config) throws U2CDeviceException;
   public static native U2C_SpiGetConfigEx(U2CDevice device, DWORD *pConfig) throws U2CDeviceException;
   public static native U2C_SpiSetFreq(U2CDevice device, BYTE Frequency) throws U2CDeviceException;
   public static native U2C_SpiGetFreq(U2CDevice device, BYTE *pFrequency) throws U2CDeviceException;
   public static native U2C_SpiReadWrite(U2CDevice device, BYTE *pOutBuffer, BYTE *pInBuffer, WORD Length) throws U2CDeviceException;
   public static native U2C_SpiWrite(U2CDevice device, BYTE *pOutBuffer, WORD Length) throws U2CDeviceException;
   public static native U2C_SpiRead(U2CDevice device, BYTE *pInBuffer, WORD Length) throws U2CDeviceException;

   not implemented yet */
}


class WrapedInt {
  private int i;
  public void set(int i) {this.i=i;}
  public int get() { return i; }
}
